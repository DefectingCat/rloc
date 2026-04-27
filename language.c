#include "language.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lang_defs.h"

/* External language definitions array */
extern const Language g_languages[];

/* Forward declaration */
static const Language* detect_shebang_from_buffer(const char* buffer, size_t bytes_read);

/* Helper: Check if a string ends with a specific extension */
static bool has_extension(const char* filepath, const char* ext) {
    size_t filepath_len = strlen(filepath);
    size_t ext_len = strlen(ext);

    if (filepath_len < ext_len + 2) { /* Need at least "." + ext + one char before */
        return false;
    }

    /* Check that there's a dot before the extension */
    size_t dot_pos = filepath_len - ext_len - 1;
    if (filepath[dot_pos] != '.') {
        return false;
    }

    /* Check if the extension matches (case-insensitive for Windows compatibility) */
    return strcasecmp(filepath + dot_pos + 1, ext) == 0;
}

/* Helper: Check if filepath has any of the comma-separated extensions */
static bool matches_extensions(const char* filepath, const char* extensions) {
    if (!extensions) return false;

    char* ext_copy = strdup(extensions);
    char* saveptr = NULL;
    char* ext = strtok_r(ext_copy, ",", &saveptr);
    bool found = false;

    while (ext && !found) {
        /* Trim whitespace */
        while (isspace((unsigned char)*ext)) ext++;

        if (has_extension(filepath, ext)) {
            found = true;
        }
        ext = strtok_r(NULL, ",", &saveptr);
    }

    free(ext_copy);
    return found;
}

/* Helper: Extract filename from path */
static const char* get_filename(const char* filepath) {
    const char* filename = strrchr(filepath, '/');
    if (filename) {
        return filename + 1;
    }

    /* Also check for Windows path separator */
    filename = strrchr(filepath, '\\');
    if (filename) {
        return filename + 1;
    }

    return filepath;
}

/* Helper: Check if filename matches any of the comma-separated exact filenames
 */
static bool matches_filenames(const char* filepath, const char* filenames) {
    if (!filenames) return false;

    const char* filename = get_filename(filepath);

    char* name_copy = strdup(filenames);
    char* saveptr = NULL;
    char* name = strtok_r(name_copy, ",", &saveptr);
    bool found = false;

    while (name && !found) {
        /* Trim whitespace */
        while (isspace((unsigned char)*name)) name++;

        if (strcasecmp(filename, name) == 0) {
            found = true;
        }
        name = strtok_r(NULL, ",", &saveptr);
    }

    free(name_copy);
    return found;
}

/* Detect language by filepath (filename or extension) */
const Language* detect_language(const char* filepath) {
    if (!filepath) return NULL;

    /* Try content detection first (highest priority) */
    const Language* content_lang = detect_language_by_content(filepath);
    if (content_lang) return content_lang;

    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        const Language* lang = &g_languages[i];

        /* Try exact filename match */
        if (matches_filenames(filepath, lang->filenames)) {
            return lang;
        }

        /* Then try extension match */
        if (matches_extensions(filepath, lang->extensions)) {
            return lang;
        }
    }

    /* Fallback to shebang detection */
    return detect_language_by_shebang(filepath);
}

/* Detect language by reading the file's shebang */
const Language* detect_language_by_shebang(const char* filepath) {
    if (!filepath) return NULL;

    FILE* fp = fopen(filepath, "r");
    if (!fp) return NULL;

    /* Read first 256 bytes to find shebang */
    char buffer[256];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[bytes_read] = '\0';
    fclose(fp);

    return detect_shebang_from_buffer(buffer, bytes_read);
}

