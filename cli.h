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
    // New unified diff fields (Phase 5)
    char* diff_refs;       // Raw diff ref string (e.g. "HEAD~1..HEAD")
    unsigned int diff_flags; // Combination of DIFF_MODE_* flags
    // Standalone diff options
    int diff_alignment;    // --diff-alignment: show alignment info
    int include_submodules; // --include-submodules: include submodule changes
    int ignore_whitespace; // --ignore-whitespace: ignore whitespace in diff
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
    // Phase 2 fields
    int processes;           // Number of parallel workers (0 = auto)
    char* batch_input;       // Internal: worker file list input
    int batch_output_tsv;    // Internal: worker TSV output flag
    char* extract_with;      // Custom archive extraction command
    char* skip_archive;      // Skip archives matching pattern
    int max_archive_depth;   // Maximum archive nesting depth (default 3)
    // Phase 3 fields
    char* git_ref;           // Git commit/branch/tag to count
    char* list_file;         // Read input file list from FILE
    char* force_lang;        // Force language for all files (LANG or LANG,EXT)
    // Phase 4 fields
    char* strip_comments;    // Extension for stripped comments output (--strip-comments=EXT)
    char* strip_code;        // Extension for stripped code output (--strip-code=EXT)
    int original_dir;        // Write stripped files to original directory
    int fullpath;            // Match full path instead of basename
    char* include_content;   // Include files containing regex content
    char* exclude_content;   // Exclude files containing regex content
    int timeout_sec;         // Processing timeout per file (seconds)
    int diff_timeout_sec;    // Diff processing timeout per file (seconds)
    char** ignore_regex_langs; // Languages for ignore-regex
    char** ignore_regex_patterns; // Regex patterns for ignore-regex
    int n_ignore_regex;      // Number of ignore-regex entries
    char* lang_no_ext;       // Language for files without extension
    char** script_langs;     // Script languages (--script-lang=LANG,S)
    char** script_names;     // Script names (--script-lang=LANG,S)
    int n_script_lang;       // Number of script-lang entries
    int follow_links;        // Follow symlinks to directories
    char* explain_lang;      // Language to explain filters for
    char* categorized_file;  // File to save categorized info
    char* counted_file;      // File to save counted file names
    char* found_file;        // File to save found file names
} CliArgs;

int cli_parse(int argc, char** argv, CliArgs* args);
void cli_free(CliArgs* args);
void cli_print_help(const char* prog_name);
void cli_print_version(void);
/* Pre-scan argv for --config/--no-config before full parsing */
int cli_prescan_config(int argc, char** argv, CliArgs* args);

#endif

