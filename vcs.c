#include "vcs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

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

    // Read all output into a dynamic buffer
    Buffer* buf = buffer_new(65536);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    char read_buf[8192];
    size_t chunk;
    while ((chunk = fread(read_buf, 1, sizeof(read_buf), fp)) > 0) {
        if (buffer_append(buf, read_buf, chunk) != 0) break;
    }
    pclose(fp);

    size_t total_read = buf->size;
    if (total_read == 0) {
        buffer_free(buf);
        return NULL;
    }

    // Count files (null-separated)
    int count = 0;
    for (size_t i = 0; i < total_read; i++) {
        if (buf->data[i] == '\0') count++;
    }
    // Handle case where last entry is not null-terminated
    if (total_read > 0 && buf->data[total_read - 1] != '\0') count++;

    if (count == 0) {
        buffer_free(buf);
        return NULL;
    }

    // Allocate file array
    char** files = malloc(count * sizeof(char*));
    if (!files) {
        buffer_free(buf);
        return NULL;
    }

    // Split by null characters
    int idx = 0;
    char* start = buf->data;
    for (size_t i = 0; i < total_read && idx < count; i++) {
        if (buf->data[i] == '\0') {
            size_t len = i - (start - buf->data);
            if (len > 0) {
                files[idx] = malloc(len + 1);
                if (files[idx]) {
                    memcpy(files[idx], start, len);
                    files[idx][len] = '\0';
                    idx++;
                }
            }
            start = buf->data + i + 1;
        }
    }
    // Handle last entry if not null-terminated
    if (idx < count && start < buf->data + total_read) {
        size_t len = total_read - (start - buf->data);
        files[idx] = malloc(len + 1);
        if (files[idx]) {
            memcpy(files[idx], start, len);
            files[idx][len] = '\0';
            idx++;
        }
    }

    buffer_free(buf);

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

    // Read all output into a dynamic buffer
    Buffer* buf = buffer_new(65536);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    char read_buf[8192];
    size_t chunk;
    while ((chunk = fread(read_buf, 1, sizeof(read_buf), fp)) > 0) {
        if (buffer_append(buf, read_buf, chunk) != 0) break;
    }
    pclose(fp);

    size_t total_read = buf->size;
    if (total_read == 0) {
        buffer_free(buf);
        return NULL;
    }

    // Count lines (newline-separated, filter out directories ending with /)
    int count = 0;
    char* line_start = buf->data;
    for (size_t i = 0; i < total_read; i++) {
        if (buf->data[i] == '\n') {
            size_t len = i - (line_start - buf->data);
            // Skip directories (lines ending with /) and empty lines
            if (len > 0 && buf->data[i - 1] != '/') {
                count++;
            }
            line_start = buf->data + i + 1;
        }
    }
    // Handle last line if not newline-terminated
    if (line_start < buf->data + total_read) {
        size_t len = total_read - (line_start - buf->data);
        // Skip directories and empty lines
        if (len > 0 && buf->data[total_read - 1] != '/') {
            count++;
        }
    }

    if (count == 0) {
        buffer_free(buf);
        return NULL;
    }

    // Allocate file array
    char** files = malloc(count * sizeof(char*));
    if (!files) {
        buffer_free(buf);
        return NULL;
    }

    // Split by newlines, filtering directories
    int idx = 0;
    line_start = buf->data;
    for (size_t i = 0; i < total_read && idx < count; i++) {
        if (buf->data[i] == '\n') {
            size_t len = i - (line_start - buf->data);
            // Skip directories (ending with /) and empty lines
            if (len > 0 && buf->data[i - 1] != '/') {
                files[idx] = malloc(len + 1);
                if (files[idx]) {
                    memcpy(files[idx], line_start, len);
                    files[idx][len] = '\0';
                    idx++;
                }
            }
            line_start = buf->data + i + 1;
        }
    }
    // Handle last line if not newline-terminated
    if (idx < count && line_start < buf->data + total_read) {
        size_t len = total_read - (line_start - buf->data);
        // Skip directories
        if (len > 0 && buf->data[total_read - 1] != '/') {
            files[idx] = malloc(len + 1);
            if (files[idx]) {
                memcpy(files[idx], line_start, len);
                files[idx][len] = '\0';
                idx++;
            }
        }
    }

    buffer_free(buf);

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
    snprintf(cmd, sizeof(cmd), "cd '%s' && git ls-tree -r --name-only '%s' 2>/dev/null", repo_path,
             commit);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    Buffer* buf = buffer_new(65536);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    char read_buf[8192];
    size_t chunk;
    while ((chunk = fread(read_buf, 1, sizeof(read_buf), fp)) > 0) {
        if (buffer_append(buf, read_buf, chunk) != 0) break;
    }
    pclose(fp);

    size_t total = buf->size;
    if (total == 0) {
        buffer_free(buf);
        return NULL;
    }

    // Count lines
    int count = 0;
    for (size_t i = 0; i < total; i++) {
        if (buf->data[i] == '\n') count++;
    }
    if (total > 0 && buf->data[total - 1] != '\n') count++;

    if (count == 0) {
        buffer_free(buf);
        return NULL;
    }

    char** files = malloc(count * sizeof(char*));
    if (!files) {
        buffer_free(buf);
        return NULL;
    }

    int idx = 0;
    char* start = buf->data;
    for (size_t i = 0; i < total && idx < count; i++) {
        if (buf->data[i] == '\n') {
            size_t len = i - (start - buf->data);
            if (len > 0) {
                files[idx] = malloc(len + 1);
                if (files[idx]) {
                    memcpy(files[idx], start, len);
                    files[idx][len] = '\0';
                    idx++;
                }
            }
            start = buf->data + i + 1;
        }
    }

    buffer_free(buf);

    *n_files = idx;
    return files;
}

