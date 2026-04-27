#ifndef DIFF_H
#define DIFF_H

// Diff mode flags
#define DIFF_MODE_RELATIVE (0x01)
#define DIFF_MODE_ALL (0x02)
#define DIFF_SHOW_ALIGNMENT (0x04)
#define DIFF_IGNORE_WHITESPACE (0x08)
#define DIFF_INCLUDE_SUBMODULES (0x10)

// Configuration for diff operations
typedef struct {
    const char* ref1;       // First reference (commit, branch, or "HEAD")
    const char* ref2;       // Second reference (may be NULL for relative mode)
    unsigned int flags;     // Combination of DIFF_MODE_* flags
    const char* repo_path;  // Path to the git repository
} DiffConfig;

// Diff statistics for a single file
typedef struct {
    char* filepath;    // File path relative to repo root
    int added;         // Lines added
    int removed;       // Lines removed
    const char* lang;  // Language name (from language detection)
} DiffFileStats;

// Get list of changed files between two commits
// Returns array of DiffFileStats, sets n_files count
// Caller must free returned array with diff_free_files()
DiffFileStats* diff_get_files(const char* repo_path, const char* commit1, const char* commit2,
                              int* n_files);

// Free diff file stats array
void diff_free_files(DiffFileStats* files, int n_files);

// Free all fields owned by a DiffConfig (does not free the config pointer itself)
void diff_free_config(DiffConfig* config);

// Extended diff with mode flags support
// Returns array of DiffFileStats, sets n_files count
// DIFF_MODE_RELATIVE: only changed files (like git diff --numstat)
// DIFF_MODE_ALL: all files aligned with historical stats (added/removed/modified/deleted)
// Caller must free returned array with diff_free_files()
DiffFileStats* diff_get_stats_extended(const DiffConfig* config, int* n_files);

#endif