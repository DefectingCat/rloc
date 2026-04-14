#include "vcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Helper: Check if path is a directory */
static int is_dir(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

int vcs_is_git_repo(const char* path) {
    char git_path[1024];
    snprintf(git_path, sizeof(git_path), "%s/.git", path);
    return is_dir(git_path);
}

int vcs_is_svn_repo(const char* path) {
    char svn_path[1024];
    snprintf(svn_path, sizeof(svn_path), "%s/.svn", path);
    return is_dir(svn_path);
}

VcsType vcs_detect(const char* path) {
    if (vcs_is_git_repo(path)) {
        return VCS_GIT;
    }
    if (vcs_is_svn_repo(path)) {
        return VCS_SVN;
    }
    return VCS_NONE;
}

char** vcs_get_files_git(const char* repo_path, int* n_files) {
    *n_files = 0;

    // Build git ls-files command
    // Using -z for null-separated output to handle filenames with spaces
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git ls-files -z 2>/dev/null", repo_path);

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return NULL;
    }

    // Read all output into a buffer
    char buffer[65536];  // 64KB buffer
    size_t total_read = 0;
    size_t chunk;

    while ((chunk = fread(buffer + total_read, 1, sizeof(buffer) - total_read - 1, fp)) > 0) {
        total_read += chunk;
        if (total_read >= sizeof(buffer) - 1) break;
    }
    pclose(fp);

    if (total_read == 0) {
        return NULL;
    }

    // Count files (null-separated)
    int count = 0;
    for (size_t i = 0; i < total_read; i++) {
        if (buffer[i] == '\0') count++;
    }
    // Handle case where last entry is not null-terminated
    if (buffer[total_read - 1] != '\0') count++;

    if (count == 0) {
        return NULL;
    }

    // Allocate file array
    char** files = malloc(count * sizeof(char*));
    if (!files) {
        return NULL;
    }

    // Split by null characters
    int idx = 0;
    char* start = buffer;
    for (size_t i = 0; i < total_read && idx < count; i++) {
        if (buffer[i] == '\0') {
            size_t len = i - (start - buffer);
            if (len > 0) {
                files[idx] = malloc(len + 1);
                if (files[idx]) {
                    memcpy(files[idx], start, len);
                    files[idx][len] = '\0';
                    idx++;
                }
            }
            start = buffer + i + 1;
        }
    }
    // Handle last entry if not null-terminated
    if (idx < count && start < buffer + total_read) {
        size_t len = total_read - (start - buffer);
        files[idx] = malloc(len + 1);
        if (files[idx]) {
            memcpy(files[idx], start, len);
            files[idx][len] = '\0';
            idx++;
        }
    }

    *n_files = idx;
    return files;
}

char** vcs_get_files_svn(const char* repo_path, int* n_files) {
    *n_files = 0;

    // Build svn list command
    // svn list -R lists all files recursively (relative paths)
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && svn list -R 2>/dev/null", repo_path);

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return NULL;
    }

    // Read all output into a buffer
    char buffer[65536];  // 64KB buffer
    size_t total_read = 0;
    size_t chunk;

    while ((chunk = fread(buffer + total_read, 1, sizeof(buffer) - total_read - 1, fp)) > 0) {
        total_read += chunk;
        if (total_read >= sizeof(buffer) - 1) break;
    }
    pclose(fp);

    if (total_read == 0) {
        return NULL;
    }

    // Count lines (newline-separated, filter out directories ending with /)
    int count = 0;
    char* line_start = buffer;
    for (size_t i = 0; i < total_read; i++) {
        if (buffer[i] == '\n') {
            size_t len = i - (line_start - buffer);
            // Skip directories (lines ending with /) and empty lines
            if (len > 0 && buffer[i - 1] != '/') {
                count++;
            }
            line_start = buffer + i + 1;
        }
    }
    // Handle last line if not newline-terminated
    if (line_start < buffer + total_read) {
        size_t len = total_read - (line_start - buffer);
        // Skip directories and empty lines
        if (len > 0 && buffer[total_read - 1] != '/') {
            count++;
        }
    }

    if (count == 0) {
        return NULL;
    }

    // Allocate file array
    char** files = malloc(count * sizeof(char*));
    if (!files) {
        return NULL;
    }

    // Split by newlines, filtering directories
    int idx = 0;
    line_start = buffer;
    for (size_t i = 0; i < total_read && idx < count; i++) {
        if (buffer[i] == '\n') {
            size_t len = i - (line_start - buffer);
            // Skip directories (ending with /) and empty lines
            if (len > 0 && buffer[i - 1] != '/') {
                files[idx] = malloc(len + 1);
                if (files[idx]) {
                    memcpy(files[idx], line_start, len);
                    files[idx][len] = '\0';
                    idx++;
                }
            }
            line_start = buffer + i + 1;
        }
    }
    // Handle last line if not newline-terminated
    if (idx < count && line_start < buffer + total_read) {
        size_t len = total_read - (line_start - buffer);
        // Skip directories
        if (len > 0 && buffer[total_read - 1] != '/') {
            files[idx] = malloc(len + 1);
            if (files[idx]) {
                memcpy(files[idx], line_start, len);
                files[idx][len] = '\0';
                idx++;
            }
        }
    }

    *n_files = idx;
    return files;
}

void vcs_free_files(char** files, int n_files) {
    if (!files) return;
    for (int i = 0; i < n_files; i++) {
        free(files[i]);
    }
    free(files);
}