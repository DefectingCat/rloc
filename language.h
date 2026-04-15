#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <stdbool.h>
#include <stddef.h>

/* Filter types for removing comments and strings */
typedef enum {
    FILTER_REMOVE_MATCHES, /* Remove lines matching pattern */
    FILTER_REMOVE_INLINE,  /* Remove inline comments from pattern to end of line */
    FILTER_REMOVE_BETWEEN  /* Remove content between pattern_open and pattern_close
                            */
} FilterType;

/* Generic filter for comments/strings */
typedef struct {
    FilterType type;
    const char* pattern_open;
    const char* pattern_close;
} GenericFilter;

/* Function pointer for complex language-specific comment handling */
typedef bool (*CommentHookFn)(const char* line, size_t line_len, size_t* comment_start);

/* Language definition */
typedef struct {
    const char* name;
    const char* extensions; /* Comma-separated, without dots */
    const char* filenames;  /* Comma-separated exact filenames */
    const char* shebangs;   /* Comma-separated interpreter patterns */
    size_t generic_filter_count;
    const GenericFilter* generic_filters;
    CommentHookFn comment_hook;
    const char* str_delimiters; /* String delimiter characters */
    const char* str_escape;     /* Escape character for strings */
} Language;

/* API functions */
const Language* detect_language(const char* filepath);
const Language* detect_language_by_shebang(const char* filepath);
const Language* get_language_by_name(const char* name);

/* Diagnostic functions */
void explain_language(const char* lang_name);

#endif /* LANGUAGE_H */
