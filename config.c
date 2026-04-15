#define _POSIX_C_SOURCE 200809L
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Helper: append comma-separated values to existing string array */
static int append_csv_to_array(const char* csv, char*** array, int* count) {
    if (!csv || !array || !count) return -1;

    char* copy = strdup(csv);
    if (!copy) return -1;

    char* token = strtok(copy, ",");
    while (token) {
        /* Check for duplicates */
        int dup = 0;
        for (int i = 0; i < *count; i++) {
            if (strcmp((*array)[i], token) == 0) {
                dup = 1;
                break;
            }
        }
        if (!dup) {
            int cap = 0;
            if (*count == 0) {
                cap = 8;
            } else {
                cap = *count;
                if ((*count & (*count - 1)) == 0 && *count >= 8) {
                    cap = *count * 2;
                } else {
                    cap = *count + 1;
                    // Round up to next power of 2 for cli.c's realloc logic
                    int p2 = 8;
                    while (p2 < cap) p2 *= 2;
                    cap = p2;
                }
            }
            if (*count == 0) {
                *array = malloc(cap * sizeof(char*));
            } else {
                char** new_arr = realloc(*array, cap * sizeof(char*));
                if (!new_arr) { free(copy); return -1; }
                *array = new_arr;
            }
            (*array)[*count] = strdup(token);
            if (!(*array)[*count]) { free(copy); return -1; }
            (*count)++;
        }
        token = strtok(NULL, ",");
    }
    free(copy);
    return 0;
}

static char* trim_leading(char* str) {
    while (*str == ' ' || *str == '\t') str++;
    return str;
}

