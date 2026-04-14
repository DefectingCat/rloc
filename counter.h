#ifndef COUNTER_H
#define COUNTER_H

#include <stddef.h>
#include "language.h"

typedef struct {
    int blank;
    int comment;
    int code;
    int total;
} CountResult;

// Count lines in a file without language awareness (legacy, keep for tests)
int count_file(const char *filepath, CountResult *result);

// Count lines from a string buffer (legacy, keep for tests)
void count_lines(const char *src, size_t len, CountResult *result);

// Count lines with language-aware comment detection
// Uses Language's filter config and string literal stripping
int count_file_with_lang(const char *filepath, const Language *lang, CountResult *result);

// Count lines from buffer with language-aware comment detection
void count_lines_with_lang(const char *src, size_t len, const Language *lang, CountResult *result);

#endif