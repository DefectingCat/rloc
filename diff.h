#ifndef DIFF_H
#define DIFF_H

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

#endif