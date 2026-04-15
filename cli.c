#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cli_parse(int argc, char** argv, CliArgs* args) {
    /* Save config-set list arrays before resetting */
    int saved_n_exclude_dirs = args->n_exclude_dirs;
    char** saved_exclude_dirs = args->exclude_dirs;
    int saved_n_include_langs = args->n_include_langs;
    char** saved_include_langs = args->include_langs;
    int saved_n_exclude_langs = args->n_exclude_langs;
    char** saved_exclude_langs = args->exclude_langs;
    int saved_n_include_exts = args->n_include_exts;
    char** saved_include_exts = args->include_exts;
    int saved_n_exclude_exts = args->n_exclude_exts;
    char** saved_exclude_exts = args->exclude_exts;

    /* Save config-set scalar values */
    int saved_no_config = args->no_config;
    char* saved_config_file = args->config_file;
    int saved_quiet = args->quiet;
    int saved_output_format = args->output_format;
    int saved_no_recurse = args->no_recurse;
    int saved_max_file_size_mb = args->max_file_size_mb;
    int saved_by_file = args->by_file;
    int saved_by_file_by_lang = args->by_file_by_lang;
    int saved_vcs = args->vcs;
    int saved_skip_leading = args->skip_leading;
    char** saved_skip_leading_exts = args->skip_leading_exts;
    int saved_n_skip_leading_exts = args->n_skip_leading_exts;
    int saved_progress_rate = args->progress_rate;
    int saved_skip_uniqueness = args->skip_uniqueness;
    char* saved_unique_file = args->unique_file;
    char* saved_ignored_file = args->ignored_file;
    int saved_max_temp_size = args->max_temp_size;
    char* saved_staging_dir = args->staging_dir;
    char* saved_exclude_list_file = args->exclude_list_file;
    char* saved_match_pattern = args->match_pattern;
    char* saved_not_match_pattern = args->not_match_pattern;
    char* saved_match_d_pattern = args->match_d_pattern;
    char* saved_not_match_d_pattern = args->not_match_d_pattern;
    char* saved_sql_file = args->sql_file;
    char* saved_report_file = args->report_file;

    // Initialize structure - reset everything
    memset(args, 0, sizeof(CliArgs));

    /* Restore config-set values */
    args->n_exclude_dirs = saved_n_exclude_dirs;
    args->exclude_dirs = saved_exclude_dirs;
    args->n_include_langs = saved_n_include_langs;
    args->include_langs = saved_include_langs;
    args->n_exclude_langs = saved_n_exclude_langs;
    args->exclude_langs = saved_exclude_langs;
    args->n_include_exts = saved_n_include_exts;
    args->include_exts = saved_include_exts;
    args->n_exclude_exts = saved_n_exclude_exts;
    args->exclude_exts = saved_exclude_exts;
    args->no_config = saved_no_config;
    args->config_file = saved_config_file;
    args->quiet = saved_quiet;
    args->output_format = saved_output_format;
    args->no_recurse = saved_no_recurse;
    args->max_file_size_mb = saved_max_file_size_mb;
    args->by_file = saved_by_file;
    args->by_file_by_lang = saved_by_file_by_lang;
    args->vcs = saved_vcs;
    args->skip_leading = saved_skip_leading;
    args->skip_leading_exts = saved_skip_leading_exts;
    args->n_skip_leading_exts = saved_n_skip_leading_exts;
    args->progress_rate = saved_progress_rate;
    args->skip_uniqueness = saved_skip_uniqueness;
    args->unique_file = saved_unique_file;
    args->ignored_file = saved_ignored_file;
    args->max_temp_size = saved_max_temp_size;
    args->staging_dir = saved_staging_dir;
    args->exclude_list_file = saved_exclude_list_file;
    args->match_pattern = saved_match_pattern;
    args->not_match_pattern = saved_not_match_pattern;
    args->match_d_pattern = saved_match_d_pattern;
    args->not_match_d_pattern = saved_not_match_d_pattern;
    args->sql_file = saved_sql_file;
    args->report_file = saved_report_file;

    // Allocate initial array for input files
    int capacity = 16;
    args->input_files = malloc(capacity * sizeof(char*));
    if (!args->input_files) {
        return -1;
    }

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            args->show_help = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            args->show_version = 1;
        } else if (strcmp(argv[i], "--no-recurse") == 0) {
            args->no_recurse = 1;
        } else if (strcmp(argv[i], "--json") == 0) {
            args->output_format = FORMAT_JSON;
        } else if (strcmp(argv[i], "--csv") == 0) {
            args->output_format = FORMAT_CSV;
        } else if (strcmp(argv[i], "--md") == 0) {
            args->output_format = FORMAT_MD;
        } else if (strcmp(argv[i], "--yaml") == 0) {
            args->output_format = FORMAT_YAML;
        } else if (strcmp(argv[i], "--xml") == 0) {
            args->output_format = FORMAT_XML;
        } else if (strcmp(argv[i], "--html") == 0) {
            args->output_format = FORMAT_HTML;
        } else if (strcmp(argv[i], "--by-file") == 0) {
            args->by_file = 1;
        } else if (strcmp(argv[i], "--by-file-by-lang") == 0) {
            args->by_file_by_lang = 1;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            args->quiet = 1;
        } else if (strncmp(argv[i], "--exclude-list-file=", 20) == 0) {
            args->exclude_list_file = strdup(argv[i] + 20);
        } else if (strncmp(argv[i], "--match-f=", 10) == 0) {
            args->match_pattern = strdup(argv[i] + 10);
        } else if (strncmp(argv[i], "--not-match-f=", 14) == 0) {
            args->not_match_pattern = strdup(argv[i] + 14);
        } else if (strncmp(argv[i], "--match-d=", 10) == 0) {
            args->match_d_pattern = strdup(argv[i] + 10);
        } else if (strncmp(argv[i], "--not-match-d=", 14) == 0) {
            args->not_match_d_pattern = strdup(argv[i] + 14);
        } else if (strncmp(argv[i], "--sql=", 6) == 0) {
            args->sql_file = strdup(argv[i] + 6);
            args->output_format = FORMAT_SQL;
        } else if (strncmp(argv[i], "--report-file=", 14) == 0) {
            args->report_file = strdup(argv[i] + 14);
        } else if (strncmp(argv[i], "--diff=", 7) == 0) {
            const char* diff_value = argv[i] + 7;
            // Look for .. separator
            const char* sep = strstr(diff_value, "..");
            if (sep != NULL) {
                // Range format: commit1..commit2
                size_t len1 = sep - diff_value;
                args->diff_commit1 = malloc(len1 + 1);
                if (args->diff_commit1) {
                    memcpy(args->diff_commit1, diff_value, len1);
                    args->diff_commit1[len1] = '\0';
                }
                args->diff_commit2 = strdup(sep + 2);
            } else {
                // Single commit: compare against HEAD
                args->diff_commit1 = strdup(diff_value);
                args->diff_commit2 = strdup("HEAD");
            }
        } else if (strncmp(argv[i], "--vcs=", 6) == 0) {
            const char* vcs_value = argv[i] + 6;
            if (strcmp(vcs_value, "git") == 0) {
                args->vcs = VCS_GIT;
            } else if (strcmp(vcs_value, "svn") == 0) {
                args->vcs = VCS_SVN;
            } else if (strcmp(vcs_value, "auto") == 0) {
                args->vcs = VCS_AUTO;
            } else {
                fprintf(stderr, "Error: Unknown VCS type '%s'\n", vcs_value);
                free(args->input_files);
                return -1;
            }
        } else if (strncmp(argv[i], "--max-file-size=", 16) == 0) {
            args->max_file_size_mb = atol(argv[i] + 16);
        } else if (strncmp(argv[i], "--exclude-dir=", 14) == 0) {
            // Parse comma-separated directories
            // Use a dummy tokenization on a copy since argv is const
            char* dir_str = strdup(argv[i] + 14);
            if (!dir_str) {
                free(args->input_files);
                return -1;
            }
            char* token = strtok(dir_str, ",");
            while (token != NULL) {
                // Allocate or grow the exclude_dirs array
                if (args->n_exclude_dirs == 0) {
                    int excl_capacity = 8;
                    args->exclude_dirs = malloc(excl_capacity * sizeof(char*));
                    if (!args->exclude_dirs) {
                        free(dir_str);
                        free(args->input_files);
                        return -1;
                    }
                } else if ((args->n_exclude_dirs & (args->n_exclude_dirs - 1)) == 0 &&
                           args->n_exclude_dirs >= 8) {
                    // Grow when count is a power of 2 >= 8
                    int new_cap = args->n_exclude_dirs * 2;
                    char** new_dirs = realloc(args->exclude_dirs, new_cap * sizeof(char*));
                    if (!new_dirs) {
                        for (int j = 0; j < args->n_exclude_dirs; j++) free(args->exclude_dirs[j]);
                        free(args->exclude_dirs);
                        free(dir_str);
                        free(args->input_files);
                        return -1;
                    }
                    args->exclude_dirs = new_dirs;
                }
                args->exclude_dirs[args->n_exclude_dirs] = strdup(token);
                if (!args->exclude_dirs[args->n_exclude_dirs]) {
                    for (int j = 0; j < args->n_exclude_dirs; j++) free(args->exclude_dirs[j]);
                    free(args->exclude_dirs);
                    free(dir_str);
                    free(args->input_files);
                    return -1;
                }
                args->n_exclude_dirs++;
                token = strtok(NULL, ",");
            }
            free(dir_str);
        } else if (strncmp(argv[i], "--include-lang=", 15) == 0) {
            // Parse comma-separated languages
            char* lang_str = strdup(argv[i] + 15);
            if (!lang_str) {
                free(args->input_files);
                return -1;
            }
            char* token = strtok(lang_str, ",");
            while (token != NULL) {
                // Allocate or grow the include_langs array
                if (args->n_include_langs == 0) {
                    int lang_capacity = 8;
                    args->include_langs = malloc(lang_capacity * sizeof(char*));
                    if (!args->include_langs) {
                        free(lang_str);
                        free(args->input_files);
                        return -1;
                    }
                } else if ((args->n_include_langs & (args->n_include_langs - 1)) == 0 &&
                           args->n_include_langs >= 8) {
                    int new_cap = args->n_include_langs * 2;
                    char** new_langs = realloc(args->include_langs, new_cap * sizeof(char*));
                    if (!new_langs) {
                        free(lang_str);
                        free(args->input_files);
                        return -1;
                    }
                    args->include_langs = new_langs;
                }
                args->include_langs[args->n_include_langs] = strdup(token);
                if (!args->include_langs[args->n_include_langs]) {
                    free(lang_str);
                    free(args->input_files);
                    return -1;
                }
                args->n_include_langs++;
                token = strtok(NULL, ",");
            }
            free(lang_str);
        } else if (strncmp(argv[i], "--exclude-lang=", 15) == 0) {
            // Parse comma-separated languages
            char* lang_str = strdup(argv[i] + 15);
            if (!lang_str) {
                free(args->input_files);
                return -1;
            }
            char* token = strtok(lang_str, ",");
            while (token != NULL) {
                // Allocate or grow the exclude_langs array
                if (args->n_exclude_langs == 0) {
                    int lang_capacity = 8;
                    args->exclude_langs = malloc(lang_capacity * sizeof(char*));
                    if (!args->exclude_langs) {
                        free(lang_str);
                        free(args->input_files);
                        return -1;
                    }
                } else if ((args->n_exclude_langs & (args->n_exclude_langs - 1)) == 0 &&
                           args->n_exclude_langs >= 8) {
                    int new_cap = args->n_exclude_langs * 2;
                    char** new_langs = realloc(args->exclude_langs, new_cap * sizeof(char*));
                    if (!new_langs) {
                        free(lang_str);
                        free(args->input_files);
                        return -1;
                    }
                    args->exclude_langs = new_langs;
                }
                args->exclude_langs[args->n_exclude_langs] = strdup(token);
                if (!args->exclude_langs[args->n_exclude_langs]) {
                    free(lang_str);
                    free(args->input_files);
                    return -1;
                }
                args->n_exclude_langs++;
                token = strtok(NULL, ",");
            }
            free(lang_str);
        } else if (strncmp(argv[i], "--include-ext=", 14) == 0) {
            // Parse comma-separated extensions
            char* ext_str = strdup(argv[i] + 14);
            if (!ext_str) {
                free(args->input_files);
                return -1;
            }
            char* token = strtok(ext_str, ",");
            while (token != NULL) {
                // Allocate or grow the include_exts array
                if (args->n_include_exts == 0) {
                    int ext_capacity = 8;
                    args->include_exts = malloc(ext_capacity * sizeof(char*));
                    if (!args->include_exts) {
                        free(ext_str);
                        free(args->input_files);
                        return -1;
                    }
                } else if ((args->n_include_exts & (args->n_include_exts - 1)) == 0 &&
                           args->n_include_exts >= 8) {
                    int new_cap = args->n_include_exts * 2;
                    char** new_exts = realloc(args->include_exts, new_cap * sizeof(char*));
                    if (!new_exts) {
                        free(ext_str);
                        free(args->input_files);
                        return -1;
                    }
                    args->include_exts = new_exts;
                }
                args->include_exts[args->n_include_exts] = strdup(token);
                if (!args->include_exts[args->n_include_exts]) {
                    free(ext_str);
                    free(args->input_files);
                    return -1;
                }
                args->n_include_exts++;
                token = strtok(NULL, ",");
            }
            free(ext_str);
        } else if (strncmp(argv[i], "--exclude-ext=", 14) == 0) {
            // Parse comma-separated extensions
            char* ext_str = strdup(argv[i] + 14);
            if (!ext_str) {
                free(args->input_files);
                return -1;
            }
            char* token = strtok(ext_str, ",");
            while (token != NULL) {
                // Allocate or grow the exclude_exts array
                if (args->n_exclude_exts == 0) {
                    int ext_capacity = 8;
                    args->exclude_exts = malloc(ext_capacity * sizeof(char*));
                    if (!args->exclude_exts) {
                        free(ext_str);
                        free(args->input_files);
                        return -1;
                    }
                } else if ((args->n_exclude_exts & (args->n_exclude_exts - 1)) == 0 &&
                           args->n_exclude_exts >= 8) {
                    int new_cap = args->n_exclude_exts * 2;
                    char** new_exts = realloc(args->exclude_exts, new_cap * sizeof(char*));
                    if (!new_exts) {
                        free(ext_str);
                        free(args->input_files);
                        return -1;
                    }
                    args->exclude_exts = new_exts;
                }
                args->exclude_exts[args->n_exclude_exts] = strdup(token);
                if (!args->exclude_exts[args->n_exclude_exts]) {
                    free(ext_str);
                    free(args->input_files);
                    return -1;
                }
                args->n_exclude_exts++;
                token = strtok(NULL, ",");
            }
            free(ext_str);
        } else if (strncmp(argv[i], "--max-temp-size=", 16) == 0) {
            long mb = atol(argv[i] + 16);
            if (mb > 0) {
                args->max_temp_size = (size_t)mb * 1024 * 1024;
            }
        } else if (strncmp(argv[i], "--sdir=", 7) == 0) {
            args->staging_dir = strdup(argv[i] + 7);
        } else if (strncmp(argv[i], "--skip-leading=", 15) == 0) {
            char* spec = strdup(argv[i] + 15);
            if (!spec) { free(args->input_files); return -1; }
            char* comma = strchr(spec, ',');
            if (comma) { *comma = '\0'; }
            args->skip_leading = (int)strtol(spec, NULL, 10);
            if (comma) {
                char* token = strtok(comma + 1, ",");
                while (token) {
                    if (args->n_skip_leading_exts == 0) {
                        args->skip_leading_exts = malloc(8 * sizeof(char*));
                        if (!args->skip_leading_exts) { free(spec); free(args->input_files); return -1; }
                    } else if ((args->n_skip_leading_exts & (args->n_skip_leading_exts - 1)) == 0 &&
                               args->n_skip_leading_exts >= 8) {
                        char** ne = realloc(args->skip_leading_exts, args->n_skip_leading_exts * 2 * sizeof(char*));
                        if (!ne) { free(spec); free(args->input_files); return -1; }
                        args->skip_leading_exts = ne;
                    }
                    args->skip_leading_exts[args->n_skip_leading_exts] = strdup(token);
                    if (!args->skip_leading_exts[args->n_skip_leading_exts]) { free(spec); free(args->input_files); return -1; }
                    args->n_skip_leading_exts++;
                    token = strtok(NULL, ",");
                }
            }
            free(spec);
        } else if (strncmp(argv[i], "--progress-rate=", 16) == 0) {
            args->progress_rate = atoi(argv[i] + 16);
            if (args->progress_rate < 1) args->progress_rate = 1;
        } else if (strcmp(argv[i], "--show-lang") == 0) {
            args->show_lang = 1;
        } else if (strncmp(argv[i], "--show-lang=", 12) == 0) {
            args->show_lang = 1;
            args->show_lang_arg = strdup(argv[i] + 12);
        } else if (strcmp(argv[i], "--show-ext") == 0) {
            args->show_ext = 1;
        } else if (strncmp(argv[i], "--show-ext=", 11) == 0) {
            args->show_ext = 1;
            args->show_ext_arg = strdup(argv[i] + 11);
        } else if (strcmp(argv[i], "--skip-uniqueness") == 0) {
            args->skip_uniqueness = 1;
        } else if (strncmp(argv[i], "--unique=", 9) == 0) {
            args->unique_file = strdup(argv[i] + 9);
        } else if (strncmp(argv[i], "--ignored=", 10) == 0) {
            args->ignored_file = strdup(argv[i] + 10);
        } else if (strcmp(argv[i], "--no-config") == 0) {
            args->no_config = 1;
        } else if (strncmp(argv[i], "--config=", 9) == 0) {
            args->config_file = strdup(argv[i] + 9);
        } else if (strncmp(argv[i], "--processes=", 12) == 0) {
            args->processes = atoi(argv[i] + 12);
        } else if (strncmp(argv[i], "--batch-input=", 14) == 0) {
            args->batch_input = strdup(argv[i] + 14);
        } else if (strcmp(argv[i], "--batch-output=tsv") == 0) {
            args->batch_output_tsv = 1;
        } else if (strncmp(argv[i], "--extract-with=", 15) == 0) {
            args->extract_with = strdup(argv[i] + 15);
        } else if (strncmp(argv[i], "--skip-archive=", 15) == 0) {
            args->skip_archive = strdup(argv[i] + 15);
        } else if (strncmp(argv[i], "--max-archive-depth=", 20) == 0) {
            args->max_archive_depth = atoi(argv[i] + 20);
        } else if (strncmp(argv[i], "--git=", 6) == 0) {
            args->git_ref = strdup(argv[i] + 6);
        } else if (strncmp(argv[i], "--list-file=", 12) == 0) {
            args->list_file = strdup(argv[i] + 12);
        } else if (strncmp(argv[i], "--force-lang=", 13) == 0) {
            args->force_lang = strdup(argv[i] + 13);
        } else {
            // Treat as input file
            if (args->n_input_files >= capacity) {
                capacity *= 2;
                char** new_files = realloc(args->input_files, capacity * sizeof(char*));
                if (!new_files) {
                    free(args->input_files);
                    args->input_files = NULL;
                    return -1;
                }
                args->input_files = new_files;
            }
            args->input_files[args->n_input_files] = argv[i];
            args->n_input_files++;
        }
    }

    // Return error if no input files and no flags set (but allow --list-file to provide files later)
    if (args->n_input_files == 0 && !args->show_help && !args->show_version && !args->show_lang && !args->show_ext && !args->list_file) {
        free(args->input_files);
        args->input_files = NULL;
        return -1;
    }

    // Set default output format if not set by config or CLI
    if (args->output_format == 0) {
        args->output_format = FORMAT_TEXT;
    }

    return 0;
}

