#ifndef CLI_H
#define CLI_H

#include "vcs.h"  // For VcsType enum

// Output format enumeration
typedef enum {
    FORMAT_TEXT,  // Default text table
    FORMAT_JSON,  // JSON output
    FORMAT_CSV,   // CSV output
    FORMAT_MD,    // Markdown output
    FORMAT_YAML,  // YAML output
    FORMAT_XML,   // XML output
    FORMAT_HTML,  // HTML output
    FORMAT_SQL    // SQL output
} OutputFormat;

// VCS type is defined in vcs.h
// VcsType: VCS_NONE, VCS_GIT, VCS_SVN, VCS_AUTO

typedef struct {
    char** input_files;          // Array of file paths (dynamically allocated)
    int n_input_files;           // Number of input files
    int show_help;               // --help flag
    int show_version;            // --version flag
    int no_recurse;              // --no-recurse flag
    long max_file_size_mb;       // --max-file-size=N (0 = use default)
    char** exclude_dirs;         // --exclude-dir=DIR1,DIR2
    int n_exclude_dirs;          // Number of exclude directories
    char** include_langs;        // --include-lang=Lang1,Lang2
    int n_include_langs;         // Number of include languages
    char** exclude_langs;        // --exclude-lang=Lang1,Lang2
    int n_exclude_langs;         // Number of exclude languages
    char** include_exts;         // --include-ext=ext1,ext2
    int n_include_exts;          // Number of include extensions
    char** exclude_exts;         // --exclude-ext=ext1,ext2
    int n_exclude_exts;          // Number of exclude extensions
    OutputFormat output_format;  // --json, --csv, --md, --yaml
    int by_file;                 // --by-file flag
    int by_file_by_lang;         // --by-file-by-lang flag
    VcsType vcs;                 // --vcs=git/svn/auto
    char* diff_commit1;          // --diff=<commit1>..<commit2> (first commit)
    char* diff_commit2;          // --diff=<commit1>..<commit2> (second commit, NULL if single)
    char* exclude_list_file;     // --exclude-list-file=<path> (file with exclude patterns)
    char* match_pattern;         // --match-f=<regex> (include files matching pattern)
    char* not_match_pattern;     // --not-match-f=<regex> (exclude files matching pattern)
    char* match_d_pattern;       // --match-d=<regex> (include dirs matching pattern)
    char* not_match_d_pattern;   // --not-match-d=<regex> (exclude dirs matching pattern)
    char* sql_file;              // --sql=<file> (SQL output file, "-" for stdout)
    char* report_file;           // --report-file=<path> (output to file)
    int quiet;                   // --quiet flag (suppress warnings)
} CliArgs;

// Parse command line arguments. Returns 0 on success, -1 on error.
int cli_parse(int argc, char** argv, CliArgs* args);

// Free dynamically allocated memory in CliArgs
void cli_free(CliArgs* args);

// Print usage information to stdout
void cli_print_help(const char* prog_name);

// Print version to stdout
void cli_print_version(void);

#endif
