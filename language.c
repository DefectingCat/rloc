#include "language.h"
#include "lang_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* External language definitions array */
extern const Language g_languages[];

/* Helper: Check if a string ends with a specific extension */
static bool has_extension(const char *filepath, const char *ext) {
    size_t filepath_len = strlen(filepath);
    size_t ext_len = strlen(ext);

    if (filepath_len < ext_len + 1) {  /* Need at least "." + ext */
        return false;
    }

    /* Check if the extension matches (case-insensitive for Windows compatibility) */
    return strcasecmp(filepath + filepath_len - ext_len, ext) == 0;
}

/* Helper: Check if filepath has any of the comma-separated extensions */
static bool matches_extensions(const char *filepath, const char *extensions) {
    if (!extensions) return false;

    char *ext_copy = strdup(extensions);
    char *saveptr = NULL;
    char *ext = strtok_r(ext_copy, ",", &saveptr);
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
static const char *get_filename(const char *filepath) {
    const char *filename = strrchr(filepath, '/');
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

/* Helper: Check if filename matches any of the comma-separated exact filenames */
static bool matches_filenames(const char *filepath, const char *filenames) {
    if (!filenames) return false;

    const char *filename = get_filename(filepath);

    char *name_copy = strdup(filenames);
    char *saveptr = NULL;
    char *name = strtok_r(name_copy, ",", &saveptr);
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
const Language *detect_language(const char *filepath) {
    if (!filepath) return NULL;

    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        const Language *lang = &g_languages[i];

        /* Try exact filename match first */
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
const Language *detect_language_by_shebang(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "r");
    if (!fp) return NULL;

    /* Read first 256 bytes to find shebang */
    char buffer[256];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[bytes_read] = '\0';
    fclose(fp);

    /* Check if file starts with #! */
    if (bytes_read < 2 || buffer[0] != '#' || buffer[1] != '!') {
        return NULL;
    }

    /* Parse shebang line */
    char *shebang_end = strchr(buffer, '\n');
    if (shebang_end) *shebang_end = '\0';

    /* Extract interpreter path (skip #! and whitespace) */
    char *interpreter = buffer + 2;
    while (isspace((unsigned char)*interpreter)) interpreter++;

    /* Get just the interpreter name (without path) */
    const char *interpreter_name = strrchr(interpreter, '/');
    if (interpreter_name) {
        interpreter_name++;
    } else {
        interpreter_name = interpreter;
    }

    /* Match against language shebang patterns */
    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        const Language *lang = &g_languages[i];

        if (!lang->shebangs) continue;

        char *shebang_copy = strdup(lang->shebangs);
        char *saveptr = NULL;
        char *pattern = strtok_r(shebang_copy, ",", &saveptr);
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

/* Get language by name (linear search) */
const Language *get_language_by_name(const char *name) {
    if (!name) return NULL;

    for (size_t i = 0; i < NUM_LANGUAGES; i++) {
        if (strcasecmp(g_languages[i].name, name) == 0) {
            return &g_languages[i];
        }
    }

    return NULL;
}
