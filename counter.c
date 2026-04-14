#include "counter.h"
#include "strlit.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static int is_blank_line(const char *start, const char *end) {
    const char *p = start;
    while (p < end) {
        if (!isspace((unsigned char)*p)) {
            return 0;
        }
        p++;
    }
    return 1;
}

static const char *trim_leading_whitespace(const char *start, const char *end) {
    const char *p = start;
    while (p < end && isspace((unsigned char)*p)) {
        p++;
    }
    return p;
}

static int starts_with_comment_marker(const char *line_start, const char *line_end,
                                       const Language *lang) {
    if (!lang || !lang->generic_filters) {
        return 0;
    }

    const char *trimmed = trim_leading_whitespace(line_start, line_end);

    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        const GenericFilter *filter = &lang->generic_filters[i];

        if (filter->type == FILTER_REMOVE_INLINE) {
            size_t pattern_len = strlen(filter->pattern_open);
            if ((size_t)(line_end - trimmed) >= pattern_len &&
                strncmp(trimmed, filter->pattern_open, pattern_len) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

void count_lines(const char *src, size_t len, CountResult *result) {
    result->blank = 0;
    result->comment = 0;
    result->code = 0;
    result->total = 0;

    if (len == 0) {
        return;
    }

    const char *line_start = src;
    const char *p = src;
    const char *end = src + len;

    while (p < end) {
        if (*p == '\n') {
            result->total++;
            if (is_blank_line(line_start, p)) {
                result->blank++;
            } else {
                result->code++;
            }
            line_start = p + 1;
        }
        p++;
    }

    // Handle last line if no trailing newline
    if (line_start < end) {
        result->total++;
        if (is_blank_line(line_start, end)) {
            result->blank++;
        } else {
            result->code++;
        }
    }
}

void count_lines_with_lang(const char *src, size_t len, const Language *lang, CountResult *result) {
    result->blank = 0;
    result->comment = 0;
    result->code = 0;
    result->total = 0;

    if (len == 0) {
        return;
    }

    // If lang is NULL, fall back to old behavior
    if (lang == NULL) {
        count_lines(src, len, result);
        return;
    }

    // Check if language has FILTER_REMOVE_INLINE filters
    int has_remove_inline = 0;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE) {
            has_remove_inline = 1;
            break;
        }
    }

    const char *work_src = src;
    size_t work_len = len;
    char *stripped_buffer = NULL;

    // If language has FILTER_REMOVE_INLINE filters, strip string literals first
    if (has_remove_inline && lang->str_delimiters) {
        stripped_buffer = (char *)malloc(len);
        if (stripped_buffer) {
            strip_string_literals(src, len, stripped_buffer,
                                 lang->str_delimiters,
                                 lang->str_escape);
            work_src = stripped_buffer;
            // Note: strip_string_literals returns original length, but we work with actual content
        }
    }

    const char *line_start = work_src;
    const char *p = work_src;
    const char *end = work_src + work_len;

    while (p < end) {
        if (*p == '\n') {
            result->total++;
            if (is_blank_line(line_start, p)) {
                result->blank++;
            } else if (starts_with_comment_marker(line_start, p, lang)) {
                result->comment++;
            } else {
                result->code++;
            }
            line_start = p + 1;
        }
        p++;
    }

    // Handle last line if no trailing newline
    if (line_start < end) {
        result->total++;
        if (is_blank_line(line_start, end)) {
            result->blank++;
        } else if (starts_with_comment_marker(line_start, end, lang)) {
            result->comment++;
        } else {
            result->code++;
        }
    }

    if (stripped_buffer) {
        free(stripped_buffer);
    }
}

int count_file(const char *filepath, CountResult *result) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        return -1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size == 0) {
        fclose(file);
        result->blank = 0;
        result->comment = 0;
        result->code = 0;
        result->total = 0;
        return 0;
    }

    // Read entire file
    char *buffer = (char *)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        return -1;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    count_lines(buffer, bytes_read, result);

    free(buffer);
    return 0;
}

int count_file_with_lang(const char *filepath, const Language *lang, CountResult *result) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        return -1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size == 0) {
        fclose(file);
        result->blank = 0;
        result->comment = 0;
        result->code = 0;
        result->total = 0;
        return 0;
    }

    // Read entire file
    char *buffer = (char *)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        return -1;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    count_lines_with_lang(buffer, bytes_read, lang, result);

    free(buffer);
    return 0;
}