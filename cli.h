#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include "vcs.h"

typedef enum {
    FORMAT_TEXT,
    FORMAT_JSON,
    FORMAT_CSV,
    FORMAT_MD,
    FORMAT_YAML,
    FORMAT_XML,
    FORMAT_HTML,
    FORMAT_SQL
} OutputFormat;

typedef struct {
    char** input_files;
    int n_input_files;
    int show_help;
    int show_version;
    int no_recurse;
    long max_file_size_mb;
    char** exclude_dirs;
    int n_exclude_dirs;
    char** include_langs;
    int n_include_langs;
    char** exclude_langs;
    int n_exclude_langs;
    char** include_exts;
    int n_include_exts;
    char** exclude_exts;
    int n_exclude_exts;
    OutputFormat output_format;
    int by_file;
    int by_file_by_lang;
    VcsType vcs;
    char* diff_commit1;
    char* diff_commit2;
    char* exclude_list_file;
    char* match_pattern;
    char* not_match_pattern;
    char* match_d_pattern;
    char* not_match_d_pattern;
    char* sql_file;
    char* report_file;
    int quiet;
    size_t max_temp_size;
    char* staging_dir;
    // Phase 1 fields
    int skip_leading;
    char** skip_leading_exts;
    int n_skip_leading_exts;
    int progress_rate;
    int no_config;
    char* config_file;
    int skip_uniqueness;
    char* unique_file;
    char* ignored_file;
    int show_lang;
    char* show_lang_arg;
    int show_ext;
    char* show_ext_arg;
} CliArgs;

int cli_parse(int argc, char** argv, CliArgs* args);
void cli_free(CliArgs* args);
void cli_print_help(const char* prog_name);
void cli_print_version(void);

#endif
/* Pre-scan argv for --config/--no-config before full parsing */
int cli_prescan_config(int argc, char** argv, CliArgs* args);

