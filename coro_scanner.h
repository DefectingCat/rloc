#ifndef CORO_SCANNER_H
#define CORO_SCANNER_H

#include <stdatomic.h>
#include "coco.h"
#include "filelist.h"

// 扫描协程上下文
typedef struct scan_context {
    coco_sched_t *sched;
    coco_channel_t *file_channel;
    const char *path;
    int depth;
    bool follow_links;
    atomic_int *scan_coros_count;  // 活跃扫描协程计数（不含 collector）
    const FilelistConfig *config;
} scan_context_t;

// 收集器协程上下文
typedef struct collector_context {
    FileList *filelist;
    coco_channel_t *file_channel;
    volatile bool done;
} collector_context_t;

// 栈大小配置 - 使用 coco 栈池 size class
#define SCAN_CORO_STACK_SIZE COCO_STACK_SMALL  // 16KB

// 深度和并发限制
#define MAX_SCAN_DEPTH 256
#define MAX_CONCURRENT_COROS 1000

// 协程扫描入口函数
int coro_scan_directory(const char *root_path, const FilelistConfig *config, FileList *filelist);

#endif