#include "strlit.h"

#include <string.h>

typedef enum { STATE_CODE, STATE_STRING } State;

size_t strip_string_literals(const char* src, size_t len, char* dst, const char* delimiters,
                             const char* escape) {
    State state = STATE_CODE;
    char current_delimiter = '\0';
    size_t dst_idx = 0;
    int skip_next = 0;

    for (size_t i = 0; i < len; i++) {
        if (skip_next) {
            skip_next = 0;
            continue;
        }

        char c = src[i];

        switch (state) {
            case STATE_CODE:
                // Check if this character starts a string literal
                if (strchr(delimiters, c) != NULL) {
                    // Check for character literals like 'a' (single character between
                    // single quotes)
                    if (c == '\'' && i + 2 < len && src[i + 2] == '\'') {
                        // Skip character literal entirely
                        dst[dst_idx++] = c;           // Copy opening quote
                        dst[dst_idx++] = src[i + 1];  // Copy character
                        i += 2;                       // Skip to closing quote
                        dst[dst_idx++] = c;           // Copy closing quote
                        continue;
                    }
                    // Start of string literal
                    state = STATE_STRING;
                    current_delimiter = c;
                    dst[dst_idx++] = c;  // Copy the opening delimiter
                } else {
                    // Regular code, copy as-is
                    dst[dst_idx++] = c;
                }
                break;

            case STATE_STRING:
                if (escape && strchr(escape, c) != NULL) {
                    // Escape character - skip the next character
                    skip_next = 1;
                } else if (c == current_delimiter) {
                    // Closing delimiter - return to CODE state
                    state = STATE_CODE;
                    current_delimiter = '\0';
                    dst[dst_idx++] = c;  // Copy the closing delimiter
                } else if (c == '\n') {
                    // Preserve newlines in strings
                    dst[dst_idx++] = c;
                } else {
                    // Replace other string content with space
                    dst[dst_idx++] = ' ';
                }
                break;
        }
    }

    return len;  // Return original length (same size in/out)
}
