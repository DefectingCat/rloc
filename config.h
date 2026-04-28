#ifndef CONFIG_H
#define CONFIG_H

#include "cli.h"

/* Default config file path: ~/.config/rloc/options.txt */
#define RLOC_CONFIG_DIR ".config/rloc"
#define RLOC_CONFIG_FILE "options.txt"

/* Scanner configuration */
#define SCANNER_INITIAL_CAPACITY 32
#define SCANNER_MAX_DEPTH 64
#define SCANNER_YIELD_INTERVAL 64

/* Counter configuration */
#define COUNTER_MAX_LINE_LENGTH 4096
#define COUNTER_MAX_FILE_SIZE_MB 100

/* Parallel processing */
#define PARALLEL_MAX_WORKERS 16
#define PARALLEL_MIN_FILES 50
#define PARALLEL_CHUNK_SIZE 100
#define PARALLEL_TIMEOUT_SEC 300

/* Coroutine scanner */
#define CORO_MAX_CONCURRENT 256
#define CORO_MAX_DEPTH 32
#define CORO_YIELD_INTERVAL 64

/* File list */
#define FILELIST_INITIAL_CAPACITY 32
#define FILELIST_PATTERN_CAPACITY 32

/* Unique detection */
#define UNIQUE_TABLE_MAX 100000

/* Load config from file and apply to args.
 * Single-value options overwrite, list options append.
 * Returns 0 on success (or no config file), -1 on error.
 */
int config_load(const char* filepath, CliArgs* args);

/* Get default config file path.
 * Caller must free the returned string.
 * Returns NULL if home directory cannot be determined.
 */
char* config_get_default_path(void);

#endif /* CONFIG_H */
