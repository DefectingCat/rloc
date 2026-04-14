#ifndef VCS_H
#define VCS_H

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

#endif