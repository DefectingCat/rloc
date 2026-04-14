#include "filelist.h"

#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"

#define INITIAL_CAPACITY 32
#define PATTERN_CAPACITY 32

static int filelist_add(FileList* list, const char* path) {
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity == 0 ? INITIAL_CAPACITY : list->capacity * 2;
        char** new_paths = realloc(list->paths, new_capacity * sizeof(char*));
        if (!new_paths) {
            return -1;
        }
        list->paths = new_paths;
        list->capacity = new_capacity;
    }

    list->paths[list->count] = strdup(path);
    if (!list->paths[list->count]) {
        return -1;
    }

    list->count++;
    return 0;
}

static int is_excluded_dir(const char* dir_name, const FilelistConfig* config) {
    if (!config || !config->exclude_dirs) {
        return 0;
    }

    for (int i = 0; i < config->n_exclude_dirs; i++) {
        if (strcmp(dir_name, config->exclude_dirs[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Check if a filepath matches any exclude pattern (glob matching)
static int matches_exclude_pattern(const char* filepath, const FilelistConfig* config) {
    if (!config || !config->exclude_patterns || config->n_exclude_patterns == 0) {
        return 0;
    }

    for (int i = 0; i < config->n_exclude_patterns; i++) {
        const char* pattern = config->exclude_patterns[i];
        // Match against the full path or just the filename
        if (fnmatch(pattern, filepath, FNM_PATHNAME) == 0) {
            return 1;
        }
        // Also try matching against just the basename
        const char* basename = strrchr(filepath, '/');
        if (basename) {
            basename++;  // Skip the '/'
        } else {
            basename = filepath;
        }
        if (fnmatch(pattern, basename, 0) == 0) {
            return 1;
        }
    }
    return 0;
}

void filelist_init(FileList* list) {
    if (!list) return;
    list->paths = NULL;
    list->count = 0;
    list->capacity = 0;
}

void filelist_free(FileList* list) {
    if (!list) return;

    for (int i = 0; i < list->count; i++) {
        free(list->paths[i]);
    }
    free(list->paths);

    list->paths = NULL;
    list->count = 0;
    list->capacity = 0;
}

int filelist_scan(const char* path, const FilelistConfig* config, FileList* list) {
    if (!path || !list) {
        return -1;
    }

    if (is_regular_file(path)) {
        return filelist_add(list, path);
    }

    if (!is_directory(path)) {
        return -1;
    }

    DIR* dir = opendir(path);
    if (!dir) {
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char* full_path = path_join(path, entry->d_name);
        if (!full_path) {
            continue;
        }

        if (is_symlink(full_path) && is_directory(full_path)) {
            free(full_path);
            continue;
        }

        if (is_directory(full_path)) {
            if (is_excluded_dir(entry->d_name, config)) {
                free(full_path);
                continue;
            }

            if (config && config->no_recurse) {
                free(full_path);
                continue;
            }

            filelist_scan(full_path, config, list);
            free(full_path);
        } else if (is_regular_file(full_path)) {
            if (config && config->max_file_size > 0) {
                long file_size = get_file_size(full_path);
                if (file_size > config->max_file_size) {
                    free(full_path);
                    continue;
                }
            }

            if (is_binary_file(full_path)) {
                free(full_path);
                continue;
            }

            // Check against exclude patterns from file
            if (matches_exclude_pattern(full_path, config)) {
                free(full_path);
                continue;
            }

            filelist_add(list, full_path);
            free(full_path);
        } else {
            free(full_path);
        }
    }

    closedir(dir);
    return 0;
}

int filelist_load_exclude_patterns(const char* filepath, FilelistConfig* config) {
    if (!filepath || !config) {
        return -1;
    }

    FILE* file = fopen(filepath, "r");
    if (!file) {
        return -1;
    }

    config->exclude_patterns = malloc(PATTERN_CAPACITY * sizeof(char*));
    if (!config->exclude_patterns) {
        fclose(file);
        return -1;
    }
    config->n_exclude_patterns = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove trailing newline
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        // Skip empty lines and comments
        if (len == 0 || line[0] == '#') {
            continue;
        }

        // Grow array if needed
        if (config->n_exclude_patterns >= PATTERN_CAPACITY) {
            int new_cap = config->n_exclude_patterns * 2;
            char** new_patterns = realloc(config->exclude_patterns, new_cap * sizeof(char*));
            if (!new_patterns) {
                fclose(file);
                return -1;
            }
            config->exclude_patterns = new_patterns;
        }

        config->exclude_patterns[config->n_exclude_patterns] = strdup(line);
        if (!config->exclude_patterns[config->n_exclude_patterns]) {
            fclose(file);
            return -1;
        }
        config->n_exclude_patterns++;
    }

    fclose(file);
    return 0;
}

void filelist_free_exclude_patterns(FilelistConfig* config) {
    if (!config || !config->exclude_patterns) {
        return;
    }

    for (int i = 0; i < config->n_exclude_patterns; i++) {
        free(config->exclude_patterns[i]);
    }
    free(config->exclude_patterns);
    config->exclude_patterns = NULL;
    config->n_exclude_patterns = 0;
}
