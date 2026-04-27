#include "counter.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strlit.h"

static int is_blank_line(const char* start, const char* end) {
    const char* p = start;
    while (p < end) {
        if (!isspace((unsigned char)*p)) {
            return 0;
        }
        p++;
    }
    return 1;
}

static const char* trim_leading_whitespace(const char* start, const char* end) {
    const char* p = start;
    while (p < end && isspace((unsigned char)*p)) {
        p++;
    }
    return p;
}

static int starts_with_comment_marker(const char* line_start, const char* line_end,
                                      const Language* lang) {
    if (!lang || !lang->generic_filters) {
        return 0;
    }

    const char* trimmed = trim_leading_whitespace(line_start, line_end);

    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        const GenericFilter* filter = &lang->generic_filters[i];

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

/* Helper: find if a line contains a block comment open marker.
 * Returns pointer to the start of the marker, or NULL if not found.
 * Sets *found_filter to the matching filter if found. */
static const char* contains_block_open(const char* line_start, const char* line_end,
                                       const Language* lang, const GenericFilter** found_filter) {
    if (!lang || !lang->generic_filters) {
        return NULL;
    }

    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        const GenericFilter* filter = &lang->generic_filters[i];
        if (filter->type == FILTER_REMOVE_BETWEEN) {
            size_t pattern_len = strlen(filter->pattern_open);
            /* Search for the open pattern anywhere in the line */
            for (const char* p = line_start; p <= line_end - (long)pattern_len; p++) {
                if (strncmp(p, filter->pattern_open, pattern_len) == 0) {
                    *found_filter = filter;
                    return p;
                }
            }
        }
    }
    return NULL;
}

/* Helper: find if a line contains a block comment close marker for the given filter.
 * Returns pointer to the end of the marker, or NULL if not found. */
static const char* contains_block_close(const char* line_start, const char* line_end,
                                        const GenericFilter* filter) {
    if (!filter || !filter->pattern_close) {
        return NULL;
    }

    size_t pattern_len = strlen(filter->pattern_close);
    /* Search for the close pattern anywhere in the line */
    for (const char* p = line_start; p <= line_end - (long)pattern_len; p++) {
        if (strncmp(p, filter->pattern_close, pattern_len) == 0) {
            return p + pattern_len; /* Return pointer after the close marker */
        }
    }
    return NULL;
}

/* Helper: check if line has non-whitespace content between two positions */
static int has_content_between(const char* start, const char* end) {
    for (const char* p = start; p < end; p++) {
        if (!isspace((unsigned char)*p)) {
            return 1;
        }
    }
    return 0;
}

/* Helper: check if line has non-whitespace content after a position */
static int has_content_after(const char* start, const char* end) {
    for (const char* p = start; p < end; p++) {
        if (!isspace((unsigned char)*p)) {
            return 1;
        }
    }
    return 0;
}

/* Helper: check if line ends with backslash (continuation) */
static int line_ends_with_backslash(const char* line_start, const char* line_end) {
    /* Find last non-whitespace character before newline */
    const char* p = line_end - 1;
    while (p >= line_start && isspace((unsigned char)*p)) {
        p--;
    }
    /* Check if the last non-whitespace char is backslash */
    return (p >= line_start && *p == '\\');
}

