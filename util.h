#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

/* FileInfo - cached file attributes from single stat call */
typedef struct {
    mode_t mode;
    off_t size;
    bool is_symlink;
    bool is_dir;
    bool is_regular;
} FileInfo;

int get_file_info(const char* path, FileInfo* info);

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