char* vcs_get_file_at_commit(const char* repo_path, const char* commit, const char* filepath,
                             size_t* content_len) {
    *content_len = 0;
    if (!repo_path || !commit || !filepath) return NULL;

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git show '%s:%s' 2>/dev/null", repo_path, commit,
             filepath);

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

// === vcs_get_changed_files Implementation ===

// Validate that a git reference (commit, branch, tag) is valid in the given repo
static int vcs_validate_git_ref(const char* repo_path, const char* ref) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git rev-parse --verify '%s' 2>/dev/null", repo_path,
             ref);
    FILE* fp = popen(cmd, "r");
    if (!fp) return 0;
    char buf[64];
    int valid = (fread(buf, 1, sizeof(buf), fp) > 0);
    pclose(fp);
    return valid;
}

char** vcs_get_changed_files(const char* repo, const char* ref1, const char* ref2,
                             unsigned int flags, int* n_files) {
    *n_files = 0;
    if (!repo || !ref1 || !ref2) return NULL;

    // Validate both references exist
    if (!vcs_validate_git_ref(repo, ref1) || !vcs_validate_git_ref(repo, ref2)) {
        return NULL;
    }

    // Build git diff command
    char cmd[2048];
    int cmd_len = 0;
    cmd_len = snprintf(cmd, sizeof(cmd), "cd '%s' && git diff --name-status", repo);

    // Apply flags
    if (flags & VCS_DIFF_IGNORE_WHITESPACE) {
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " -w");
    }

    cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, " '%s' '%s' 2>/dev/null", ref1, ref2);

    if (flags & VCS_DIFF_INCLUDE_SUBMODULES) {
        // Append submodule diff output
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len,
                            " && cd '%s' && git diff --name-status -w --ignore-submodules=none "
                            "'%s' '%s' 2>/dev/null",
                            repo, ref1, ref2);
    }

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    Buffer* buf = buffer_new(65536);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    char read_buf[8192];
    size_t chunk;
    while ((chunk = fread(read_buf, 1, sizeof(read_buf), fp)) > 0) {
        if (buffer_append(buf, read_buf, chunk) != 0) break;
    }
    pclose(fp);

    size_t total = buf->size;
    if (total == 0) {
        buffer_free(buf);
        return NULL;
    }

    // Count non-empty lines
    int count = 0;
    char* line_start = buf->data;
    for (size_t i = 0; i < total; i++) {
        if (buf->data[i] == '\n') {
            size_t len = i - (line_start - buf->data);
            if (len > 0) count++;
            line_start = buf->data + i + 1;
        }
    }
    // Handle last line
    if (line_start < buf->data + total && (total - (line_start - buf->data)) > 0) {
        count++;
    }

    if (count == 0) {
        buffer_free(buf);
        return NULL;
    }

    char** files = malloc(count * sizeof(char*));
    if (!files) {
        buffer_free(buf);
        return NULL;
    }

    // De-duplicate lines (submodule mode may produce duplicates)
    int idx = 0;
    line_start = buf->data;
    for (size_t i = 0; i < total && idx < count; i++) {
        if (buf->data[i] == '\n') {
            size_t len = i - (line_start - buf->data);
            if (len > 0) {
                // Check for duplicates
                int is_dup = 0;
                for (int j = 0; j < idx; j++) {
                    if (strlen(files[j]) == len && strncmp(files[j], line_start, len) == 0) {
                        is_dup = 1;
                        break;
                    }
                }
                if (!is_dup) {
                    files[idx] = malloc(len + 1);
                    if (files[idx]) {
                        memcpy(files[idx], line_start, len);
                        files[idx][len] = '\0';
                        idx++;
                    }
                }
            }
            line_start = buf->data + i + 1;
        }
    }
    // Handle last line
    if (idx < count && line_start < buf->data + total) {
        size_t len = total - (line_start - buf->data);
        if (len > 0) {
            int is_dup = 0;
            for (int j = 0; j < idx; j++) {
                if (strlen(files[j]) == len && strncmp(files[j], line_start, len) == 0) {
                    is_dup = 1;
                    break;
                }
            }
            if (!is_dup) {
                files[idx] = malloc(len + 1);
                if (files[idx]) {
                    memcpy(files[idx], line_start, len);
                    files[idx][len] = '\0';
                    idx++;
                }
            }
        }
    }

    buffer_free(buf);

    *n_files = idx;
    return files;
}

