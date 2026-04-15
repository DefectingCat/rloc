#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "cli.h"
#include "config.h"
#include "counter.h"
#include "diff.h"
#include "filelist.h"
#include "language.h"
#include "output.h"
#include "unique.h"
#include "util.h"
#include "config.h"
#include "lang_defs.h"
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

    if (args.show_help) { cli_print_help(argv[0]); cli_free(&args); return 0; }
    if (args.show_version) { cli_print_version(); cli_free(&args); return 0; }
    if (args.show_lang) { lang_show(args.show_lang_arg); cli_free(&args); return 0; }
    if (args.show_ext) { ext_show(args.show_ext_arg); cli_free(&args); return 0; }

    if (args.diff_commit1 != NULL && args.diff_commit2 != NULL) {
        const char* repo_path = (args.n_input_files > 0) ? args.input_files[0] : ".";
        if (!is_directory(repo_path)) repo_path = ".";
        int n_diff_files = 0;
        DiffFileStats* diff_files = diff_get_files(repo_path, args.diff_commit1, args.diff_commit2, &n_diff_files);
        if (!diff_files || n_diff_files == 0) {
            printf("No changes between %s and %s\n", args.diff_commit1, args.diff_commit2);
            cli_free(&args);
            return 0;
        }
        output_diff(diff_files, n_diff_files, args.diff_commit1, args.diff_commit2, args.by_file);
        diff_free_files(diff_files, n_diff_files);
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

    if (args.exclude_list_file) {
        if (filelist_load_exclude_patterns(args.exclude_list_file, &config) != 0) {
            fprintf(stderr, "Error: Cannot load exclude patterns from '%s'\n", args.exclude_list_file);
            cli_free(&args);
            return 1;
        }
    }

    FileList filelist;
    filelist_init(&filelist);

    clock_t start_time = clock();

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
                if (effective_vcs == VCS_GIT) vcs_files = vcs_get_files_git(path, &n_vcs_files);
                else if (effective_vcs == VCS_SVN) vcs_files = vcs_get_files_svn(path, &n_vcs_files);
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
                        if (!np) { vcs_free_files(vcs_files, n_vcs_files); filelist_free(&filelist); cli_free(&args); return 1; }
                        filelist.paths = np;
                        filelist.capacity = nc;
                    }
                    char full[2048];
                    snprintf(full, sizeof(full), "%s/%s", path, vcs_files[j]);
                    filelist.paths[filelist.count] = strdup(full);
                    if (!filelist.paths[filelist.count]) { vcs_free_files(vcs_files, n_vcs_files); filelist_free(&filelist); cli_free(&args); return 1; }
                    filelist.count++;
                }
                vcs_free_files(vcs_files, n_vcs_files);
            } else {
                if (filelist.count >= filelist.capacity) {
                    int nc = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                    char** np = realloc(filelist.paths, nc * sizeof(char*));
                    if (!np) { filelist_free(&filelist); cli_free(&args); return 1; }
                    filelist.paths = np;
                    filelist.capacity = nc;
                }
                filelist.paths[filelist.count] = strdup(path);
                if (!filelist.paths[filelist.count]) { filelist_free(&filelist); cli_free(&args); return 1; }
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
                if (filelist.count >= filelist.capacity) {
                    int nc = (filelist.capacity == 0) ? 16 : filelist.capacity * 2;
                    char** np = realloc(filelist.paths, nc * sizeof(char*));
                    if (!np) { filelist_free(&filelist); cli_free(&args); return 1; }
                    filelist.paths = np;
                    filelist.capacity = nc;
                }
                filelist.paths[filelist.count] = strdup(path);
                if (!filelist.paths[filelist.count]) { filelist_free(&filelist); cli_free(&args); return 1; }
                filelist.count++;
            } else {
                fprintf(stderr, "Error: Path '%s' is not a valid file or directory\n", path);
                error_count++;
            }
        }
    }

    FileStats* files = malloc(filelist.count * sizeof(FileStats));
    if (!files) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        filelist_free(&filelist);
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
    for (int i = 0; i < filelist.count; i++) {
        if (files[i].ignore_reason != NULL) continue;

        const char* filepath = files[i].filepath;
        const Language* lang = detect_language(filepath);

        if (args.n_include_langs > 0) {
            if (lang == NULL || !is_in_string_array(lang->name, args.include_langs, args.n_include_langs)) {
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
                if (files[i].ignore_reason == NULL)
                    fprintf(fp, "%s\n", files[i].filepath);
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

    // Pass 3: Count lines with language awareness
    for (int i = 0; i < filelist.count; i++) {
        if (files[i].lang == NULL) continue;

        // Determine skip_lines for this file based on extension
        int skip_lines = 0;
        if (args.skip_leading > 0) {
            if (args.n_skip_leading_exts > 0) {
                const char* ext = get_extension(files[i].filepath);
                if (extension_matches(ext, args.skip_leading_exts, args.n_skip_leading_exts)) {
                    skip_lines = args.skip_leading;
                }
            } else {
                skip_lines = args.skip_leading;
            }
        }

        if (count_file_with_lang(files[i].filepath, files[i].lang, skip_lines, &files[i].counts) != 0) {
            fprintf(stderr, "Error: Cannot read file '%s'\n", files[i].filepath);
            files[i].counts.blank = 0;
            files[i].counts.comment = 0;
            files[i].counts.code = 0;
            files[i].counts.total = 0;
            error_count++;
        }
    }

    clock_t end_time = clock();
    double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

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
            if (args.by_file) output_json_by_file(files, filelist.count, elapsed_sec);
            else output_json(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_CSV:
            if (args.by_file) output_csv_by_file(files, filelist.count, elapsed_sec);
            else output_csv(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_MD:
            if (args.by_file) output_md_by_file(files, filelist.count, elapsed_sec);
            else output_md(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_YAML:
            if (args.by_file) output_yaml_by_file(files, filelist.count, elapsed_sec);
            else output_yaml(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_XML:
            if (args.by_file) output_xml_by_file(files, filelist.count, elapsed_sec);
            else output_xml(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_HTML:
            if (args.by_file) output_html_by_file(files, filelist.count, elapsed_sec);
            else output_html(files, filelist.count, elapsed_sec);
            break;
        case FORMAT_SQL:
            if (args.by_file) output_sql_by_file(files, filelist.count, elapsed_sec, NULL);
            else output_sql(files, filelist.count, elapsed_sec, NULL);
            break;
        case FORMAT_TEXT:
        default:
            if (args.by_file_by_lang) output_by_file_by_lang(files, filelist.count, elapsed_sec);
            else if (args.by_file) output_text_by_file(files, filelist.count, elapsed_sec);
            else output_text(files, filelist.count, elapsed_sec);
            break;
    }

    if (report_fp) fclose(report_fp);
    free(files);
    filelist_free(&filelist);
    filelist_free_exclude_patterns(&config);
    cli_free(&args);
    return (error_count > 0) ? 1 : 0;
}
