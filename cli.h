#ifndef CLI_H
#define CLI_H

#include "vcs.h"  // For VcsType enum

// Output format enumeration
typedef enum {
    FORMAT_TEXT,   // Default text table
    FORMAT_JSON,   // JSON output
    FORMAT_CSV,    // CSV output
    FORMAT_MD      // Markdown output
} OutputFormat;

// VCS type is defined in vcs.h
// VcsType: VCS_NONE, VCS_GIT, VCS_SVN, VCS_AUTO

typedef struct {
    char** input_files;     // Array of file paths (dynamically allocated)
    int n_input_files;      // Number of input files
    int show_help;          // --help flag
    int show_version;       // --version flag
    int no_recurse;         // --no-recurse flag
    long max_file_size_mb;  // --max-file-size=N (0 = use default)
    char** exclude_dirs;    // --exclude-dir=DIR1,DIR2
    int n_exclude_dirs;     // Number of exclude directories
    char** include_langs;   // --include-lang=Lang1,Lang2
    int n_include_langs;    // Number of include languages
    char** exclude_langs;   // --exclude-lang=Lang1,Lang2
    int n_exclude_langs;    // Number of exclude languages
    char** include_exts;    // --include-ext=ext1,ext2
    int n_include_exts;     // Number of include extensions
    char** exclude_exts;    // --exclude-ext=ext1,ext2
    int n_exclude_exts;     // Number of exclude extensions
    OutputFormat output_format;  // --json, --csv, --md
    int by_file;            // --by-file flag
    VcsType vcs;            // --vcs=git/svn/auto
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
