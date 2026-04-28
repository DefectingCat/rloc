#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "archive.h"
#include "cli.h"
#include "config.h"
#include "counter.h"
#include "diff.h"
#include "file_processor.h"
#include "filelist.h"
#include "lang_defs.h"
#include "language.h"
#include "output.h"
#include "parallel.h"
#include "report.h"
#include "temp_manager.h"
#include "threaded_counter.h"
#include "util.h"
#include "vcs.h"
#include "vcs_ops.h"
#include "coro_scanner.h"

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

/* Handle --list-file: read input paths from file or STDIN */
static int handle_list_file(CliArgs* args) {
    if (args->list_file == NULL) return 0;

    FILE* list_fp = NULL;
    if (strcmp(args->list_file, "-") == 0) {
        list_fp = stdin;
    } else {
        list_fp = fopen(args->list_file, "r");
        if (!list_fp) {
            fprintf(stderr, "Error: Cannot open list file '%s'\n", args->list_file);
            return -1;
        }
    }

    char line[2048];
    while (fgets(line, sizeof(line), list_fp) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }
        if (len == 0) continue;

        int capacity = args->n_input_files > 0 ? args->n_input_files * 2 : 16;
        char** new_files = realloc(args->input_files, capacity * sizeof(char*));
        if (!new_files) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            if (list_fp != stdin) fclose(list_fp);
            return -1;
        }
        args->input_files = new_files;
        args->input_files[args->n_input_files] = strdup(line);
        if (!args->input_files[args->n_input_files]) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            if (list_fp != stdin) fclose(list_fp);
            return -1;
        }
        args->n_input_files++;
    }

    if (list_fp != stdin) fclose(list_fp);

    if (args->n_input_files == 0) {
        fprintf(stderr, "Error: No input files found in list file '%s'\n", args->list_file);
        return -1;
    }
    return 0;
}

/* Scan input paths and populate filelist */
static int scan_input_paths(const CliArgs* args, FilelistConfig* config, FileList* filelist,
                            TempManager* temp_mgr, int* error_count) {
    VcsType effective_vcs = vcs_ops_detect_effective(args);

    if (effective_vcs == VCS_GIT || effective_vcs == VCS_SVN) {
        VcsOpsContext vcs_ctx;
        vcs_ops_init(&vcs_ctx, args, temp_mgr, filelist);
        return vcs_ops_handle_vcs(&vcs_ctx);
    }

    for (int i = 0; i < args->n_input_files; i++) {
        const char* path = args->input_files[i];
        if (is_directory(path)) {
            if (args->use_coro) {
                if (coro_scan_directory(path, config, filelist) != 0) {
                    fprintf(stderr, "Error: Cannot scan directory '%s' (coro mode)\n", path);
                    (*error_count)++;
                }
            } else if (filelist_scan(path, config, filelist) != 0) {
                fprintf(stderr, "Error: Cannot scan directory '%s'\n", path);
                (*error_count)++;
            }
        } else if (is_regular_file(path)) {
            if (archive_is_archive(path)) {
                char* extract_dir = archive_extract(path, temp_mgr);
                if (extract_dir) {
                    if (filelist_scan(extract_dir, config, filelist) != 0) {
                        fprintf(stderr, "Warning: Cannot scan extracted archive '%s'\n", extract_dir);
                    }
                } else {
                    fprintf(stderr, "Warning: Cannot extract archive '%s'\n", path);
                }
            } else {
                filelist_append(filelist, path);
            }
        } else {
            fprintf(stderr, "Error: Path '%s' is not a valid file or directory\n", path);
            (*error_count)++;
        }
    }
    return 0;
}

