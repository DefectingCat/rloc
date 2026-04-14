#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>

int is_regular_file(const char *path);
int is_directory(const char *path);
int is_symlink(const char *path);
long get_file_size(const char *path);
char *path_join(const char *dir, const char *filename);
int is_binary_file(const char *path);

#endif
