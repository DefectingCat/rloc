#include "coro_scanner.h"

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filelist.h"

#define YIELD_INTERVAL 64  // 每 64 个条目 yield 一次

// 创建扫描上下文
static scan_context_t* create_scan_context(
    coco_sched_t *sched,
    const char *path,
    coco_channel_t *file_channel,
    atomic_int *scan_coros_count,
    const FilelistConfig *config
) {
    scan_context_t *ctx = malloc(sizeof(scan_context_t));
    if (!ctx) return NULL;

    ctx->sched = sched;
    ctx->path = strdup(path);
    ctx->file_channel = file_channel;
    ctx->depth = 0;
    ctx->follow_links = config->follow_links;
    ctx->scan_coros_count = scan_coros_count;
    ctx->config = config;

    atomic_fetch_add(scan_coros_count, 1);  // 初始计数
    return ctx;
}

// 创建子目录扫描上下文
static scan_context_t* create_child_context(
    const scan_context_t *parent,
    const char *child_path
) {
    scan_context_t *ctx = malloc(sizeof(scan_context_t));
    if (!ctx) return NULL;

    ctx->sched = parent->sched;
    ctx->path = strdup(child_path);
    ctx->file_channel = parent->file_channel;
    ctx->depth = parent->depth + 1;
    ctx->follow_links = parent->follow_links;
    ctx->scan_coros_count = parent->scan_coros_count;
    ctx->config = parent->config;

    atomic_fetch_add(parent->scan_coros_count, 1);  // 增加计数
    return ctx;
}

// 顺序扫描（当达到协程限制时使用）
static void scan_dir_sequential(scan_context_t *ctx) {
    DIR *dir = opendir(ctx->path);
    if (!dir) {
        fprintf(stderr, "Warning: cannot open directory: %s\n", ctx->path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", ctx->path, entry->d_name);

        // 简化处理：只发送文件，不递归子目录
        if (entry->d_type != DT_DIR) {
            coco_channel_send(ctx->file_channel, strdup(full_path));
        }
    }
    closedir(dir);
}

// 扫描协程
void scan_dir_coro(void *arg) {
    scan_context_t *ctx = (scan_context_t*)arg;

    // 深度限制
    if (ctx->depth > MAX_SCAN_DEPTH) {
        fprintf(stderr, "Warning: directory depth exceeded %d, skipping: %s\n",
                MAX_SCAN_DEPTH, ctx->path);
        if (atomic_fetch_sub(ctx->scan_coros_count, 1) == 1) {
            coco_channel_close(ctx->file_channel);
        }
        free((void*)ctx->path);
        free(ctx);
        return;
    }

    // 并发协程限制
    if (atomic_load(ctx->scan_coros_count) > MAX_CONCURRENT_COROS) {
        scan_dir_sequential(ctx);
        if (atomic_fetch_sub(ctx->scan_coros_count, 1) == 1) {
            coco_channel_close(ctx->file_channel);
        }
        free((void*)ctx->path);
        free(ctx);
        return;
    }

    DIR *dir = opendir(ctx->path);

    // 错误处理：目录无法访问
    if (!dir) {
        fprintf(stderr, "Warning: cannot open directory: %s\n", ctx->path);
        if (atomic_fetch_sub(ctx->scan_coros_count, 1) == 1) {
            coco_channel_close(ctx->file_channel);
        }
        free((void*)ctx->path);
        free(ctx);
        return;
    }

    struct dirent *entry;
    int entry_count = 0;

    while ((entry = readdir(dir))) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // 构建完整路径
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", ctx->path, entry->d_name);

        // d_type fallback：某些文件系统不支持 DT_*
        unsigned char d_type = entry->d_type;
        if (d_type == DT_UNKNOWN) {
            struct stat st;
            if (lstat(full_path, &st) == 0) {
                d_type = S_ISDIR(st.st_mode) ? DT_DIR : DT_REG;
            }
        }

        if (d_type == DT_DIR) {
            // 符号链接目录处理：遵循 --follow-links 配置
            if (!ctx->follow_links && entry->d_type == DT_LNK) {
                continue;  // 跳过符号链接目录
            }

            // 为子目录创建新的扫描协程（并发点）
            scan_context_t *child_ctx = create_child_context(ctx, full_path);
            if (child_ctx) {
                coco_create(ctx->sched, scan_dir_coro, child_ctx, SCAN_CORO_STACK_SIZE);
            }
        } else {
            // 发送文件路径到 channel
            coco_channel_send(ctx->file_channel, strdup(full_path));
        }

        // 显式 yield：让调度器切换到其他协程
        if (++entry_count % YIELD_INTERVAL == 0) {
            coco_yield();
        }
    }

    closedir(dir);

    // 递减扫描协程计数，如果是最后一个则关闭 channel
    if (atomic_fetch_sub(ctx->scan_coros_count, 1) == 1) {
        coco_channel_close(ctx->file_channel);
    }
    free((void*)ctx->path);
    free(ctx);
}