/* Count files using parallel or sequential processing */
static int count_files(FileStats* files, int n_files, const CliArgs* args, int* error_count) {
    int n_count_files = 0;
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang != NULL) n_count_files++;
    }
    if (n_count_files == 0) return 0;

    int n_workers = args->processes;
    if (n_workers == 0) {
        ParallelConfig auto_config;
        parallel_default_config(&auto_config);
        n_workers = auto_config.n_workers;
    }

    if (n_workers > 1 && n_count_files >= PARALLEL_MIN_FILES) {
        /* Parallel processing */
        if (args->use_threads) {
            ThreadInputFile* thread_inputs = malloc(n_count_files * sizeof(ThreadInputFile));
            if (!thread_inputs) return -1;

            int idx = 0;
            for (int i = 0; i < n_files; i++) {
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
                return -1;
            }

            int n_thread_results = n_count_files;
            threaded_count_files(thread_inputs, n_count_files, &thread_config,
                                 args->skip_leading_exts, args->n_skip_leading_exts,
                                 args->skip_leading, thread_results, &n_thread_results);

            for (int i = 0; i < n_thread_results; i++) {
                for (int j = 0; j < n_files; j++) {
                    if (strcmp(files[j].filepath, thread_results[i].filepath) == 0) {
                        files[j].counts = thread_results[i].counts;
                        break;
                    }
                }
            }
            free(thread_results);
            free(thread_inputs);
        } else {
            ParallelInputFile* count_paths = malloc(n_count_files * sizeof(ParallelInputFile));
            if (!count_paths) return -1;

            int idx = 0;
            for (int i = 0; i < n_files; i++) {
                if (files[i].lang != NULL) {
                    count_paths[idx].filepath = files[i].filepath;
                    count_paths[idx].lang = files[i].lang;
                    idx++;
                }
            }

            ParallelConfig par_config;
            par_config.n_workers = n_workers;
            par_config.chunk_size = PARALLEL_CHUNK_SIZE;
            par_config.timeout_sec = PARALLEL_TIMEOUT_SEC;

            ParallelResult* par_results = malloc(n_count_files * sizeof(ParallelResult));
            if (!par_results) {
                free(count_paths);
                return -1;
            }

            int n_par_results = n_count_files;
            parallel_count_files(count_paths, n_count_files, &par_config,
                                 args->skip_leading_exts, args->n_skip_leading_exts,
                                 args->skip_leading, par_results, &n_par_results);

            for (int i = 0; i < n_par_results; i++) {
                for (int j = 0; j < n_files; j++) {
                    if (strcmp(files[j].filepath, par_results[i].filepath) == 0) {
                        files[j].counts = par_results[i].counts;
                        break;
                    }
                }
            }
            free(par_results);
            free(count_paths);
        }
    } else {
        /* Sequential counting */
        for (int i = 0; i < n_files; i++) {
            if (files[i].lang == NULL) continue;

            int skip_lines = 0;
            if (args->skip_leading > 0) {
                if (args->n_skip_leading_exts > 0) {
                    const char* ext = fp_get_extension(files[i].filepath);
                    if (fp_extension_matches(ext, args->skip_leading_exts, args->n_skip_leading_exts)) {
                        skip_lines = args->skip_leading;
                    }
                } else {
                    skip_lines = args->skip_leading;
                }
            }

            if (count_file_with_lang(files[i].filepath, files[i].lang, skip_lines,
                                     &files[i].counts) != 0) {
                fprintf(stderr, "Error: Cannot read file '%s'\n", files[i].filepath);
                files[i].counts.blank = 0;
                files[i].counts.comment = 0;
                files[i].counts.code = 0;
                files[i].counts.total = 0;
                (*error_count)++;
            }
        }
    }
    return 0;
}

