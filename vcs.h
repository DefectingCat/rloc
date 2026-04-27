#ifndef VCS_H
#define VCS_H

#include <stddef.h>  // For size_t

// VCS type enumeration
typedef enum {
    VCS_NONE,  // No VCS, use regular file scanning
    VCS_GIT,   // Use git ls-files
    VCS_SVN,   // Use svn list -R (future)
    VCS_AUTO   // Auto-detect based on .git/.svn presence
} VcsType;

// VCS configuration
typedef struct {
    VcsType type;
    char** exclude_dirs;  // Additional directories to exclude
    int n_exclude_dirs;
} VcsConfig;

// Get file list from git repository using git ls-files
// Returns array of file paths, sets n_files count
// Caller must free returned array with vcs_free_files()
char** vcs_get_files_git(const char* repo_path, int* n_files);

// Get file list from SVN repository using svn list -R
// Returns array of file paths, sets n_files count
// Caller must free returned array with vcs_free_files()
char** vcs_get_files_svn(const char* repo_path, int* n_files);

// Free file list returned by vcs_get_files_git or vcs_get_files_svn
void vcs_free_files(char** files, int n_files);

// Check if directory is a git repository (has .git subdir)
int vcs_is_git_repo(const char* path);

// Check if directory is an SVN repository (has .svn subdir)
int vcs_is_svn_repo(const char* path);

// Detect VCS type at given path (VCS_GIT if .git exists, VCS_SVN if .svn exists, VCS_NONE
// otherwise)
VcsType vcs_detect(const char* path);

// === Git Commit Statistics API ===

// Check if git command is available
int vcs_check_git_available(void);

// Get file list at specific commit
// Returns array of file paths, sets n_files count
// Caller must free returned array with vcs_free_files()
char** vcs_get_files_at_commit(const char* repo_path, const char* commit, int* n_files);

// Get file content at specific commit
// Uses git show <commit>:<filepath> to retrieve content
// Returns allocated buffer with content, sets content_len
// Caller must free returned buffer
// Returns NULL on error
char* vcs_get_file_at_commit(const char* repo_path, const char* commit, const char* filepath,
                             size_t* content_len);

// Diff flags for vcs_get_changed_files
#define VCS_DIFF_IGNORE_WHITESPACE (0x01)
#define VCS_DIFF_INCLUDE_SUBMODULES (0x02)

// Get files changed between two references (commits, branches, etc.)
// Returns array of changed file paths, sets n_files count
// If flags includes VCS_DIFF_IGNORE_WHITESPACE, whitespace-only changes are ignored
// If flags includes VCS_DIFF_INCLUDE_SUBMODULES, submodule changes are included
// Returns NULL and sets n_files=0 on error or invalid references
// Caller must free returned array with vcs_free_files()
char** vcs_get_changed_files(const char* repo, const char* ref1, const char* ref2,
                             unsigned int flags, int* n_files);

// Get files changed between two commits
// Returns array of changed file paths, sets n_files count
// Each path is prefixed with status: 'A' (added), 'M' (modified), 'D' (deleted)
char** vcs_get_diff_files(const char* repo_path, const char* commit1, const char* commit2,
                          int* n_files);

#endif