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

/* === Buffer Dynamic Buffer Implementation === */

Buffer* buffer_new(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 1024;
    }

    Buffer* buf = malloc(sizeof(Buffer));
    if (!buf) {
        return NULL;
    }

    buf->data = malloc(initial_capacity);
    if (!buf->data) {
        free(buf);
        return NULL;
    }

    buf->size = 0;
    buf->capacity = initial_capacity;
    return buf;
}

int buffer_append(Buffer* buf, const char* data, size_t len) {
    if (!buf || !data || len == 0) {
        return -1;
    }

    size_t needed = buf->size + len + 1;
    if (needed > buf->capacity) {
        // Double capacity until it fits
        size_t new_capacity = buf->capacity;
        while (new_capacity < needed) {
            new_capacity *= 2;
        }
        char* new_data = realloc(buf->data, new_capacity);
        if (!new_data) {
            return -1;
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }

    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
    return 0;
}

void buffer_free(Buffer* buf) {
    if (!buf) return;
    free(buf->data);
    free(buf);
}

char* buffer_steal(Buffer* buf, size_t* out_size) {
    if (!buf) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    char* data = buf->data;
    size_t size = buf->size;

    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
    free(buf);

    if (out_size) *out_size = size;
    return data;
}

void buffer_clear(Buffer* buf) {
    if (!buf) return;
    buf->size = 0;
    if (buf->data) {
        buf->data[0] = '\0';
    }
}

int buffer_reserve(Buffer* buf, size_t min_capacity) {
    if (!buf) return -1;
    if (min_capacity <= buf->capacity) return 0;

    char* new_data = realloc(buf->data, min_capacity);
    if (!new_data) {
        return -1;
    }
    buf->data = new_data;
    buf->capacity = min_capacity;
    return 0;
}
