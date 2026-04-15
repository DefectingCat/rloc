#include "vcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

// === Git Commit Statistics Implementation ===

int vcs_check_git_available(void) {
    FILE* fp = popen("git --version 2>/dev/null", "r");
    if (!fp) return 0;
    char buf[64];
    int found = (fread(buf, 1, sizeof(buf), fp) > 0);
    pclose(fp);
    return found;
}

char** vcs_get_files_at_commit(const char* repo_path, const char* commit, int* n_files) {
    *n_files = 0;
    if (!repo_path || !commit) return NULL;

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git ls-tree -r --name-only '%s' 2>/dev/null", repo_path, commit);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    char buffer[65536];
    size_t total = 0;
    size_t chunk;
    while ((chunk = fread(buffer + total, 1, sizeof(buffer) - total - 1, fp)) > 0) {
        total += chunk;
        if (total >= sizeof(buffer) - 1) break;
    }
    pclose(fp);

    if (total == 0) return NULL;

    // Count lines
    int count = 0;
    for (size_t i = 0; i < total; i++) {
        if (buffer[i] == '\n') count++;
    }
    if (buffer[total - 1] != '\n') count++;

    if (count == 0) return NULL;

    char** files = malloc(count * sizeof(char*));
    if (!files) return NULL;

    int idx = 0;
    char* start = buffer;
    for (size_t i = 0; i < total && idx < count; i++) {
        if (buffer[i] == '\n') {
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

    *n_files = idx;
    return files;
}

char* vcs_get_file_at_commit(const char* repo_path, const char* commit,
                              const char* filepath, size_t* content_len) {
    *content_len = 0;
    if (!repo_path || !commit || !filepath) return NULL;

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git show '%s:%s' 2>/dev/null", repo_path, commit, filepath);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    // Read all content
    size_t capacity = 4096;
    char* content = malloc(capacity);
    if (!content) {
        pclose(fp);
        return NULL;
    }

    size_t total = 0;
    size_t chunk;
    while ((chunk = fread(content + total, 1, capacity - total, fp)) > 0) {
        total += chunk;
        if (total >= capacity - 1) {
            capacity *= 2;
            char* new_content = realloc(content, capacity);
            if (!new_content) {
                free(content);
                pclose(fp);
                return NULL;
            }
            content = new_content;
        }
    }
    pclose(fp);

    *content_len = total;
    return content;
}

char** vcs_get_diff_files(const char* repo_path, const char* commit1,
                          const char* commit2, int* n_files) {
    *n_files = 0;
    if (!repo_path || !commit1 || !commit2) return NULL;

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git diff --name-status '%s' '%s' 2>/dev/null",
             repo_path, commit1, commit2);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    char buffer[65536];
    size_t total = 0;
    while (fread(buffer + total, 1, sizeof(buffer) - total - 1, fp) > 0) {
        total += fread(buffer + total, 1, sizeof(buffer) - total - 1, fp);
        if (total >= sizeof(buffer) - 1) break;
    }
    pclose(fp);

    if (total == 0) return NULL;

    int count = 0;
    for (size_t i = 0; i < total; i++) {
        if (buffer[i] == '\n') count++;
    }

    if (count == 0) return NULL;

    char** files = malloc(count * sizeof(char*));
    if (!files) return NULL;

    int idx = 0;
    char* line_start = buffer;
    for (size_t i = 0; i < total && idx < count; i++) {
        if (buffer[i] == '\n') {
            size_t line_len = i - (line_start - buffer);
            if (line_len > 2) {
                // Format: STATUS\tpath or STATUS path
                // Create new string with status prefix
                files[idx] = malloc(line_len + 1);
                if (files[idx]) {
                    memcpy(files[idx], line_start, line_len);
                    files[idx][line_len] = '\0';
                    idx++;
                }
            }
            line_start = buffer + i + 1;
        }
    }

    *n_files = idx;
    return files;
}