char** vcs_get_diff_files(const char* repo_path, const char* commit1, const char* commit2,
                          int* n_files) {
    *n_files = 0;
    if (!repo_path || !commit1 || !commit2) return NULL;

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git diff --name-status '%s' '%s' 2>/dev/null", repo_path,
             commit1, commit2);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    Buffer* buf = buffer_new(65536);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    char read_buf[8192];
    size_t chunk;
    while ((chunk = fread(read_buf, 1, sizeof(read_buf), fp)) > 0) {
        if (buffer_append(buf, read_buf, chunk) != 0) break;
    }
    pclose(fp);

    size_t total = buf->size;
    if (total == 0) {
        buffer_free(buf);
        return NULL;
    }

    int count = 0;
    for (size_t i = 0; i < total; i++) {
        if (buf->data[i] == '\n') count++;
    }

    if (count == 0) {
        buffer_free(buf);
        return NULL;
    }

    char** files = malloc(count * sizeof(char*));
    if (!files) {
        buffer_free(buf);
        return NULL;
    }

    int idx = 0;
    char* line_start = buf->data;
    for (size_t i = 0; i < total && idx < count; i++) {
        if (buf->data[i] == '\n') {
            size_t line_len = i - (line_start - buf->data);
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
            line_start = buf->data + i + 1;
        }
    }

    buffer_free(buf);

    *n_files = idx;
    return files;
}