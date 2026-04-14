#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "cli.h"
#include "counter.h"
#include "diff.h"
#include "filelist.h"
#include "language.h"
#include "output.h"
#include "util.h"
#include "vcs.h"

/* Helper: Get file extension from path */
static const char* get_extension(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext && ext != filepath) {
        return ext + 1;  // Skip the dot
    }
    return NULL;
}

/* Helper: Check if a string is in a string array */
static int is_in_string_array(const char* str, char** array, int n) {
    for (int i = 0; i < n; i++) {
        if (strcasecmp(str, array[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Helper: Check if extension matches any in array */
static int extension_matches(const char* ext, char** array, int n) {
    if (!ext) return 0;

    for (int i = 0; i < n; i++) {
        // Remove leading dot if present
        const char* pattern = array[i];
        if (pattern[0] == '.') {
            pattern++;
        }

        if (strcasecmp(ext, pattern) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    CliArgs args;

    // Parse command line arguments
    if (cli_parse(argc, argv, &args) != 0) {
        fprintf(stderr, "Error: No input files specified\n");
        cli_print_help(argv[0]);
        return 1;
    }

    // Handle --help flag
    if (args.show_help) {
        cli_print_help(argv[0]);
        cli_free(&args);
        return 0;
    }

    // Handle --version flag
    if (args.show_version) {
        cli_print_version();
        cli_free(&args);
        return 0;
    }

    // Handle --diff mode (git diff between commits)
    if (args.diff_commit1 != NULL && args.diff_commit2 != NULL) {
        // Get diff files from the first input path (or current directory)
        const char* repo_path = (args.n_input_files > 0) ? args.input_files[0] : ".";
        if (!is_directory(repo_path)) {
            repo_path = ".";
        }

        int n_diff_files = 0;
        DiffFileStats* diff_files =
            diff_get_files(repo_path, args.diff_commit1, args.diff_commit2, &n_diff_files);

        if (!diff_files || n_diff_files == 0) {
            printf("No changes between %s and %s\n", args.diff_commit1, args.diff_commit2);
            cli_free(&args);
            return 0;
        }

        // Output diff results
        output_diff(diff_files, n_diff_files, args.diff_commit1, args.diff_commit2, args.by_file);

        diff_free_files(diff_files, n_diff_files);
        cli_free(&args);
        return 0;
    }

    // Create FilelistConfig from CliArgs
    FilelistConfig config;
    config.no_recurse = args.no_recurse;
    config.max_file_size = (args.max_file_size_mb > 0 ? args.max_file_size_mb : 100) * 1024 * 1024;
    config.exclude_dirs = args.exclude_dirs;
    config.n_exclude_dirs = args.n_exclude_dirs;

    // Initialize FileList
    FileList filelist;
    filelist_init(&filelist);

    // Start timer
    clock_t start_time = clock();

    // Resolve VCS type if auto-detect
    VcsType effective_vcs = args.vcs;
    if (args.vcs == VCS_AUTO) {
        // Auto-detect VCS for each input path
        for (int i = 0; i < args.n_input_files; i++) {
            const char* path = args.input_files[i];
            if (is_directory(path)) {
                effective_vcs = vcs_detect(path);
                if (effective_vcs != VCS_NONE) {
                    break;
                }
            }
        }
    }

    // Process each input path
    int error_count = 0;

    // Handle VCS-based file discovery
    if (effective_vcs == VCS_GIT || effective_vcs == VCS_SVN) {
        // Use VCS for file discovery
        for (int i = 0; i < args.n_input_files; i++) {
            const char* path = args.input_files[i];

            if (is_directory(path)) {
                // Get files from VCS
                int n_vcs_files = 0;
                char** vcs_files = NULL;

                if (effective_vcs == VCS_GIT) {
                    vcs_files = vcs_get_files_git(path, &n_vcs_files);
                } else if (effective_vcs == VCS_SVN) {
                    vcs_files = vcs_get_files_svn(path, &n_vcs_files);
                }

                if (!vcs_files || n_vcs_files == 0) {
                    fprintf(stderr, "Error: Cannot get %s files from '%s'\n",
                            effective_vcs == VCS_GIT ? "git" : "svn", path);
                    error_count++;
                    continue;
                }

                // Add files to filelist
                for (int j = 0; j < n_vcs_files; j++) {
                    if (filelist.count >= filelist.capacity) {
                        int new_capacity = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                        char** new_paths = realloc(filelist.paths, new_capacity * sizeof(char*));
                        if (!new_paths) {
                            fprintf(stderr, "Error: Memory allocation failed\n");
                            vcs_free_files(vcs_files, n_vcs_files);
                            filelist_free(&filelist);
                            cli_free(&args);
                            return 1;
                        }
                        filelist.paths = new_paths;
                        filelist.capacity = new_capacity;
                    }
                    // Build full path
                    char full_path[2048];
                    snprintf(full_path, sizeof(full_path), "%s/%s", path, vcs_files[j]);
                    filelist.paths[filelist.count] = strdup(full_path);
                    if (!filelist.paths[filelist.count]) {
                        fprintf(stderr, "Error: Memory allocation failed\n");
                        vcs_free_files(vcs_files, n_vcs_files);
                        filelist_free(&filelist);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.count++;
                }
                vcs_free_files(vcs_files, n_vcs_files);
            } else {
                // Single file - add directly
                fprintf(stderr,
                        "Warning: --vcs only works on directories, treating '%s' as regular file\n",
                        path);
                if (filelist.count >= filelist.capacity) {
                    int new_capacity = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                    char** new_paths = realloc(filelist.paths, new_capacity * sizeof(char*));
                    if (!new_paths) {
                        fprintf(stderr, "Error: Memory allocation failed\n");
                        filelist_free(&filelist);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.paths = new_paths;
                    filelist.capacity = new_capacity;
                }
                filelist.paths[filelist.count] = strdup(path);
                if (!filelist.paths[filelist.count]) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    filelist_free(&filelist);
                    cli_free(&args);
                    return 1;
                }
                filelist.count++;
            }
        }
    } else {
        // Regular file scanning (non-VCS mode)
        for (int i = 0; i < args.n_input_files; i++) {
            const char* path = args.input_files[i];

            if (is_directory(path)) {
                // Scan directory recursively or non-recursively
                if (filelist_scan(path, &config, &filelist) != 0) {
                    fprintf(stderr, "Error: Cannot scan directory '%s'\n", path);
                    error_count++;
                }
            } else if (is_regular_file(path)) {
                // Add single file directly
                if (filelist.count >= filelist.capacity) {
                    // Expand capacity (simple implementation: double it)
                    int new_capacity = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                    char** new_paths = realloc(filelist.paths, new_capacity * sizeof(char*));
                    if (!new_paths) {
                        fprintf(stderr, "Error: Memory allocation failed\n");
                        filelist_free(&filelist);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.paths = new_paths;
                    filelist.capacity = new_capacity;
                }
                // Duplicate the path string
                filelist.paths[filelist.count] = strdup(path);
                if (!filelist.paths[filelist.count]) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    filelist_free(&filelist);
                    cli_free(&args);
                    return 1;
                }
                filelist.count++;
            } else {
                fprintf(stderr, "Error: Path '%s' is not a valid file or directory\n", path);
                error_count++;
            }
        }
    }

    // Allocate array for file statistics
    FileStats* files = malloc(filelist.count * sizeof(FileStats));
    if (!files) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        filelist_free(&filelist);
        cli_free(&args);
        return 1;
    }

    // Count each discovered file
    for (int i = 0; i < filelist.count; i++) {
        const char* filepath = filelist.paths[i];

        // Detect language for this file
        const Language* lang = detect_language(filepath);

        // Apply --include-lang filter
        if (args.n_include_langs > 0) {
            if (lang == NULL ||
                !is_in_string_array(lang->name, args.include_langs, args.n_include_langs)) {
                // Skip this file
                files[i].filepath = filepath;
                files[i].lang = NULL;
                files[i].counts.blank = 0;
                files[i].counts.comment = 0;
                files[i].counts.code = 0;
                files[i].counts.total = 0;
                continue;
            }
        }

        // Apply --exclude-lang filter
        if (lang != NULL && args.n_exclude_langs > 0) {
            if (is_in_string_array(lang->name, args.exclude_langs, args.n_exclude_langs)) {
                // Skip this file
                files[i].filepath = filepath;
                files[i].lang = NULL;
                files[i].counts.blank = 0;
                files[i].counts.comment = 0;
                files[i].counts.code = 0;
                files[i].counts.total = 0;
                continue;
            }
        }

        // Apply --include-ext filter
        if (args.n_include_exts > 0) {
            const char* ext = get_extension(filepath);
            if (!extension_matches(ext, args.include_exts, args.n_include_exts)) {
                // Skip this file
                files[i].filepath = filepath;
                files[i].lang = NULL;
                files[i].counts.blank = 0;
                files[i].counts.comment = 0;
                files[i].counts.code = 0;
                files[i].counts.total = 0;
                continue;
            }
        }

        // Apply --exclude-ext filter
        if (args.n_exclude_exts > 0) {
            const char* ext = get_extension(filepath);
            if (extension_matches(ext, args.exclude_exts, args.n_exclude_exts)) {
                // Skip this file
                files[i].filepath = filepath;
                files[i].lang = NULL;
                files[i].counts.blank = 0;
                files[i].counts.comment = 0;
                files[i].counts.code = 0;
                files[i].counts.total = 0;
                continue;
            }
        }

        // Count lines with language awareness
        files[i].filepath = filepath;
        files[i].lang = lang;

        if (count_file_with_lang(filepath, lang, &files[i].counts) != 0) {
            fprintf(stderr, "Error: Cannot read file '%s'\n", filepath);
            files[i].counts.blank = 0;
            files[i].counts.comment = 0;
            files[i].counts.code = 0;
            files[i].counts.total = 0;
            error_count++;
        }

        // Print warning for unrecognized files (unless --quiet)
        if (lang == NULL && !args.quiet) {
            fprintf(stderr, "Warning: Unrecognized language for file '%s'\n", filepath);
        }
    }

    // Calculate elapsed time
    clock_t end_time = clock();
    double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Output results
    switch (args.output_format) {
        case FORMAT_JSON:
            if (args.by_file) {
                output_json_by_file(files, filelist.count, elapsed_sec);
            } else {
                output_json(files, filelist.count, elapsed_sec);
            }
            break;
        case FORMAT_CSV:
            if (args.by_file) {
                output_csv_by_file(files, filelist.count, elapsed_sec);
            } else {
                output_csv(files, filelist.count, elapsed_sec);
            }
            break;
        case FORMAT_MD:
            if (args.by_file) {
                output_md_by_file(files, filelist.count, elapsed_sec);
            } else {
                output_md(files, filelist.count, elapsed_sec);
            }
            break;
        case FORMAT_TEXT:
        default:
            if (args.by_file) {
                output_text_by_file(files, filelist.count, elapsed_sec);
            } else {
                output_text(files, filelist.count, elapsed_sec);
            }
            break;
    }

    // Cleanup
    free(files);
    filelist_free(&filelist);
    cli_free(&args);

    return (error_count > 0) ? 1 : 0;
}