void cli_free(CliArgs* args) {
    if (args->input_files) {
        free(args->input_files);
        args->input_files = NULL;
    }
    args->n_input_files = 0;

    if (args->exclude_dirs) {
        for (int i = 0; i < args->n_exclude_dirs; i++) {
            free(args->exclude_dirs[i]);
        }
        free(args->exclude_dirs);
        args->exclude_dirs = NULL;
    }
    args->n_exclude_dirs = 0;

    if (args->include_langs) {
        for (int i = 0; i < args->n_include_langs; i++) {
            free(args->include_langs[i]);
        }
        free(args->include_langs);
        args->include_langs = NULL;
    }
    args->n_include_langs = 0;

    if (args->exclude_langs) {
        for (int i = 0; i < args->n_exclude_langs; i++) {
            free(args->exclude_langs[i]);
        }
        free(args->exclude_langs);
        args->exclude_langs = NULL;
    }
    args->n_exclude_langs = 0;

    if (args->include_exts) {
        for (int i = 0; i < args->n_include_exts; i++) {
            free(args->include_exts[i]);
        }
        free(args->include_exts);
        args->include_exts = NULL;
    }
    args->n_include_exts = 0;

    if (args->exclude_exts) {
        for (int i = 0; i < args->n_exclude_exts; i++) {
            free(args->exclude_exts[i]);
        }
        free(args->exclude_exts);
        args->exclude_exts = NULL;
    }
    args->n_exclude_exts = 0;

    if (args->diff_commit1) {
        free(args->diff_commit1);
        args->diff_commit1 = NULL;
    }
    if (args->diff_commit2) {
        free(args->diff_commit2);
        args->diff_commit2 = NULL;
    }
    if (args->exclude_list_file) {
        free(args->exclude_list_file);
        args->exclude_list_file = NULL;
    }
    if (args->match_pattern) {
        free(args->match_pattern);
        args->match_pattern = NULL;
    }
    if (args->not_match_pattern) {
        free(args->not_match_pattern);
        args->not_match_pattern = NULL;
    }
    if (args->match_d_pattern) {
        free(args->match_d_pattern);
        args->match_d_pattern = NULL;
    }
    if (args->not_match_d_pattern) {
        free(args->not_match_d_pattern);
        args->not_match_d_pattern = NULL;
    }
    if (args->sql_file) {
        free(args->sql_file);
        args->sql_file = NULL;
    }
    if (args->report_file) {
        free(args->report_file);
        args->report_file = NULL;
    }
    if (args->staging_dir) {
        free(args->staging_dir);
        args->staging_dir = NULL;
    }
    if (args->skip_leading_exts) {
        for (int i = 0; i < args->n_skip_leading_exts; i++) {
            free(args->skip_leading_exts[i]);
        }
        free(args->skip_leading_exts);
        args->skip_leading_exts = NULL;
    }
    args->n_skip_leading_exts = 0;
    if (args->config_file) { free(args->config_file); args->config_file = NULL; }
    if (args->show_lang_arg) { free(args->show_lang_arg); args->show_lang_arg = NULL; }
    if (args->show_ext_arg) { free(args->show_ext_arg); args->show_ext_arg = NULL; }
    if (args->unique_file) { free(args->unique_file); args->unique_file = NULL; }
    if (args->ignored_file) { free(args->ignored_file); args->ignored_file = NULL; }
    if (args->batch_input) { free(args->batch_input); args->batch_input = NULL; }
    if (args->extract_with) { free(args->extract_with); args->extract_with = NULL; }
    if (args->skip_archive) { free(args->skip_archive); args->skip_archive = NULL; }
    if (args->git_ref) { free(args->git_ref); args->git_ref = NULL; }
    if (args->list_file) { free(args->list_file); args->list_file = NULL; }
    if (args->force_lang) { free(args->force_lang); args->force_lang = NULL; }
}