void count_lines(const char* src, size_t len, CountResult* result) {
    result->blank = 0;
    result->comment = 0;
    result->code = 0;
    result->total = 0;

    if (len == 0) {
        return;
    }

    const char* line_start = src;
    const char* p = src;
    const char* end = src + len;

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

void count_lines_with_lang(const char* src, size_t len, const Language* lang, int skip_lines,
                           CountResult* result) {
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

    // Check if language has any filter types that need string literal stripping
    int has_remove_inline = 0;
    int has_remove_between = 0;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE) {
            has_remove_inline = 1;
        }
        if (lang->generic_filters[i].type == FILTER_REMOVE_BETWEEN) {
            has_remove_between = 1;
        }
    }

    const char* work_src = src;
    size_t work_len = len;
    char* stripped_buffer = NULL;

    // Skip leading lines if requested
    if (skip_lines > 0) {
        const char* p = work_src;
        const char* end_skip = work_src + work_len;
        int lines_to_skip = skip_lines;
        while (p < end_skip && lines_to_skip > 0) {
            if (*p == '\n') {
                lines_to_skip--;
                work_src = p + 1;
            }
            p++;
        }
        if (lines_to_skip > 0 && work_src < end_skip) {
            work_src = end_skip;
        }
        work_len = end_skip - work_src;
    }

    // If language has filters that need string literal stripping, do it first
    if ((has_remove_inline || has_remove_between) && lang->str_delimiters) {
        stripped_buffer = (char*)malloc(len);
        if (stripped_buffer) {
            strip_string_literals(src, len, stripped_buffer, lang->str_delimiters,
                                  lang->str_escape);
            work_src = stripped_buffer;
            // Note: strip_string_literals returns original length, but we work with
            // actual content
        }
    }

    // Skip leading lines after stripping is done
    if (skip_lines > 0) {
        const char* p2 = work_src;
        const char* end_skip = work_src + work_len;
        int lines_to_skip = skip_lines;
        while (p2 < end_skip && lines_to_skip > 0) {
            if (*p2 == '\n') {
                lines_to_skip--;
                work_src = p2 + 1;
            }
            p2++;
        }
        if (lines_to_skip > 0 && work_src < end_skip) {
            work_src = end_skip;
        }
        work_len = end_skip - work_src;
    }

    const char* line_start = work_src;
    const char* p = work_src;
    const char* end = work_src + work_len;

    // Block comment state tracking
    int in_block_comment = 0;
    const GenericFilter* active_block_filter = NULL;

    // Continuation line tracking
    int prev_line_type = 0;  // 0=blank, 1=comment, 2=code
    int is_continuation = 0;

    while (p < end) {
        if (*p == '\n') {
            result->total++;
            const char* line_end = p;

            // Check if this line ends with backslash (continuation)
            int ends_with_backslash = line_ends_with_backslash(line_start, line_end);

            if (is_continuation) {
                // This is a continuation line - inherit previous classification
                result->comment += (prev_line_type == 1);
                result->code += (prev_line_type == 2);
                result->blank += (prev_line_type == 0);
                // Reset continuation if this line doesn't continue further
                is_continuation = ends_with_backslash;
            } else if (is_blank_line(line_start, line_end)) {
                result->blank++;
                prev_line_type = 0;
            } else if (in_block_comment) {
                // Currently inside a block comment
                const char* after_close =
                    contains_block_close(line_start, line_end, active_block_filter);
                if (after_close) {
                    // Block comment ends on this line
                    // Check if there's code after the closing marker
                    if (has_content_after(after_close, line_end)) {
                        result->code++;
                    } else {
                        result->comment++;
                    }
                    in_block_comment = 0;
                    active_block_filter = NULL;
                } else {
                    // Entire line is inside block comment
                    result->comment++;
                    prev_line_type = 1;
                }
            } else {
                // Not in a block comment, check for block comment start
                const GenericFilter* block_filter = NULL;
                const char* open_pos =
                    contains_block_open(line_start, line_end, lang, &block_filter);
                if (open_pos) {
                    size_t open_len = strlen(block_filter->pattern_open);
                    const char* after_open = open_pos + open_len;
                    // Check for closing marker on same line
                    const char* after_close =
                        contains_block_close(after_open, line_end, block_filter);
                    if (after_close) {
                        // Block comment opens and closes on same line
                        // Check if there's code before the opening marker or after the closing
                        // marker
                        if (has_content_between(line_start, open_pos) ||
                            has_content_after(after_close, line_end)) {
                            result->code++;
                        } else {
                            result->comment++;
                        }
                    } else {
                        // Block comment starts on this line and continues
                        // Check if there's code before the opening marker
                        if (has_content_between(line_start, open_pos)) {
                            result->code++;
                        } else {
                            result->comment++;
                        }
                        in_block_comment = 1;
                        active_block_filter = block_filter;
                    }
                } else if (starts_with_comment_marker(line_start, line_end, lang)) {
                    result->comment++;
                    prev_line_type = 1;
                } else {
                    result->code++;
                    prev_line_type = 2;
                }
                // Set continuation if this line ends with backslash
                is_continuation = ends_with_backslash;
            }
            line_start = p + 1;
        }
        p++;
    }

    // Handle last line if no trailing newline
    if (line_start < end) {
        result->total++;
        const char* line_end = end;

        if (is_blank_line(line_start, line_end)) {
            result->blank++;
        } else if (in_block_comment) {
            // Currently inside a block comment
            const char* after_close =
                contains_block_close(line_start, line_end, active_block_filter);
            if (after_close) {
                // Block comment ends on this line
                if (has_content_after(after_close, line_end)) {
                    result->code++;
                } else {
                    result->comment++;
                }
                in_block_comment = 0;
                active_block_filter = NULL;
            } else {
                // Entire line is inside block comment
                result->comment++;
            }
        } else {
            // Not in a block comment, check for block comment start
            const GenericFilter* block_filter = NULL;
            const char* open_pos = contains_block_open(line_start, line_end, lang, &block_filter);
            if (open_pos) {
                size_t open_len = strlen(block_filter->pattern_open);
                const char* after_open = open_pos + open_len;
                // Check for closing marker on same line
                const char* after_close = contains_block_close(after_open, line_end, block_filter);
                if (after_close) {
                    // Block comment opens and closes on same line
                    if (has_content_between(line_start, open_pos) ||
                        has_content_after(after_close, line_end)) {
                        result->code++;
                    } else {
                        result->comment++;
                    }
                } else {
                    // Block comment starts on this line and continues
                    if (has_content_between(line_start, open_pos)) {
                        result->code++;
                    } else {
                        result->comment++;
                    }
                    // Note: unclosed block comment at end of file, but we still count this line
                }
            } else if (starts_with_comment_marker(line_start, line_end, lang)) {
                result->comment++;
            } else {
                result->code++;
            }
        }
    }

    if (stripped_buffer) {
        free(stripped_buffer);
    }
}

int count_file(const char* filepath, CountResult* result) {
    FILE* file = fopen(filepath, "rb");
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
    char* buffer = (char*)malloc(file_size);
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

int count_file_with_lang(const char* filepath, const Language* lang, int skip_lines,
                         CountResult* result) {
    FILE* file = fopen(filepath, "rb");
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
    char* buffer = (char*)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        return -1;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    count_lines_with_lang(buffer, bytes_read, lang, skip_lines, result);

    free(buffer);
    return 0;
}