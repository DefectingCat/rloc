#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "temp_manager.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Global for signal handlers
static volatile sig_atomic_t g_manager_ready = 0;
static TempManager* g_manager = NULL;

// Forward declarations
static int remove_dir_recursive(const char* path);
static void signal_cleanup_handler(int sig);
static void atexit_cleanup_wrapper(void);

int temp_manager_create(TempManager* mgr, size_t max_size) {
    if (!mgr) return -1;

    memset(mgr, 0, sizeof(TempManager));
    mgr->max_total_size = (max_size > 0) ? max_size : TEMP_MANAGER_DEFAULT_MAX_SIZE;
    mgr->total_size = 0;
    mgr->n_dirs = 0;
    mgr->n_files = 0;

    return 0;
}

void temp_manager_install_handlers(TempManager* mgr) {
    g_manager = mgr;
    g_manager_ready = 1;

    atexit(atexit_cleanup_wrapper);
    signal(SIGTERM, signal_cleanup_handler);
    signal(SIGINT, signal_cleanup_handler);
}

char* temp_manager_create_dir(TempManager* mgr, const char* prefix) {
    if (!mgr || !prefix) return NULL;
    if (mgr->n_dirs >= TEMP_MANAGER_MAX_DIRS) return NULL;

    // Build template path: /tmp/<prefix>.XXXXXX
    char tmpl[1024];
    int n = snprintf(tmpl, sizeof(tmpl), "/tmp/%s.XXXXXX", prefix);
    if (n < 0 || n >= (int)sizeof(tmpl)) return NULL;

    // Create temp directory
    char* result = mkdtemp(tmpl);
    if (!result) return NULL;

    // Register the directory
    char* dir = strdup(result);
    if (!dir) return NULL;

    mgr->temp_dirs[mgr->n_dirs] = dir;
    mgr->n_dirs++;

    // Estimate size as 0 initially (will be calculated at cleanup)
    return dir;
}

char* temp_manager_create_file(TempManager* mgr, const char* prefix) {
    if (!mgr || !prefix) return NULL;
    if (mgr->n_files >= TEMP_MANAGER_MAX_FILES) return NULL;

    // Build template path: /tmp/<prefix>.XXXXXX
    char tmpl[1024];
    int n = snprintf(tmpl, sizeof(tmpl), "/tmp/%s.XXXXXX", prefix);
    if (n < 0 || n >= (int)sizeof(tmpl)) return NULL;

    // Create temp file
    int fd = mkstemp(tmpl);
    if (fd < 0) return NULL;
    close(fd);

    // Register the file
    char* file = strdup(tmpl);
    if (!file) {
        unlink(tmpl);
        return NULL;
    }

    mgr->temp_files[mgr->n_files] = file;
    mgr->n_files++;

    // Get file size
    struct stat st;
    if (stat(file, &st) == 0) {
        mgr->total_size += (size_t)st.st_size;
    }

    return file;
}

int temp_manager_register_dir(TempManager* mgr, const char* path) {
    if (!mgr || !path) return -1;
    if (mgr->n_dirs >= TEMP_MANAGER_MAX_DIRS) return -1;

    char* dir = strdup(path);
    if (!dir) return -1;

    mgr->temp_dirs[mgr->n_dirs] = dir;
    mgr->n_dirs++;

    return 0;
}

int temp_manager_register_file(TempManager* mgr, const char* path) {
    if (!mgr || !path) return -1;
    if (mgr->n_files >= TEMP_MANAGER_MAX_FILES) return -1;

    char* file = strdup(path);
    if (!file) return -1;

    mgr->temp_files[mgr->n_files] = file;
    mgr->n_files++;

    // Get file size
    struct stat st;
    if (stat(file, &st) == 0) {
        mgr->total_size += (size_t)st.st_size;
    }

    return 0;
}

void temp_manager_cleanup_all(TempManager* mgr) {
    if (!mgr) return;

    // Remove temp directories (recursive)
    for (int i = 0; i < mgr->n_dirs; i++) {
        if (mgr->temp_dirs[i]) {
            remove_dir_recursive(mgr->temp_dirs[i]);
            free(mgr->temp_dirs[i]);
            mgr->temp_dirs[i] = NULL;
        }
    }
    mgr->n_dirs = 0;

    // Remove temp files
    for (int i = 0; i < mgr->n_files; i++) {
        if (mgr->temp_files[i]) {
            unlink(mgr->temp_files[i]);
            free(mgr->temp_files[i]);
            mgr->temp_files[i] = NULL;
        }
    }
    mgr->n_files = 0;

    mgr->total_size = 0;
}

void temp_manager_destroy(TempManager* mgr) {
    if (!mgr) return;

    // Free any remaining registered paths (without deleting files)
    for (int i = 0; i < mgr->n_dirs; i++) {
        free(mgr->temp_dirs[i]);
        mgr->temp_dirs[i] = NULL;
    }
    for (int i = 0; i < mgr->n_files; i++) {
        free(mgr->temp_files[i]);
        mgr->temp_files[i] = NULL;
    }
    mgr->n_dirs = 0;
    mgr->n_files = 0;
    mgr->total_size = 0;
    g_manager_ready = 0;
    g_manager = NULL;
}

// --- Internal helpers ---

static void atexit_cleanup_wrapper(void) {
    if (g_manager_ready && g_manager) {
        temp_manager_cleanup_all(g_manager);
    }
}

static void signal_cleanup_handler(int sig) {
    if (g_manager_ready && g_manager) {
        temp_manager_cleanup_all(g_manager);
    }
    if (sig != 0) _exit(128 + sig);
}

static int remove_dir_recursive(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        // Not a directory, try to remove as file
        unlink(path);
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char child[1024];
        int n = snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);
        if (n < 0 || n >= (int)sizeof(child)) continue;

        struct stat st;
        if (stat(child, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                remove_dir_recursive(child);
            } else {
                unlink(child);
            }
        }
    }
    closedir(dir);
    rmdir(path);
    return 0;
}
