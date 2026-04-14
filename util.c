#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int is_regular_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISREG(st.st_mode);
}

int is_directory(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

int is_symlink(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        return 0;
    }
    return S_ISLNK(st.st_mode);
}

long get_file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }
    return st.st_size;
}

char* path_join(const char* dir, const char* filename) {
    size_t dir_len = strlen(dir);
    size_t filename_len = strlen(filename);

    // Check if dir already has trailing slash
    int has_trailing_slash = (dir_len > 0 && dir[dir_len - 1] == '/');

    // Calculate total length
    size_t total_len = dir_len + filename_len + (has_trailing_slash ? 0 : 1) + 1;

    char* result = malloc(total_len);
    if (!result) {
        return NULL;
    }

    if (has_trailing_slash) {
        snprintf(result, total_len, "%s%s", dir, filename);
    } else {
        snprintf(result, total_len, "%s/%s", dir, filename);
    }

    return result;
}

int is_binary_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return 0;  // Can't open, treat as non-binary
    }

    unsigned char buffer[8192];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    if (bytes_read == 0) {
        return 0;  // Empty file
    }

    // Check for null bytes in the first 8192 bytes
    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] == '\0') {
            return 1;  // Binary file found
        }
    }

    return 0;  // No null bytes, likely text
}