/* Generate stripped files (--strip-comments or --strip-code) */
static void generate_stripped_files(FileStats* files, int n_files, const CliArgs* args) {
    if (!args->strip_comments && !args->strip_code) return;

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;

        FILE* src_fp = fopen(files[i].filepath, "r");
        if (!src_fp) continue;

        char out_path[2048];
        if (args->original_dir) {
            snprintf(out_path, sizeof(out_path), "%s.%s", files[i].filepath,
                     args->strip_comments ? args->strip_comments : args->strip_code);
        } else {
            const char* basename = strrchr(files[i].filepath, '/');
            basename = basename ? basename + 1 : files[i].filepath;
            snprintf(out_path, sizeof(out_path), "%s.%s", basename,
                     args->strip_comments ? args->strip_comments : args->strip_code);
        }

        FILE* out_fp = fopen(out_path, "w");
        if (!out_fp) {
            fclose(src_fp);
            continue;
        }

        char line[4096];
        while (fgets(line, sizeof(line), src_fp) != NULL) {
            size_t len = strlen(line);
            while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
                line[len - 1] = '\0';
                len--;
            }

            int is_blank = 1;
            for (size_t j = 0; j < len; j++) {
                if ((unsigned char)line[j] > 127 || !isspace((unsigned char)line[j])) {
                    is_blank = 0;
                    break;
                }
            }

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

            if (args->strip_comments) {
                if (!is_blank && !is_comment) fprintf(out_fp, "%s\n", line);
            } else if (args->strip_code) {
                if (is_blank || is_comment) fprintf(out_fp, "%s\n", line);
            }
        }
        fclose(src_fp);
        fclose(out_fp);
    }
}

/* Output results based on format */
static void output_results(FileStats* files, int n_files, const CliArgs* args, double elapsed_sec) {
    switch (args->output_format) {
        case FORMAT_JSON:
            if (args->by_file) output_json_by_file(files, n_files, elapsed_sec);
            else output_json(files, n_files, elapsed_sec);
            break;
        case FORMAT_CSV:
            if (args->by_file) output_csv_by_file(files, n_files, elapsed_sec);
            else output_csv(files, n_files, elapsed_sec);
            break;
        case FORMAT_MD:
            if (args->by_file) output_md_by_file(files, n_files, elapsed_sec);
            else output_md(files, n_files, elapsed_sec);
            break;
        case FORMAT_YAML:
            if (args->by_file) output_yaml_by_file(files, n_files, elapsed_sec);
            else output_yaml(files, n_files, elapsed_sec);
            break;
        case FORMAT_XML:
            if (args->by_file) output_xml_by_file(files, n_files, elapsed_sec);
            else output_xml(files, n_files, elapsed_sec);
            break;
        case FORMAT_HTML:
            if (args->by_file) output_html_by_file(files, n_files, elapsed_sec);
            else output_html(files, n_files, elapsed_sec);
            break;
        case FORMAT_SQL:
            if (args->by_file) output_sql_by_file(files, n_files, elapsed_sec, NULL);
            else output_sql(files, n_files, elapsed_sec, NULL);
            break;
        case FORMAT_TEXT:
        default:
            if (args->by_file_by_lang) output_by_file_by_lang(files, n_files, elapsed_sec);
            else if (args->by_file) output_text_by_file(files, n_files, elapsed_sec);
            else output_text(files, n_files, elapsed_sec);
            break;
    }
}

int main(int argc, char** argv) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    /* Load config before parsing CLI (unless --no-config) */
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

    /* Handle early exit options */
    if (args.show_help) { cli_print_help(argv[0]); cli_free(&args); return 0; }
    if (args.show_version) { cli_print_version(); cli_free(&args); return 0; }
    if (args.show_lang) { lang_show(args.show_lang_arg); cli_free(&args); return 0; }
    if (args.show_ext) { ext_show(args.show_ext_arg); cli_free(&args); return 0; }
    if (args.explain_lang) { explain_language(args.explain_lang); cli_free(&args); return 0; }

    /* Validate mutually exclusive options */
    if (args.strip_comments && args.strip_code) {
        fprintf(stderr, "Error: --strip-comments and --strip-code are mutually exclusive\n");
        cli_free(&args);
        return 1;
    }

    /* Handle --list-file */
    if (handle_list_file(&args) != 0) {
        cli_free(&args);
        return 1;
    }

    /* Initialize temp manager */
    TempManager temp_mgr;
    size_t max_temp = args.max_temp_size > 0 ? args.max_temp_size : (1024 * 1024 * 1024);
    if (temp_manager_create(&temp_mgr, max_temp) != 0) {
        fprintf(stderr, "Error: Failed to create temp manager\n");
        cli_free(&args);
        return 1;
    }
    temp_manager_install_handlers(&temp_mgr);

    /* Handle diff mode */
    VcsOpsContext vcs_ctx;
    vcs_ops_init(&vcs_ctx, &args, &temp_mgr, NULL);
    if (vcs_ops_is_diff_mode(&args)) {
        int ret = vcs_ops_handle_diff(&vcs_ctx);
        cli_free(&args);
        return ret;
    }

    /* Configure file scanning */
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
            fprintf(stderr, "Error: Cannot load exclude patterns from '%s'\n", args.exclude_list_file);
            cli_free(&args);
            return 1;
        }
    }

    FileList filelist;
    filelist_init(&filelist);

    clock_t start_time = clock();

    /* Handle --git mode */
    VcsOpsContext git_ctx;
    vcs_ops_init(&git_ctx, &args, &temp_mgr, &filelist);
    if (args.git_ref != NULL) {
        if (vcs_ops_handle_git_ref(&git_ctx) != 0) {
            cli_free(&args);
            return 1;
        }
        goto skip_normal_scan;
    }

    /* Scan input paths */
    int error_count = 0;
    scan_input_paths(&args, &config, &filelist, &temp_mgr, &error_count);

