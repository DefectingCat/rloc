#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>

int is_regular_file(const char* path);
int is_directory(const char* path);
int is_symlink(const char* path);
long get_file_size(const char* path);
char* path_join(const char* dir, const char* filename);
int is_binary_file(const char* path);

/* Buffer dynamic buffer */
typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} Buffer;

Buffer* buffer_new(size_t initial_capacity);
int buffer_append(Buffer* buf, const char* data, size_t len);
void buffer_free(Buffer* buf);
char* buffer_steal(Buffer* buf, size_t* out_size);
void buffer_clear(Buffer* buf);
int buffer_reserve(Buffer* buf, size_t min_capacity);

#endif