void cli_print_help(const char* prog_name) {
    printf("Usage: %s [options] <file|dir> [file|dir ...]\n", prog_name);
    printf("Options:\n");
    printf("  --help              Show this help message\n");
    printf("  --version           Show version information\n");
    printf("  --no-recurse        Don't recurse into subdirectories\n");
    printf(
        "  --max-file-size=MB  Skip files larger than N megabytes (default: "
        "100)\n");
    printf("  --exclude-dir=DIR   Exclude directories (comma-separated)\n");
    printf(
        "  --include-lang=LANG Include only specified languages "
        "(comma-separated)\n");
    printf("  --exclude-lang=LANG Exclude specified languages (comma-separated)\n");
    printf(
        "  --include-ext=EXT   Include only specified extensions "
        "(comma-separated)\n");
    printf("  --exclude-ext=EXT   Exclude specified extensions (comma-separated)\n");
    printf("  --exclude-list-file=FILE Read exclude patterns from file (one per line)\n");
    printf("  --match-f=PATTERN   Include only files matching regex pattern\n");
    printf("  --not-match-f=PATTERN Exclude files matching regex pattern\n");
    printf("  --match-d=PATTERN   Include only directories matching regex pattern\n");
    printf("  --not-match-d=PATTERN Exclude directories matching regex pattern\n");
    printf("  --sql=FILE          Output in SQLite SQL format (\"-\" for stdout)\n");
    printf("  --json              Output in JSON format\n");
    printf("  --csv               Output in CSV format\n");
    printf("  --md                Output in Markdown format\n");
    printf("  --yaml              Output in YAML format\n");
    printf("  --xml               Output in XML format\n");
    printf("  --html              Output in HTML format\n");
    printf("  --by-file           Report statistics for each file\n");
    printf("  --by-file-by-lang   Report by file and language\n");
    printf("  --report-file=FILE  Write output to file instead of stdout\n");
    printf("  --vcs=git|svn|auto  Use VCS to get file list (respects .gitignore)\n");
    printf("  --diff=COMMIT..COMMIT Compare changes between commits (git diff)\n");
    printf("  --quiet             Suppress warning messages\n");
    printf("  --max-temp-size=MB  Limit temp space usage in MB (default: 1024)\n");
    printf("  --sdir=DIR          Use specified directory for temp files\n");
    printf("  --skip-leading=N,ext1,ext2  Skip N leading lines for given extensions\n");
    printf("  --show-lang[=NAME]  Show supported languages (optionally filter by name)\n");
    printf("  --show-ext[=EXT]    Show extension to language mapping (optionally filter by ext)\n");
    printf("  --config=FILE       Load options from FILE (default: ~/.config/rloc/options.txt)\n");
    printf("  --no-config         Do not load any config file\n");
    printf("  --skip-uniqueness   Skip duplicate file detection via MD5\n");
    printf("  --unique=FILE       Write unique (non-duplicate) file list to FILE\n");
    printf("  --ignored=FILE      Write ignored file list to FILE\n");
    printf("  --processes=N       Use N parallel processes (0 = auto detect)\n");
    printf("  --git=REF           Count files at git commit/branch/tag\n");
    printf("  --list-file=FILE    Read input file list from FILE (- for STDIN)\n");
    printf("  --force-lang=LANG   Force language for all files (or LANG,EXT for specific ext)\n");
    printf("  --extract-with=CMD  Custom archive extraction command\n");
    printf("  --skip-archive=REGEX Skip archives matching pattern\n");
    printf("  --max-archive-depth=N Maximum archive nesting depth (default 3)\n");
}

void cli_print_version(void) {
    printf("rloc 0.1.0 (24 languages, built on %s %s)\n", __DATE__, __TIME__);
}

/* Pre-scan argv for --config= and --no-config only.
 * Returns 0 on success.
 */
int cli_prescan_config(int argc, char** argv, CliArgs* args) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-config") == 0) {
            args->no_config = 1;
        } else if (strncmp(argv[i], "--config=", 9) == 0) {
            args->config_file = strdup(argv[i] + 9);
            if (!args->config_file) return -1;
        }
    }
    return 0;
}
