#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "archive.h"
#include "cli.h"
#include "config.h"
#include "counter.h"
#include "diff.h"
#include "filelist.h"
#include "lang_defs.h"
#include "language.h"
#include "output.h"
#include "parallel.h"
#include "temp_manager.h"
#include "threaded_counter.h"
#include "unique.h"
#include "util.h"
#include "vcs.h"

static const char* get_extension(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext && ext != filepath) return ext + 1;
    return NULL;
}

static int is_in_string_array(const char* str, char** array, int n) {
    for (int i = 0; i < n; i++) {
        if (strcasecmp(str, array[i]) == 0) return 1;
    }
    return 0;
}

static int extension_matches(const char* ext, char** array, int n) {
    if (!ext) return 0;
    for (int i = 0; i < n; i++) {
        const char* p = array[i];
        if (p[0] == '.') p++;
        if (strcasecmp(ext, p) == 0) return 1;
    }
    return 0;
}

/* Pre-scan argv for --config and --no-config */
static int check_no_config(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-config") == 0) return 1;
    }
    return 0;
}

static char* get_config_file(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--config=", 9) == 0) {
            return strdup(argv[i] + 9);
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    // Load config before parsing CLI (unless --no-config)
    if (!check_no_config(argc, argv)) {
        char* config_path = get_config_file(argc, argv);
        if (!config_path) config_path = config_get_default_path();
        if (config_path) {
            config_load(config_path, &args);
            free(config_path);
        }
    }

    if (cli_parse(argc, argv, &args) != 0) {
        fprintf(stderr, "Error: No input files specified\n");
        cli_print_help(argv[0]);
        return 1;
    }

    if (args.show_help) {
        cli_print_help(argv[0]);
        cli_free(&args);
        return 0;
    }
    if (args.show_version) {
        cli_print_version();
        cli_free(&args);
        return 0;
    }
    if (args.show_lang) {
        lang_show(args.show_lang_arg);
        cli_free(&args);
        return 0;
    }
    if (args.show_ext) {
        ext_show(args.show_ext_arg);
        cli_free(&args);
        return 0;
    }
    if (args.explain_lang) {
        explain_language(args.explain_lang);
        cli_free(&args);
        return 0;
    }

    // Validate mutually exclusive options
    if (args.strip_comments && args.strip_code) {
        fprintf(stderr, "Error: --strip-comments and --strip-code are mutually exclusive\n");
        cli_free(&args);
        return 1;
    }

    // Handle --list-file: read input paths from file or STDIN
    if (args.list_file != NULL) {
        FILE* list_fp = NULL;
        if (strcmp(args.list_file, "-") == 0) {
            list_fp = stdin;
        } else {
            list_fp = fopen(args.list_file, "r");
            if (!list_fp) {
                fprintf(stderr, "Error: Cannot open list file '%s'\n", args.list_file);
                cli_free(&args);
                return 1;
            }
        }

        // Read paths from file and add to args.input_files
        char line[2048];
        while (fgets(line, sizeof(line), list_fp) != NULL) {
            // Remove trailing newline
            size_t len = strlen(line);
            while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
                line[len - 1] = '\0';
                len--;
            }
            if (len == 0) continue;  // Skip empty lines

            // Grow input_files array if needed
            int capacity = args.n_input_files > 0 ? args.n_input_files * 2 : 16;
            char** new_files = realloc(args.input_files, capacity * sizeof(char*));
            if (!new_files) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                if (list_fp != stdin) fclose(list_fp);
                cli_free(&args);
                return 1;
            }
            args.input_files = new_files;
            args.input_files[args.n_input_files] = strdup(line);
            if (!args.input_files[args.n_input_files]) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                if (list_fp != stdin) fclose(list_fp);
                cli_free(&args);
                return 1;
            }
            args.n_input_files++;
        }

        if (list_fp != stdin) fclose(list_fp);

        // Check if we have any input files now
        if (args.n_input_files == 0) {
            fprintf(stderr, "Error: No input files found in list file '%s'\n", args.list_file);
            cli_free(&args);
            return 1;
        }
    }

    // Initialize temp manager for archive extraction
    TempManager temp_mgr;
    size_t max_temp = args.max_temp_size > 0 ? args.max_temp_size : (1024 * 1024 * 1024);
    if (temp_manager_create(&temp_mgr, max_temp) != 0) {
        fprintf(stderr, "Error: Failed to create temp manager\n");
        cli_free(&args);
        return 1;
    }
    temp_manager_install_handlers(&temp_mgr);

    // Unified diff handling using new diff_get_stats_extended API
    if (args.diff_refs != NULL || (args.diff_commit1 != NULL && args.diff_commit2 != NULL)) {
        const char* repo_path = (args.n_input_files > 0) ? args.input_files[0] : ".";
        if (!is_directory(repo_path)) repo_path = ".";

        DiffConfig diff_cfg;
        memset(&diff_cfg, 0, sizeof(diff_cfg));
        diff_cfg.repo_path = repo_path;

        // Use new unified fields if available, otherwise fall back to legacy fields
        if (args.diff_refs != NULL) {
            diff_cfg.ref1 = args.diff_commit1;
            diff_cfg.ref2 = args.diff_commit2;
            diff_cfg.flags = args.diff_flags;
        } else {
            // Legacy --diff format without mode
            diff_cfg.ref1 = args.diff_commit1;
            diff_cfg.ref2 = args.diff_commit2;
            diff_cfg.flags = DIFF_MODE_RELATIVE;
        }

        int n_diff_files = 0;
        DiffFileStats* diff_files = diff_get_stats_extended(&diff_cfg, &n_diff_files);

        if (!diff_files || n_diff_files == 0) {
            printf("No changes between %s and %s\n", diff_cfg.ref1, diff_cfg.ref2);
            diff_free_config(&diff_cfg);
            cli_free(&args);
            return 0;
        }

        // Output based on mode flags
        if (diff_cfg.flags & DIFF_SHOW_ALIGNMENT) {
            // Build alignment entries from diff stats
            AlignmentEntry* entries = malloc(n_diff_files * sizeof(AlignmentEntry));
            if (entries) {
                for (int i = 0; i < n_diff_files; i++) {
                    entries[i].filepath = diff_files[i].filepath;
                    entries[i].type =
                        (diff_files[i].added > 0 && diff_files[i].removed == 0)   ? ALIGN_ADDED
                        : (diff_files[i].removed > 0 && diff_files[i].added == 0) ? ALIGN_REMOVED
                                                                                  : ALIGN_MODIFIED;
                    entries[i].added = diff_files[i].added;
                    entries[i].removed = diff_files[i].removed;
                    entries[i].language = diff_files[i].lang ? diff_files[i].lang : "(unknown)";
                }
                int json_output = (args.output_format == FORMAT_JSON);
                output_alignment(entries, n_diff_files, diff_cfg.ref1, diff_cfg.ref2, json_output);
                free(entries);
            }
        } else {
            output_diff(diff_files, n_diff_files, diff_cfg.ref1, diff_cfg.ref2, args.by_file);
        }

        diff_free_files(diff_files, n_diff_files);
        diff_free_config(&diff_cfg);
        cli_free(&args);
        return 0;
    }

    FilelistConfig config;
    config.no_recurse = args.no_recurse;
    config.max_file_size = (args.max_file_size_mb > 0 ? args.max_file_size_mb : 100) * 1024 * 1024;
    config.exclude_dirs = args.exclude_dirs;
    config.n_exclude_dirs = args.n_exclude_dirs;
    config.exclude_patterns = NULL;
    config.n_exclude_patterns = 0;
    config.match_pattern = args.match_pattern;
    config.not_match_pattern = args.not_match_pattern;
    config.match_d_pattern = args.match_d_pattern;
    config.not_match_d_pattern = args.not_match_d_pattern;
    config.fullpath = args.fullpath;
    config.follow_links = args.follow_links;

    if (args.exclude_list_file) {
        if (filelist_load_exclude_patterns(args.exclude_list_file, &config) != 0) {
            fprintf(stderr, "Error: Cannot load exclude patterns from '%s'\n",
                    args.exclude_list_file);
            cli_free(&args);
            return 1;
        }
    }

    FileList filelist;
    filelist_init(&filelist);

    clock_t start_time = clock();

    // Handle --git mode: count files at specific commit/branch/tag
    if (args.git_ref != NULL) {
        const char* repo_path = (args.n_input_files > 0) ? args.input_files[0] : ".";
        if (!is_directory(repo_path)) repo_path = ".";

        int n_git_files = 0;
        char** git_files = vcs_get_files_at_commit(repo_path, args.git_ref, &n_git_files);
        if (!git_files || n_git_files == 0) {
            fprintf(stderr, "Error: Cannot get files at git ref '%s'\n", args.git_ref);
            cli_free(&args);
            return 1;
        }

        // Create temp directory for extracted files
        char* git_extract_dir = temp_manager_create_dir(&temp_mgr, "git_extract");
        if (!git_extract_dir) {
            fprintf(stderr, "Error: Cannot create temp directory for git extraction\n");
            vcs_free_files(git_files, n_git_files);
            cli_free(&args);
            return 1;
        }

        // Extract each file from git and add to filelist
        for (int j = 0; j < n_git_files; j++) {
            size_t content_len = 0;
            char* content =
                vcs_get_file_at_commit(repo_path, args.git_ref, git_files[j], &content_len);
            if (content && content_len > 0) {
                // Write to temp file
                char temp_path[2048];
                snprintf(temp_path, sizeof(temp_path), "%s/%s", git_extract_dir, git_files[j]);

                // Create subdirectories if needed
                char* last_slash = strrchr(temp_path, '/');
                if (last_slash && last_slash != temp_path) {
                    char dir_path[2048];
                    strncpy(dir_path, temp_path, last_slash - temp_path);
                    dir_path[last_slash - temp_path] = '\0';
                    // Simple mkdir -p using system call
                    char mkdir_cmd[2048];
                    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s' 2>/dev/null", dir_path);
                    system(mkdir_cmd);
                }

                FILE* fp = fopen(temp_path, "w");
                if (fp) {
                    fwrite(content, 1, content_len, fp);
                    fclose(fp);

                    if (filelist.count >= filelist.capacity) {
                        int nc = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                        char** np = realloc(filelist.paths, nc * sizeof(char*));
                        if (!np) {
                            free(content);
                            vcs_free_files(git_files, n_git_files);
                            filelist_free(&filelist);
                            temp_manager_destroy(&temp_mgr);
                            cli_free(&args);
                            return 1;
                        }
                        filelist.paths = np;
                        filelist.capacity = nc;
                    }
                    filelist.paths[filelist.count] = strdup(temp_path);
                    if (!filelist.paths[filelist.count]) {
                        free(content);
                        vcs_free_files(git_files, n_git_files);
                        filelist_free(&filelist);
                        temp_manager_destroy(&temp_mgr);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.count++;
                }
                free(content);
            }
        }
        vcs_free_files(git_files, n_git_files);

        // Skip normal file scanning for --git mode
        goto skip_normal_scan;
    }

    VcsType effective_vcs = args.vcs;
    if (args.vcs == VCS_AUTO) {
        for (int i = 0; i < args.n_input_files; i++) {
            const char* path = args.input_files[i];
            if (is_directory(path)) {
                effective_vcs = vcs_detect(path);
                if (effective_vcs != VCS_NONE) break;
            }
        }
    }

    int error_count = 0;

    if (effective_vcs == VCS_GIT || effective_vcs == VCS_SVN) {
        for (int i = 0; i < args.n_input_files; i++) {
            const char* path = args.input_files[i];
            if (is_directory(path)) {
                int n_vcs_files = 0;
                char** vcs_files = NULL;
                if (effective_vcs == VCS_GIT)
                    vcs_files = vcs_get_files_git(path, &n_vcs_files);
                else if (effective_vcs == VCS_SVN)
                    vcs_files = vcs_get_files_svn(path, &n_vcs_files);
                if (!vcs_files || n_vcs_files == 0) {
                    fprintf(stderr, "Error: Cannot get %s files from '%s'\n",
                            effective_vcs == VCS_GIT ? "git" : "svn", path);
                    error_count++;
                    continue;
                }
                for (int j = 0; j < n_vcs_files; j++) {
                    if (filelist.count >= filelist.capacity) {
                        int nc = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                        char** np = realloc(filelist.paths, nc * sizeof(char*));
                        if (!np) {
                            vcs_free_files(vcs_files, n_vcs_files);
                            filelist_free(&filelist);
                            cli_free(&args);
                            return 1;
                        }
                        filelist.paths = np;
                        filelist.capacity = nc;
                    }
                    char full[2048];
                    snprintf(full, sizeof(full), "%s/%s", path, vcs_files[j]);
                    filelist.paths[filelist.count] = strdup(full);
                    if (!filelist.paths[filelist.count]) {
                        vcs_free_files(vcs_files, n_vcs_files);
                        filelist_free(&filelist);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.count++;
                }
                vcs_free_files(vcs_files, n_vcs_files);
            } else {
                if (filelist.count >= filelist.capacity) {
                    int nc = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                    char** np = realloc(filelist.paths, nc * sizeof(char*));
                    if (!np) {
                        filelist_free(&filelist);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.paths = np;
                    filelist.capacity = nc;
                }
                filelist.paths[filelist.count] = strdup(path);
                if (!filelist.paths[filelist.count]) {
                    filelist_free(&filelist);
                    cli_free(&args);
                    return 1;
                }
                filelist.count++;
            }
        }
    } else {
        for (int i = 0; i < args.n_input_files; i++) {
            const char* path = args.input_files[i];
            if (is_directory(path)) {
                if (filelist_scan(path, &config, &filelist) != 0) {
                    fprintf(stderr, "Error: Cannot scan directory '%s'\n", path);
                    error_count++;
                }
            } else if (is_regular_file(path)) {
                // Check if file is an archive
                if (archive_is_archive(path)) {
                    // Extract archive to temp directory
                    char* extract_dir = archive_extract(path, &temp_mgr);
                    if (extract_dir) {
                        // Scan extracted directory
                        if (filelist_scan(extract_dir, &config, &filelist) != 0) {
                            fprintf(stderr, "Warning: Cannot scan extracted archive '%s'\n",
                                    extract_dir);
                        }
                        // extract_dir is managed by temp_mgr, will be cleaned up on exit
                    } else {
                        fprintf(stderr, "Warning: Cannot extract archive '%s'\n", path);
                    }
                } else {
                    // Regular file - add to list
                    if (filelist.count >= filelist.capacity) {
                        int nc = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                        char** np = realloc(filelist.paths, nc * sizeof(char*));
                        if (!np) {
                            filelist_free(&filelist);
                            temp_manager_destroy(&temp_mgr);
                            cli_free(&args);
                            return 1;
                        }
                        filelist.paths = np;
                        filelist.capacity = nc;
                    }
                    filelist.paths[filelist.count] = strdup(path);
                    if (!filelist.paths[filelist.count]) {
                        filelist_free(&filelist);
                        temp_manager_destroy(&temp_mgr);
                        cli_free(&args);
                        return 1;
                    }
                    filelist.count++;
                }
            } else {
                fprintf(stderr, "Error: Path '%s' is not a valid file or directory\n", path);
                error_count++;
            }
        }
    }

skip_normal_scan:;  // Empty statement before declaration (C11 compatibility)

    FileStats* files = malloc(filelist.count * sizeof(FileStats));
    if (!files) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        filelist_free(&filelist);
        temp_manager_destroy(&temp_mgr);
        cli_free(&args);
        return 1;
    }

    // Initialize all entries
    for (int i = 0; i < filelist.count; i++) {
        files[i].filepath = filelist.paths[i];
        files[i].lang = NULL;
        files[i].counts.blank = 0;
        files[i].counts.comment = 0;
        files[i].counts.code = 0;
        files[i].counts.total = 0;
        files[i].ignore_reason = NULL;
    }

    // Pass 1: Uniqueness check via MD5 (unless --skip-uniqueness)
    if (!args.skip_uniqueness && filelist.count > 0) {
        UniqueTable unique_tbl;
        int table_cap = filelist.count < UNIQUE_TABLE_MAX ? filelist.count : UNIQUE_TABLE_MAX;
        if (unique_table_init(&unique_tbl, table_cap) == 0) {
            for (int i = 0; i < filelist.count; i++) {
                uint8_t md5[16];
                if (compute_file_md5(files[i].filepath, md5) == 0) {
                    if (unique_table_insert(&unique_tbl, md5) != 0) {
                        files[i].ignore_reason = "duplicate";
                    }
                }
            }
            unique_table_free(&unique_tbl);
        }
    }

    // Pass 2: Filter by language/extension, mark ignored files
    // Parse --force-lang option if present
    const Language* force_lang_all = NULL;
    char* force_lang_ext = NULL;
    const Language* force_lang_for_ext = NULL;

    if (args.force_lang != NULL) {
        // Check if format is LANG or LANG,EXT
        char* comma = strchr(args.force_lang, ',');
        if (comma) {
            // LANG,EXT format
            char* lang_name = strndup(args.force_lang, comma - args.force_lang);
            force_lang_for_ext = get_language_by_name(lang_name);
            free(lang_name);
            force_lang_ext = strdup(comma + 1);
        } else {
            // LANG only - apply to all files
            force_lang_all = get_language_by_name(args.force_lang);
        }
    }

    for (int i = 0; i < filelist.count; i++) {
        if (files[i].ignore_reason != NULL) continue;

        const char* filepath = files[i].filepath;
        const Language* lang = NULL;

        // Apply --force-lang override
        if (force_lang_all != NULL) {
            lang = force_lang_all;
        } else if (force_lang_ext != NULL && force_lang_for_ext != NULL) {
            const char* ext = get_extension(filepath);
            if (ext && strcasecmp(ext, force_lang_ext) == 0) {
                lang = force_lang_for_ext;
            } else {
                lang = detect_language(filepath);
            }
        } else {
            lang = detect_language(filepath);
        }

        // Apply --lang-no-ext for files without extension
        if (lang == NULL && args.lang_no_ext) {
            const char* ext = get_extension(filepath);
            if (ext == NULL) {
                lang = get_language_by_name(args.lang_no_ext);
            }
        }

        // Apply --script-lang for shebang language override
        if (lang == NULL && args.n_script_lang > 0) {
            // Try shebang detection with custom mappings
            FILE* fp = fopen(filepath, "r");
            if (fp) {
                char line[256];
                if (fgets(line, sizeof(line), fp)) {
                    fclose(fp);
                    if (line[0] == '#' && line[1] == '!') {
                        // Extract interpreter name
                        char* interp = line + 2;
                        while (*interp && isspace((unsigned char)*interp)) interp++;
                        char* interp_end = interp;
                        while (*interp_end && !isspace((unsigned char)*interp_end)) interp_end++;
                        *interp_end = '\0';
                        // Get just the name without path
                        const char* interp_name = strrchr(interp, '/');
                        if (interp_name)
                            interp_name++;
                        else
                            interp_name = interp;
                        // Match against --script-lang mappings
                        for (int j = 0; j < args.n_script_lang; j++) {
                            if (strcmp(interp_name, args.script_names[j]) == 0 ||
                                strstr(interp_name, args.script_names[j]) == interp_name) {
                                lang = get_language_by_name(args.script_langs[j]);
                                break;
                            }
                        }
                    }
                } else {
                    fclose(fp);
                }
            }
        }

        if (args.n_include_langs > 0) {
            if (lang == NULL ||
                !is_in_string_array(lang->name, args.include_langs, args.n_include_langs)) {
                files[i].ignore_reason = "include-lang filter";
                continue;
            }
        }
        if (lang != NULL && args.n_exclude_langs > 0) {
            if (is_in_string_array(lang->name, args.exclude_langs, args.n_exclude_langs)) {
                files[i].ignore_reason = "exclude-lang filter";
                continue;
            }
        }
        if (args.n_include_exts > 0) {
            const char* ext = get_extension(filepath);
            if (!extension_matches(ext, args.include_exts, args.n_include_exts)) {
                files[i].ignore_reason = "include-ext filter";
                continue;
            }
        }
        if (args.n_exclude_exts > 0) {
            const char* ext = get_extension(filepath);
            if (extension_matches(ext, args.exclude_exts, args.n_exclude_exts)) {
                files[i].ignore_reason = "exclude-ext filter";
                continue;
            }
        }
        files[i].lang = lang;
        if (lang == NULL && !args.quiet)
            fprintf(stderr, "Warning: Unrecognized language for file '%s'\n", filepath);
    }

    // Write unique file list if requested
    if (args.unique_file) {
        FILE* fp = fopen(args.unique_file, "w");
        if (fp) {
            for (int i = 0; i < filelist.count; i++)
                if (files[i].ignore_reason == NULL) fprintf(fp, "%s\n", files[i].filepath);
            fclose(fp);
        }
    }

    // Write ignored file list if requested
    if (args.ignored_file) {
        FILE* fp = fopen(args.ignored_file, "w");
        if (fp) {
            for (int i = 0; i < filelist.count; i++)
                if (files[i].ignore_reason != NULL)
                    fprintf(fp, "%s\t%s\n", files[i].filepath, files[i].ignore_reason);
            fclose(fp);
        }
    }

    // Write found file list if requested (--found)
    if (args.found_file) {
        FILE* fp = fopen(args.found_file, "w");
        if (fp) {
            for (int i = 0; i < filelist.count; i++) fprintf(fp, "%s\n", files[i].filepath);
            fclose(fp);
        }
    }

    // Write counted file list if requested (--counted)
    if (args.counted_file) {
        FILE* fp = fopen(args.counted_file, "w");
        if (fp) {
            for (int i = 0; i < filelist.count; i++)
                if (files[i].lang != NULL) fprintf(fp, "%s\n", files[i].filepath);
            fclose(fp);
        }
    }

    // Write categorized file list if requested (--categorized)
    if (args.categorized_file) {
        FILE* fp = fopen(args.categorized_file, "w");
        if (fp) {
            fprintf(fp, "filepath\tsize\tlanguage\tcategory\n");
            for (int i = 0; i < filelist.count; i++) {
                long size = get_file_size(files[i].filepath);
                const char* lang_name = files[i].lang ? files[i].lang->name : "(unknown)";
                const char* category = files[i].ignore_reason ? files[i].ignore_reason : "counted";
                fprintf(fp, "%s\t%ld\t%s\t%s\n", files[i].filepath, size, lang_name, category);
            }
            fclose(fp);
        }
    }

    // Apply --include-content/--exclude-content filtering
    if (args.include_content || args.exclude_content) {
        regex_t include_regex, exclude_regex;
        int has_include = 0, has_exclude = 0;

        if (args.include_content) {
            if (regcomp(&include_regex, args.include_content, REG_EXTENDED | REG_NOSUB) == 0) {
                has_include = 1;
            }
        }
        if (args.exclude_content) {
            if (regcomp(&exclude_regex, args.exclude_content, REG_EXTENDED | REG_NOSUB) == 0) {
                has_exclude = 1;
            }
        }

        for (int i = 0; i < filelist.count; i++) {
            if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;

            // Read file content to check against regex
            FILE* fp = fopen(files[i].filepath, "r");
            if (!fp) continue;

            int matched_include = 0, matched_exclude = 0;
            char line[4096];
            while (fgets(line, sizeof(line), fp) != NULL) {
                if (has_include && regexec(&include_regex, line, 0, NULL, 0) == 0) {
                    matched_include = 1;
                }
                if (has_exclude && regexec(&exclude_regex, line, 0, NULL, 0) == 0) {
                    matched_exclude = 1;
                }
                if (matched_include && matched_exclude) break;  // Found both, no need to continue
            }
            fclose(fp);

            // Apply filters
            if (has_include && !matched_include) {
                files[i].ignore_reason = "include-content filter";
                files[i].lang = NULL;
            }
            if (has_exclude && matched_exclude) {
                files[i].ignore_reason = "exclude-content filter";
                files[i].lang = NULL;
            }
        }

        if (has_include) regfree(&include_regex);
        if (has_exclude) regfree(&exclude_regex);
    }

    // Pass 3: Count lines with language awareness
    // Apply --timeout filtering based on file size estimate
    if (args.timeout_sec > 0) {
        // Estimate processing time based on file size (rough: 1MB ~ 1s for typical code)
        for (int i = 0; i < filelist.count; i++) {
            if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;
            long size = get_file_size(files[i].filepath);
            if (size > args.timeout_sec * 1024 * 1024) {  // size > timeout * 1MB
                files[i].ignore_reason = "timeout estimate";
                files[i].lang = NULL;
            }
        }
    }

    // Build list of files to count (those with identified languages)
    int n_count_files = 0;
    for (int i = 0; i < filelist.count; i++) {
        if (files[i].lang != NULL) n_count_files++;
    }

    if (n_count_files > 0) {
        // Determine number of workers
        int n_workers = args.processes;
        if (n_workers == 0) {
            // Auto: use CPU count
            ParallelConfig auto_config;
            parallel_default_config(&auto_config);
            n_workers = auto_config.n_workers;
        }

        // Use parallel processing for large file counts with multiple workers
        if (n_workers > 1 && n_count_files >= 50) {
            if (args.use_threads) {
                // Thread mode - build input array with pre-detected languages
                ThreadInputFile* thread_inputs = malloc(n_count_files * sizeof(ThreadInputFile));
                if (!thread_inputs) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    free(files);
                    filelist_free(&filelist);
                    filelist_free_exclude_patterns(&config);
                    temp_manager_destroy(&temp_mgr);
                    cli_free(&args);
                    return 1;
                }

                int idx = 0;
                for (int i = 0; i < filelist.count; i++) {
                    if (files[i].lang != NULL) {
                        thread_inputs[idx].filepath = files[i].filepath;
                        thread_inputs[idx].lang = files[i].lang;
                        idx++;
                    }
                }

                ThreadConfig thread_config;
                thread_default_config(&thread_config);
                thread_config.n_threads = n_workers;

                FileStats* thread_results = malloc(n_count_files * sizeof(FileStats));
                if (!thread_results) {
                    free(thread_inputs);
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    free(files);
                    filelist_free(&filelist);
                    filelist_free_exclude_patterns(&config);
                    temp_manager_destroy(&temp_mgr);
                    cli_free(&args);
                    return 1;
                }

                int n_thread_results = n_count_files;
                int thread_count = threaded_count_files(
                    thread_inputs, n_count_files, &thread_config, args.skip_leading_exts,
                    args.n_skip_leading_exts, args.skip_leading, thread_results, &n_thread_results);

                // Copy results back to files array
                for (int i = 0; i < n_thread_results; i++) {
                    for (int j = 0; j < filelist.count; j++) {
                        if (strcmp(files[j].filepath, thread_results[i].filepath) == 0) {
                            files[j].counts = thread_results[i].counts;
                            break;
                        }
                    }
                }

                free(thread_results);
                free(thread_inputs);

                if (thread_count < 0) {
                    fprintf(stderr,
                            "Warning: Thread processing failed, some files may not be counted\n");
                }
            } else {
                // Process mode (fork + pipes) - build array of file paths
                const char** count_paths = malloc(n_count_files * sizeof(const char*));
                if (!count_paths) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    free(files);
                    filelist_free(&filelist);
                    filelist_free_exclude_patterns(&config);
                    temp_manager_destroy(&temp_mgr);
                    cli_free(&args);
                    return 1;
                }

                int idx = 0;
                for (int i = 0; i < filelist.count; i++) {
                    if (files[i].lang != NULL) {
                        count_paths[idx++] = files[i].filepath;
                    }
                }

                ParallelConfig par_config;
                par_config.n_workers = n_workers;
                par_config.chunk_size = 100;
                par_config.timeout_sec = 300;

                ParallelResult* par_results = malloc(n_count_files * sizeof(ParallelResult));
                if (!par_results) {
                    free(count_paths);
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    free(files);
                    filelist_free(&filelist);
                    filelist_free_exclude_patterns(&config);
                    temp_manager_destroy(&temp_mgr);
                    cli_free(&args);
                    return 1;
                }

                int n_par_results = n_count_files;
                int par_count = parallel_count_files(
                    count_paths, n_count_files, &par_config, args.skip_leading_exts,
                    args.n_skip_leading_exts, args.skip_leading, par_results, &n_par_results);

                // Copy results back to files array
                for (int i = 0; i < n_par_results; i++) {
                    for (int j = 0; j < filelist.count; j++) {
                        if (strcmp(files[j].filepath, par_results[i].filepath) == 0) {
                            files[j].counts = par_results[i].counts;
                            break;
                        }
                    }
                }

                free(par_results);
                free(count_paths);

                if (par_count < 0) {
                    fprintf(stderr,
                            "Warning: Parallel processing failed, some files may not be counted\n");
                }
            }
        } else {
            // Sequential counting (fallback for small file counts or single worker)
            for (int i = 0; i < filelist.count; i++) {
                if (files[i].lang == NULL) continue;

                // Determine skip_lines for this file based on extension
                int skip_lines = 0;
                if (args.skip_leading > 0) {
                    if (args.n_skip_leading_exts > 0) {
                        const char* ext = get_extension(files[i].filepath);
                        if (extension_matches(ext, args.skip_leading_exts,
                                              args.n_skip_leading_exts)) {
                            skip_lines = args.skip_leading;
                        }
                    } else {
                        skip_lines = args.skip_leading;
                    }
                }

                if (count_file_with_lang(files[i].filepath, files[i].lang, skip_lines,
                                         &files[i].counts) != 0) {
                    fprintf(stderr, "Error: Cannot read file '%s'\n", files[i].filepath);
                    files[i].counts.blank = 0;
                    files[i].counts.comment = 0;
                    files[i].counts.code = 0;
                    files[i].counts.total = 0;
                    error_count++;
                }
            }
        }
    }

    clock_t end_time = clock();
    double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Generate stripped files if requested (--strip-comments or --strip-code)
    if (args.strip_comments || args.strip_code) {
        for (int i = 0; i < filelist.count; i++) {
            if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;

            // Read source file
            FILE* src_fp = fopen(files[i].filepath, "r");
            if (!src_fp) continue;

            // Build output filename
            char out_path[2048];
            if (args.original_dir) {
                snprintf(out_path, sizeof(out_path), "%s.%s", files[i].filepath,
                         args.strip_comments ? args.strip_comments : args.strip_code);
            } else {
                const char* basename = strrchr(files[i].filepath, '/');
                basename = basename ? basename + 1 : files[i].filepath;
                snprintf(out_path, sizeof(out_path), "%s.%s", basename,
                         args.strip_comments ? args.strip_comments : args.strip_code);
            }

            FILE* out_fp = fopen(out_path, "w");
            if (!out_fp) {
                fclose(src_fp);
                continue;
            }

            // Process each line
            char line[4096];
            while (fgets(line, sizeof(line), src_fp) != NULL) {
                size_t len = strlen(line);
                // Remove trailing newline for analysis
                while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
                    line[len - 1] = '\0';
                    len--;
                }

                // Determine line type
                int is_blank = 1;
                for (size_t j = 0; j < len; j++) {
                    if (!isspace((unsigned char)line[j])) {
                        is_blank = 0;
                        break;
                    }
                }

                // Simple comment detection based on language
                int is_comment = 0;
                if (files[i].lang && files[i].lang->generic_filters) {
                    for (size_t f = 0; f < files[i].lang->generic_filter_count; f++) {
                        const GenericFilter* gf = &files[i].lang->generic_filters[f];
                        if (gf->type == FILTER_REMOVE_INLINE) {
                            size_t plen = strlen(gf->pattern_open);
                            const char* trimmed = line;
                            while (*trimmed && isspace((unsigned char)*trimmed)) trimmed++;
                            if (strncmp(trimmed, gf->pattern_open, plen) == 0) {
                                is_comment = 1;
                                break;
                            }
                        }
                    }
                }

                // Write line based on mode
                if (args.strip_comments) {
                    // Strip blank and comment lines - keep only code
                    if (!is_blank && !is_comment) {
                        fprintf(out_fp, "%s\n", line);
                    }
                } else if (args.strip_code) {
                    // Strip code lines - keep only blank and comments
                    if (is_blank || is_comment) {
                        fprintf(out_fp, "%s\n", line);
                    }
                }
            }

            fclose(src_fp);
            fclose(out_fp);
        }
    }

    FILE* report_fp = NULL;
    if (args.report_file) {
        report_fp = fopen(args.report_file, "w");
        if (!report_fp) {
            fprintf(stderr, "Error: Cannot open report file '%s'\n", args.report_file);
            free(files);
            filelist_free(&filelist);
            filelist_free_exclude_patterns(&config);
            cli_free(&args);
            return 1;
        }
        freopen(args.report_file, "w", stdout);
    }

    switch (args.output_format) {
        case FORMAT_JSON:
            if (args.by_file)
                output_json_by_file(files, filelist.count, elapsed_sec);
            else
                output_json(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_CSV:
            if (args.by_file)
                output_csv_by_file(files, filelist.count, elapsed_sec);
            else
                output_csv(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_MD:
            if (args.by_file)
                output_md_by_file(files, filelist.count, elapsed_sec);
            else
                output_md(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_YAML:
            if (args.by_file)
                output_yaml_by_file(files, filelist.count, elapsed_sec);
            else
                output_yaml(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_XML:
            if (args.by_file)
                output_xml_by_file(files, filelist.count, elapsed_sec);
            else
                output_xml(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_HTML:
            if (args.by_file)
                output_html_by_file(files, filelist.count, elapsed_sec);
            else
                output_html(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_SQL:
            if (args.by_file)
                output_sql_by_file(files, filelist.count, elapsed_sec, NULL);
            else
                output_sql(files, filelist.count, elapsed_sec, NULL);
            break;
        case FORMAT_TEXT:
        default:
            if (args.by_file_by_lang)
                output_by_file_by_lang(files, filelist.count, elapsed_sec);
            else if (args.by_file)
                output_text_by_file(files, filelist.count, elapsed_sec);
            else
                output_text(files, filelist.count, elapsed_sec);
            break;
    }

    if (report_fp) fclose(report_fp);
    if (force_lang_ext) free(force_lang_ext);
    free(files);
    filelist_free(&filelist);
    filelist_free_exclude_patterns(&config);
    temp_manager_destroy(&temp_mgr);
    cli_free(&args);
    return (error_count > 0) ? 1 : 0;
}