skip_normal_scan:;

    /* Initialize FileStats array */
    FileStats* files = malloc(filelist.count * sizeof(FileStats));
    if (!files) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        filelist_free(&filelist);
        temp_manager_destroy(&temp_mgr);
        cli_free(&args);
        return 1;
    }

    for (int i = 0; i < filelist.count; i++) {
        files[i].filepath = filelist.paths[i];
        files[i].lang = NULL;
        files[i].counts.blank = 0;
        files[i].counts.comment = 0;
        files[i].counts.code = 0;
        files[i].counts.total = 0;
        files[i].ignore_reason = NULL;
    }

    /* Pass 1: Uniqueness check */
    fp_check_uniqueness(files, filelist.count, args.skip_uniqueness);

    /* Pass 2: Language detection and filtering */
    FileProcessorContext fp_ctx;
    fp_context_init(&fp_ctx, &args, &filelist, files, filelist.count);
    fp_detect_languages_and_filter(&fp_ctx);

    /* Write auxiliary reports */
    if (args.unique_file) report_write_unique_file(files, filelist.count, args.unique_file);
    if (args.ignored_file) report_write_ignored_file(files, filelist.count, args.ignored_file);
    if (args.found_file) report_write_found_file(files, filelist.count, args.found_file);
    if (args.counted_file) report_write_counted_file(files, filelist.count, args.counted_file);
    if (args.categorized_file) report_write_categorized_file(files, filelist.count, args.categorized_file);

    /* Content filtering */
    fp_filter_by_content(files, filelist.count, args.include_content, args.exclude_content);

    /* Timeout filtering */
    fp_filter_by_timeout(files, filelist.count, args.timeout_sec);

    /* Pass 3: Count lines */
    count_files(files, filelist.count, &args, &error_count);

    clock_t end_time = clock();
    double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    /* Generate stripped files */
    generate_stripped_files(files, filelist.count, &args);

    /* Output results */
    FILE* report_fp = NULL;
    if (args.report_file) {
        report_fp = fopen(args.report_file, "w");
        if (!report_fp) {
            fprintf(stderr, "Error: Cannot open report file '%s'\n", args.report_file);
            free(files);
            filelist_free(&filelist);
            filelist_free_exclude_patterns(&config);
            temp_manager_destroy(&temp_mgr);
            cli_free(&args);
            return 1;
        }
        freopen(args.report_file, "w", stdout);
    }

    output_results(files, filelist.count, &args, elapsed_sec);

    if (report_fp) fclose(report_fp);

    /* Cleanup */
    fp_context_free(&fp_ctx);
    free(files);
    filelist_free(&filelist);
    filelist_free_exclude_patterns(&config);
    temp_manager_destroy(&temp_mgr);
    cli_free(&args);

    return (error_count > 0) ? 1 : 0;
}