/* Helper: Detect shebang from buffer content */
const Language* detect_shebang_from_buffer(const char* buffer, size_t bytes_read) {
    /* Check if file starts with #! */
    if (bytes_read < 2 || buffer[0] != '#' || buffer[1] != '!') {
        return NULL;
    }

    /* Parse shebang line */
    char shebang_buf[256];
    memcpy(shebang_buf, buffer, bytes_read < 256 ? bytes_read : 255);
    shebang_buf[bytes_read < 256 ? bytes_read : 255] = '\0';

    char* shebang_end = strchr(shebang_buf, '\n');
    if (shebang_end) *shebang_end = '\0';

    /* Extract interpreter path (skip #! and whitespace) */
    char* interpreter = shebang_buf + 2;
    while (isspace((unsigned char)*interpreter)) interpreter++;

    /* Get just the interpreter name (without path) */
    const char* interpreter_name = strrchr(interpreter, '/');
    if (interpreter_name) {
        interpreter_name++;
    } else {
        interpreter_name = interpreter;
    }

    /* Match against language shebang patterns */
    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        const Language* lang = &g_languages[i];

        if (!lang->shebangs) continue;

        char* shebang_copy = strdup(lang->shebangs);
        char* saveptr = NULL;
        char* pattern = strtok_r(shebang_copy, ",", &saveptr);
        bool matched = false;

        while (pattern && !matched) {
            while (isspace((unsigned char)*pattern)) pattern++;

            if (strstr(interpreter_name, pattern) == interpreter_name) {
                matched = true;
            }
            pattern = strtok_r(NULL, ",", &saveptr);
        }

        free(shebang_copy);

        if (matched) {
            return lang;
        }
    }

    return NULL;
}

/* Detect language by file content patterns */
const Language* detect_language_by_content(const char* filepath) {
    if (!filepath) return NULL;

    FILE* fp = fopen(filepath, "r");
    if (!fp) return NULL;

    /* Read first 512 bytes for content detection */
    char buffer[512];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[bytes_read] = '\0';
    fclose(fp);

    /* Check each language's content patterns */
    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        const Language* lang = &g_languages[i];

        if (!lang->content_patterns) continue;

        char* patterns_copy = strdup(lang->content_patterns);
        char* saveptr = NULL;
        char* pattern = strtok_r(patterns_copy, ",", &saveptr);
        int match_count = 0;

        while (pattern) {
            while (isspace((unsigned char)*pattern)) pattern++;

            if (strlen(pattern) > 0 && strstr(buffer, pattern)) {
                match_count++;
            }
            pattern = strtok_r(NULL, ",", &saveptr);
        }

        free(patterns_copy);

        /* Require at least 2 pattern matches for confidence */
        if (match_count >= 2) {
            return lang;
        }
    }

    return NULL;
}

/* Get language by name (linear search) */
const Language* get_language_by_name(const char* name) {
    if (!name) return NULL;

    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        if (strcasecmp(g_languages[i].name, name) == 0) {
            return &g_languages[i];
        }
    }

    return NULL;
}

/* Explain comment filters for a language */
void explain_language(const char* lang_name) {
    if (!lang_name) {
        fprintf(stderr, "Error: No language specified for --explain\n");
        return;
    }

    const Language* lang = get_language_by_name(lang_name);
    if (!lang) {
        fprintf(stderr, "Error: Unknown language '%s'\n", lang_name);
        fprintf(stderr, "Use --show-lang to see supported languages\n");
        return;
    }

    printf("Language: %s\n", lang->name);
    printf("Extensions: %s\n", lang->extensions ? lang->extensions : "(none)");
    printf("Filenames: %s\n", lang->filenames ? lang->filenames : "(none)");
    printf("Shebangs: %s\n", lang->shebangs ? lang->shebangs : "(none)");
    printf("\n");

    if (lang->generic_filter_count == 0) {
        printf("Comment filters: None (language has no defined comment patterns)\n");
    } else {
        printf("Comment filters (%zu):\n", lang->generic_filter_count);
        for (size_t i = 0; i < lang->generic_filter_count; i++) {
            const GenericFilter* f = &lang->generic_filters[i];
            switch (f->type) {
                case FILTER_REMOVE_MATCHES:
                    printf("  [%zu] REMOVE_MATCHES: pattern='%s'\n", i + 1, f->pattern_open);
                    printf("       Removes entire lines matching this pattern\n");
                    break;
                case FILTER_REMOVE_INLINE:
                    printf("  [%zu] REMOVE_INLINE: pattern='%s'\n", i + 1, f->pattern_open);
                    printf("       Removes inline comments (from pattern to end of line)\n");
                    break;
                case FILTER_REMOVE_BETWEEN:
                    printf("  [%zu] REMOVE_BETWEEN: open='%s', close='%s'\n", i + 1, f->pattern_open, f->pattern_close);
                    printf("       Removes content between open and close patterns (multi-line blocks)\n");
                    break;
            }
        }
    }

    if (lang->comment_hook) {
        printf("\nCustom comment hook: Yes (language-specific handling)\n");
    }

    if (lang->str_delimiters) {
        printf("\nString delimiters: '%s'\n", lang->str_delimiters);
        printf("String escape: '%s'\n", lang->str_escape ? lang->str_escape : "(none)");
    }
}
