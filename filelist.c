#include "filelist.h"

#include <dirent.h>
#include <fnmatch.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include "util.h"

#define INITIAL_CAPACITY 32
#define PATTERN_CAPACITY 32

/* Known source code extensions - these are always text files */
static const char* text_extensions[] = {
    "c",   "h",    "cpp",  "cxx",  "cc",   "hpp", "hxx",      "hh",  "py",   "pyw",   "js",
    "mjs", "ts",   "tsx",  "java", "go",   "rs",  "rb",       "php", "cs",   "swift", "kt",
    "kts", "sh",   "bash", "zsh",  "ksh",  "pl",  "pm",       "css", "html", "htm",   "sql",
    "xml", "yaml", "yml",  "toml", "json", "md",  "markdown", "vue", "lua",  "conf"};
static const int n_text_extensions = sizeof(text_extensions) / sizeof(text_extensions[0]);

/* Check if extension is a known text file extension */
static bool is_known_text_extension(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (!ext || ext == filepath) return false;
    ext++; /* Skip the dot */

    for (int i = 0; i < n_text_extensions; i++) {
        if (strcasecmp(ext, text_extensions[i]) == 0) {
            return true;
        }
    }
    return false;
}

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

// Check if filepath matches regex pattern (--match-f)
static int matches_regex(const char* filepath, const char* pattern) {
    if (!pattern) return 0;

    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        return 0;  // Invalid regex, treat as no match
    }

    ret = regexec(&regex, filepath, 0, NULL, 0);
    regfree(&regex);
    return (ret == 0) ? 1 : 0;
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

    FileInfo info;
    if (get_file_info(path, &info) != 0) {
        return -1;
    }

    if (info.is_regular) {
        return filelist_add(list, path);
    }

    if (!info.is_dir) {
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

        /* Use d_type when available to avoid stat calls */
        unsigned char d_type = entry->d_type;
        bool is_dir = false;
        bool is_regular = false;
        bool is_symlink = false;
        bool need_stat = false;

        if (d_type == DT_DIR) {
            is_dir = true;
        } else if (d_type == DT_REG) {
            is_regular = true;
        } else if (d_type == DT_LNK) {
            is_symlink = true;
            need_stat = true; /* Need to check symlink target */
        } else if (d_type == DT_UNKNOWN) {
            need_stat = true; /* FS doesn't support d_type, fallback to stat */
        } else {
            /* Other types (DT_BLK, DT_CHR, DT_FIFO, DT_SOCK) - skip */
            free(full_path);
            continue;
        }

        if (need_stat) {
            if (get_file_info(full_path, &info) != 0) {
                free(full_path);
                continue;
            }
            is_symlink = info.is_symlink;
            is_dir = info.is_dir;
            is_regular = info.is_regular;
        }

        if (is_symlink && is_dir) {
            /* Skip symlink directories unless --follow-links is set */
            if (!config || !config->follow_links) {
                free(full_path);
                continue;
            }
        }

        if (is_dir) {
            if (is_excluded_dir(entry->d_name, config)) {
                free(full_path);
                continue;
            }

            /* Check against directory regex patterns */
            if (config && config->match_d_pattern) {
                if (!matches_regex(entry->d_name, config->match_d_pattern)) {
                    free(full_path);
                    continue;
                }
            }

            if (config && config->not_match_d_pattern) {
                if (matches_regex(entry->d_name, config->not_match_d_pattern)) {
                    free(full_path);
                    continue;
                }
            }

            if (config && config->no_recurse) {
                free(full_path);
                continue;
            }

            filelist_scan(full_path, config, list);
            free(full_path);
        } else if (is_regular) {
            if (config && config->max_file_size > 0) {
                long file_size;
                if (need_stat) {
                    file_size = info.size;
                } else {
                    file_size = get_file_size(full_path);
                }
                if (file_size > config->max_file_size) {
                    free(full_path);
                    continue;
                }
            }

            /* Skip binary check for known source code extensions */
            if (!is_known_text_extension(full_path)) {
                if (is_binary_file(full_path)) {
                    free(full_path);
                    continue;
                }
            }

            /* Check against exclude patterns from file */
            if (matches_exclude_pattern(full_path, config)) {
                free(full_path);
                continue;
            }

            /* Check against regex match pattern (--match-f) */
            if (config && config->match_pattern) {
                const char* match_target = full_path;
                if (!config->fullpath) {
                    /* Match against basename only */
                    const char* basename = strrchr(full_path, '/');
                    match_target = basename ? basename + 1 : full_path;
                }
                if (!matches_regex(match_target, config->match_pattern)) {
                    free(full_path);
                    continue;
                }
            }

            /* Check against regex not-match pattern (--not-match-f) */
            if (config && config->not_match_pattern) {
                const char* match_target = full_path;
                if (!config->fullpath) {
                    const char* basename = strrchr(full_path, '/');
                    match_target = basename ? basename + 1 : full_path;
                }
                if (matches_regex(match_target, config->not_match_pattern)) {
                    free(full_path);
                    continue;
                }
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
