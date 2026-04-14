#include "filelist.h"
#include "util.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_CAPACITY 32

static int filelist_add(FileList *list, const char *path) {
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity == 0 ? INITIAL_CAPACITY : list->capacity * 2;
        char **new_paths = realloc(list->paths, new_capacity * sizeof(char *));
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

static int is_excluded_dir(const char *dir_name, const FilelistConfig *config) {
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

void filelist_init(FileList *list) {
    if (!list) return;
    list->paths = NULL;
    list->count = 0;
    list->capacity = 0;
}

void filelist_free(FileList *list) {
    if (!list) return;

    for (int i = 0; i < list->count; i++) {
        free(list->paths[i]);
    }
    free(list->paths);

    list->paths = NULL;
    list->count = 0;
    list->capacity = 0;
}

int filelist_scan(const char *path, const FilelistConfig *config, FileList *list) {
    if (!path || !list) {
        return -1;
    }

    if (is_regular_file(path)) {
        return filelist_add(list, path);
    }

    if (!is_directory(path)) {
        return -1;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char *full_path = path_join(path, entry->d_name);
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

            filelist_add(list, full_path);
            free(full_path);
        } else {
            free(full_path);
        }
    }

    closedir(dir);
    return 0;
}
