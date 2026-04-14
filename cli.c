#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cli_parse(int argc, char** argv, CliArgs* args) {
    // Initialize structure
    args->input_files = NULL;
    args->n_input_files = 0;
    args->show_help = 0;
    args->show_version = 0;
    args->no_recurse = 0;
    args->max_file_size_mb = 0;
    args->exclude_dirs = NULL;
    args->n_exclude_dirs = 0;
    args->include_langs = NULL;
    args->n_include_langs = 0;
    args->exclude_langs = NULL;
    args->n_exclude_langs = 0;
    args->include_exts = NULL;
    args->n_include_exts = 0;
    args->exclude_exts = NULL;
    args->n_exclude_exts = 0;

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

    // Return error if no input files and no flags set
    if (args->n_input_files == 0 && !args->show_help && !args->show_version) {
        free(args->input_files);
        args->input_files = NULL;
        return -1;
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
}

void cli_print_version(void) { printf("rloc 0.1.0\n"); }