static void trim_trailing(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' ||
                       str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

int config_load(const char* filepath, CliArgs* args) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        /* File not found is OK - return success */
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char* trimmed = trim_leading(line);
        trim_trailing(trimmed);

        /* Skip empty lines and comments */
        if (*trimmed == '\0' || *trimmed == '#') continue;

        /* Parse as --key=value or --flag */
        if (trimmed[0] != '-' || trimmed[1] != '-') continue;

        char* eq = strchr(trimmed + 2, '=');
        char* key_end = eq ? eq : trimmed + strlen(trimmed);

        /* Extract key length */
        size_t key_len = key_end - trimmed;
        char key[64] = {0};
        if (key_len >= sizeof(key)) continue;
        strncpy(key, trimmed, key_len);

        /* Value (after '=') */
        char* value = eq ? trim_leading(eq + 1) : NULL;

        /* Single-value options: overwrite */
        if (strcmp(key, "--no-recurse") == 0) {
            args->no_recurse = 1;
        } else if (strcmp(key, "--quiet") == 0) {
            args->quiet = 1;
        } else if (strcmp(key, "--by-file") == 0) {
            args->by_file = 1;
        } else if (strcmp(key, "--by-file-by-lang") == 0) {
            args->by_file_by_lang = 1;
        } else if (strcmp(key, "--skip-uniqueness") == 0) {
            args->skip_uniqueness = 1;
        } else if (strcmp(key, "--json") == 0) {
            args->output_format = FORMAT_JSON;
        } else if (strcmp(key, "--csv") == 0) {
            args->output_format = FORMAT_CSV;
        } else if (strcmp(key, "--md") == 0) {
            args->output_format = FORMAT_MD;
        } else if (strcmp(key, "--yaml") == 0) {
            args->output_format = FORMAT_YAML;
        } else if (strcmp(key, "--xml") == 0) {
            args->output_format = FORMAT_XML;
        } else if (strcmp(key, "--html") == 0) {
            args->output_format = FORMAT_HTML;
        } else if (value) {
            /* Value options */
            if (strcmp(key, "--max-file-size") == 0) {
                args->max_file_size_mb = atol(value);
            } else if (strcmp(key, "--max-temp-size") == 0) {
                long mb = atol(value);
                if (mb > 0) args->max_temp_size = (size_t)mb * 1024 * 1024;
            } else if (strcmp(key, "--progress-rate") == 0) {
                args->progress_rate = atoi(value);
                if (args->progress_rate < 1) args->progress_rate = 1;
            } else if (strcmp(key, "--skip-leading") == 0) {
                char* comma = strchr(value, ',');
                if (comma) *comma = '\0';
                args->skip_leading = (int)strtol(value, NULL, 10);
            } else if (strcmp(key, "--sdir") == 0) {
                if (args->staging_dir) free(args->staging_dir);
                args->staging_dir = strdup(value);
            } else if (strcmp(key, "--report-file") == 0) {
                if (args->report_file) free(args->report_file);
                args->report_file = strdup(value);
            } else if (strcmp(key, "--exclude-list-file") == 0) {
                if (args->exclude_list_file) free(args->exclude_list_file);
                args->exclude_list_file = strdup(value);
            } else if (strcmp(key, "--sql") == 0) {
                if (args->sql_file) free(args->sql_file);
                args->sql_file = strdup(value);
                args->output_format = FORMAT_SQL;
            } else if (strcmp(key, "--vcs") == 0) {
                if (strcmp(value, "git") == 0) args->vcs = VCS_GIT;
                else if (strcmp(value, "svn") == 0) args->vcs = VCS_SVN;
                else if (strcmp(value, "auto") == 0) args->vcs = VCS_AUTO;
            } else if (strcmp(key, "--diff") == 0) {
                if (args->diff_commit1) free(args->diff_commit1);
                if (args->diff_commit2) free(args->diff_commit2);
                const char* sep = strstr(value, "..");
                if (sep) {
                    size_t len1 = sep - value;
                    args->diff_commit1 = malloc(len1 + 1);
                    if (args->diff_commit1) {
                        memcpy(args->diff_commit1, value, len1);
                        args->diff_commit1[len1] = '\0';
                    }
                    args->diff_commit2 = strdup(sep + 2);
                } else {
                    args->diff_commit1 = strdup(value);
                    args->diff_commit2 = strdup("HEAD");
                }
            } else if (strcmp(key, "--match-f") == 0) {
                if (args->match_pattern) free(args->match_pattern);
                args->match_pattern = strdup(value);
            } else if (strcmp(key, "--not-match-f") == 0) {
                if (args->not_match_pattern) free(args->not_match_pattern);
                args->not_match_pattern = strdup(value);
            } else if (strcmp(key, "--match-d") == 0) {
                if (args->match_d_pattern) free(args->match_d_pattern);
                args->match_d_pattern = strdup(value);
            } else if (strcmp(key, "--not-match-d") == 0) {
                if (args->not_match_d_pattern) free(args->not_match_d_pattern);
                args->not_match_d_pattern = strdup(value);
            } else if (strcmp(key, "--unique") == 0) {
                if (args->unique_file) free(args->unique_file);
                args->unique_file = strdup(value);
            } else if (strcmp(key, "--ignored") == 0) {
                if (args->ignored_file) free(args->ignored_file);
                args->ignored_file = strdup(value);
            }
            /* List options: append (no duplicates) */
            else if (strcmp(key, "--exclude-dir") == 0) {
                append_csv_to_array(value, &args->exclude_dirs, &args->n_exclude_dirs);
            } else if (strcmp(key, "--include-lang") == 0) {
                append_csv_to_array(value, &args->include_langs, &args->n_include_langs);
            } else if (strcmp(key, "--exclude-lang") == 0) {
                append_csv_to_array(value, &args->exclude_langs, &args->n_exclude_langs);
            } else if (strcmp(key, "--include-ext") == 0) {
                append_csv_to_array(value, &args->include_exts, &args->n_include_exts);
            } else if (strcmp(key, "--exclude-ext") == 0) {
                append_csv_to_array(value, &args->exclude_exts, &args->n_exclude_exts);
            } else if (strcmp(key, "--skip-leading") == 0 && value && strchr(value, ',')) {
                /* Re-parse extensions part */
                char* comma = strchr(value, ',');
                if (comma) {
                    append_csv_to_array(comma + 1, &args->skip_leading_exts, &args->n_skip_leading_exts);
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

char* config_get_default_path(void) {
    const char* home = getenv("HOME");
    if (!home) return NULL;

    size_t len = strlen(home) + strlen(RLOC_CONFIG_DIR) + strlen(RLOC_CONFIG_FILE) + 3;
    char* path = malloc(len);
    if (!path) return NULL;

    snprintf(path, len, "%s/%s/%s", home, RLOC_CONFIG_DIR, RLOC_CONFIG_FILE);
    return path;
}
