#ifndef STRLIT_H
#define STRLIT_H
#include <stddef.h>
size_t strip_string_literals(const char* src, size_t len, char* dst, const char* delimiters,
                             const char* escape);
#endif
