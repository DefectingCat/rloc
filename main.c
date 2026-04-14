#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "cli.h"
#include "counter.h"
#include "filelist.h"
#include "language.h"
#include "output.h"
#include "strlit.h"
#include "util.h"

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

    // Process each input path
    int error_count = 0;
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

        // Print warning for unrecognized files
        if (lang == NULL) {
            fprintf(stderr, "Warning: Unrecognized language for file '%s'\n", filepath);
        }
    }

    // Calculate elapsed time
    clock_t end_time = clock();
    double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Output results
    output_text(files, filelist.count, elapsed_sec);

    // Cleanup
    free(files);
    filelist_free(&filelist);
    cli_free(&args);

    return (error_count > 0) ? 1 : 0;
}