// 收集器协程：从 channel 读取文件路径并填充 filelist
void collector_coro(void *arg) {
    collector_context_t *ctx = (collector_context_t*)arg;
    void *file_path;
    int ret;

    while ((ret = coco_channel_recv(ctx->file_channel, &file_path)) == COCO_OK) {
        filelist_append(ctx->filelist, (char*)file_path);
        free(file_path);
    }

    // 检查是否为正常关闭
    if (ret != COCO_ERROR_CHANNEL_CLOSED) {
        fprintf(stderr, "Warning: channel recv error: %d\n", ret);
    }

    // 标记收集完成
    ctx->done = true;
}

// 协程扫描入口函数
int coro_scan_directory(const char *root_path, const FilelistConfig *config, FileList *filelist) {
    // 创建调度器和 channel（带错误检查）
    coco_sched_t *sched = coco_sched_create();
    if (!sched) {
        fprintf(stderr, "Error: failed to create scheduler\n");
        return -1;
    }

    // io_uring 配置 (Linux 5.1+ 专属，macOS 自动跳过)
#if defined(__linux__)
    coco_io_options_t io_opts = {
        .queue_depth = 256,
        .sqpoll_enabled = true,
        .sqpoll_idle_ms = 1000,
    };
    if (coco_sched_set_io_options(sched, &io_opts) == COCO_OK) {
        coco_sched_set_io_backend(sched, COCO_IO_BACKEND_IOURING);
        // Fallback 自动处理: 失败时使用 epoll
    }
#endif

    coco_channel_t *file_channel = coco_channel_create(2048);  // 缓冲 2048 条目
    if (!file_channel) {
        fprintf(stderr, "Error: failed to create file channel\n");
        coco_sched_destroy(sched);
        return -1;
    }

    // 初始化收集器上下文
    collector_context_t collector_ctx = {
        .filelist = filelist,
        .file_channel = file_channel,
        .done = false
    };

    // 启动收集器协程 - 高优先级
    coco_coro_t *collector = coco_create(sched, collector_coro, &collector_ctx, SCAN_CORO_STACK_SIZE);
    coco_set_priority(collector, COCO_PRIORITY_HIGH);

    // 启动根目录扫描协程 - 低优先级
    atomic_int scan_coros_count = ATOMIC_VAR_INIT(0);
    scan_context_t *root_ctx = create_scan_context(sched, root_path, file_channel, &scan_coros_count, config);
    if (!root_ctx) {
        fprintf(stderr, "Error: failed to create scan context\n");
        coco_channel_destroy(file_channel);
        coco_sched_destroy(sched);
        return -1;
    }
    coco_coro_t *scanner = coco_create(sched, scan_dir_coro, root_ctx, SCAN_CORO_STACK_SIZE);
    coco_set_priority(scanner, COCO_PRIORITY_LOW);

    // 运行调度器直到所有协程完成
    while (!collector_ctx.done) {
        // 检查是否所有扫描协程已完成
        if (atomic_load(&scan_coros_count) == 0) {
            coco_channel_close(file_channel);
        }

        int ret = coco_sched_run_once(sched);
        if (ret != COCO_OK && !collector_ctx.done) {
            usleep(1000);  // 1ms
        }
    }

    // 清理
    coco_channel_destroy(file_channel);
    coco_sched_destroy(sched);

    return 0;
}