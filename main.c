#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cli.h"
#include "config.h"
#include "counter_ops.h"
#include "file_processor.h"
#include "filelist.h"
#include "input_handler.h"
#include "lang_defs.h"
#include "language.h"
#include "output.h"
#include "report.h"
#include "scanner.h"
#include "temp_manager.h"
#include "util.h"
#include "vcs_ops.h"

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
    if (input_handler_process_list_file(&args) != 0) {
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
    scanner_scan_input_paths(&args, &config, &filelist, &temp_mgr, &error_count);

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
    counter_ops_count_files(files, filelist.count, &args, &error_count);

    clock_t end_time = clock();
    double elapsed_sec = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    /* Generate stripped files */
    counter_ops_generate_stripped(files, filelist.count, &args);

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
