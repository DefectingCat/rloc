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

// Legacy functions (no language awareness)
int count_file(const char* filepath, CountResult* result);
void count_lines(const char* src, size_t len, CountResult* result);

// Language-aware counting with skip_lines support
int count_file_with_lang(const char* filepath, const Language* lang, int skip_lines,
                         CountResult* result);
void count_lines_with_lang(const char* src, size_t len, const Language* lang, int skip_lines,
                           CountResult* result);

#endif