#ifndef TEMP_MANAGER_H
#define TEMP_MANAGER_H

#include <signal.h>
#include <stddef.h>

// Maximum limits
#define TEMP_MANAGER_MAX_DIRS 32
#define TEMP_MANAGER_MAX_FILES 256
#define TEMP_MANAGER_DEFAULT_MAX_SIZE (1024 * 1024 * 1024)  // 1 GB

// Temporary file manager
typedef struct {
    char* temp_dirs[TEMP_MANAGER_MAX_DIRS];    // Registered temp directories
    char* temp_files[TEMP_MANAGER_MAX_FILES];  // Registered temp files
    int n_dirs;
    int n_files;
    size_t total_size;      // Accumulated temp space usage
    size_t max_total_size;  // Max temp space limit
} TempManager;

// Create and initialize a temp manager. Returns 0 on success, -1 on error.
int temp_manager_create(TempManager* mgr, size_t max_size);

// Install atexit + SIGTERM/SIGINT cleanup handlers.
void temp_manager_install_handlers(TempManager* mgr);

// Create a temporary directory with given prefix. Returns path on success, NULL on error.
char* temp_manager_create_dir(TempManager* mgr, const char* prefix);

// Create a temporary file with given prefix. Returns path on success, NULL on error.
char* temp_manager_create_file(TempManager* mgr, const char* prefix);

// Register an externally created temporary directory for cleanup. Returns 0 on success, -1 on
// error.
int temp_manager_register_dir(TempManager* mgr, const char* path);

// Register an externally created temporary file for cleanup. Returns 0 on success, -1 on error.
int temp_manager_register_file(TempManager* mgr, const char* path);

// Cleanup all registered temp files and directories.
void temp_manager_cleanup_all(TempManager* mgr);

// Destroy the manager (free internal arrays, do NOT cleanup files).
void temp_manager_destroy(TempManager* mgr);

#endif
