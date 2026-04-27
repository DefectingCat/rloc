#include "lang_defs.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Custom language registry */
CustomLanguage g_custom_languages[MAX_CUSTOM_LANGUAGES];
int g_custom_language_count = 0;

/* Generic filter definitions */
static const GenericFilter python_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter javascript_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter c_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter cpp_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter java_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter go_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter rust_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter typescript_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter ruby_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter php_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_INLINE, "#", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter csharp_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter swift_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter kotlin_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter shell_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter perl_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter css_filters[] = {
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter html_filters[] = {
    {FILTER_REMOVE_BETWEEN, "<!--", "-->"},
};

static const GenericFilter sql_filters[] = {
    {FILTER_REMOVE_INLINE, "--", NULL},
};

static const GenericFilter toml_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter xml_filters[] = {
    {FILTER_REMOVE_BETWEEN, "<!--", "-->"},
};

static const GenericFilter yaml_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

/* Language definitions array */
const Language g_languages[NUM_LANGUAGES] = {{.name = "Python",
                                              .extensions = "py,pyw",
                                              .filenames = NULL,
                                              .shebangs = "python,python3",
                                              .generic_filter_count = 1,
                                              .generic_filters = python_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "JavaScript",
                                              .extensions = "js,mjs",
                                              .filenames = NULL,
                                              .shebangs = "node",
                                              .generic_filter_count = 1,
                                              .generic_filters = javascript_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "C",
                                              .extensions = "c,h",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 2,
                                              .generic_filters = c_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "C++",
                                              .extensions = "cpp,cxx,cc,hpp,hxx,hh",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 2,
                                              .generic_filters = cpp_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Java",
                                              .extensions = "java",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 2,
                                              .generic_filters = java_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Go",
                                              .extensions = "go",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = go_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Rust",
                                              .extensions = "rs",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 2,
                                              .generic_filters = rust_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "TypeScript",
                                              .extensions = "ts,tsx",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 2,
                                              .generic_filters = typescript_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Ruby",
                                              .extensions = "rb",
                                              .filenames = NULL,
                                              .shebangs = "ruby",
                                              .generic_filter_count = 1,
                                              .generic_filters = ruby_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"",
                                              .str_escape = "\\"},
                                             {.name = "PHP",
                                              .extensions = "php",
                                              .filenames = NULL,
                                              .shebangs = "php",
                                              .generic_filter_count = 3,
                                              .generic_filters = php_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "C#",
                                              .extensions = "cs",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 2,
                                              .generic_filters = csharp_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Swift",
                                              .extensions = "swift",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = swift_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Kotlin",
                                              .extensions = "kt,kts",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = kotlin_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Shell",
                                              .extensions = "sh,bash,zsh,ksh",
                                              .filenames = "Makefile,Dockerfile",
                                              .shebangs = "bash,sh,zsh,ksh",
                                              .generic_filter_count = 1,
                                              .generic_filters = shell_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "Config",
                                              .extensions = NULL,
                                              .filenames = ".gitignore,.clang-format,.dockerignore,.editorconfig,.gitattributes,.gitmodules,.mailmap",
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = shell_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = NULL,
                                              .str_escape = NULL},
                                             {.name = "Perl",
                                              .extensions = "pl,pm",
                                              .filenames = NULL,
                                              .shebangs = "perl",
                                              .generic_filter_count = 1,
                                              .generic_filters = perl_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"",
                                              .str_escape = "\\"},
                                             {.name = "CSS",
                                              .extensions = "css",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = css_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "HTML",
                                              .extensions = "html,htm",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = html_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "JSON",
                                              .extensions = "json",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 0,
                                              .generic_filters = NULL,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"",
                                              .str_escape = "\\"},
                                             {.name = "Markdown",
                                              .extensions = "md,markdown",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 0,
                                              .generic_filters = NULL,
                                              .comment_hook = NULL,
                                              .str_delimiters = NULL,
                                              .str_escape = NULL},
                                             {.name = "SQL",
                                              .extensions = "sql",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = sql_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "TOML",
                                              .extensions = "toml",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = toml_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"",
                                              .str_escape = "\\"},
                                             {.name = "Vue",
                                              .extensions = "vue",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = javascript_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "XML",
                                              .extensions = "xml",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = xml_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"'",
                                              .str_escape = "\\"},
                                             {.name = "YAML",
                                              .extensions = "yaml,yml",
                                              .filenames = NULL,
                                              .shebangs = NULL,
                                              .generic_filter_count = 1,
                                              .generic_filters = yaml_filters,
                                              .comment_hook = NULL,
                                              .str_delimiters = "\"",
                                              .str_escape = "\\"}};

/* === Custom Language Definition Implementation === */

/* Custom filter structure (stored in CustomLanguage) */
typedef struct {
    FilterType type;
    char pattern_open[64];
    char pattern_close[64];
} CustomFilter;

/* Trim whitespace from string */
static char* trim(char* str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str;
}

/* Parse a single language definition block */
static int parse_lang_block(FILE* fp, CustomLanguage* lang) {
    char line[256];
    int in_block = 0;
    CustomFilter custom_filters[MAX_CUSTOM_FILTERS];
    int custom_filter_count = 0;

    memset(lang, 0, sizeof(CustomLanguage));

    while (fgets(line, sizeof(line), fp)) {
        char* trimmed = trim(line);

        // Empty line or comment
        if (*trimmed == '\0' || *trimmed == '#') continue;

        // First non-empty line is the language name
        if (!in_block) {
            strncpy(lang->name, trimmed, sizeof(lang->name) - 1);
            in_block = 1;
            continue;
        }

        // Indented line = property
        if (line[0] == ' ' || line[0] == '\t') {
            trimmed = trim(line);

            // filter remove_inline
            if (strncmp(trimmed, "filter remove_inline ", 21) == 0) {
                if (custom_filter_count < MAX_CUSTOM_FILTERS) {
                    custom_filters[custom_filter_count].type = FILTER_REMOVE_INLINE;
                    strncpy(custom_filters[custom_filter_count].pattern_open, trim(trimmed + 21),
                            sizeof(custom_filters[custom_filter_count].pattern_open) - 1);
                    custom_filter_count++;
                }
            }
            // filter remove_between
            else if (strncmp(trimmed, "filter remove_between ", 22) == 0) {
                if (custom_filter_count < MAX_CUSTOM_FILTERS) {
                    custom_filters[custom_filter_count].type = FILTER_REMOVE_BETWEEN;
                    char* rest = trim(trimmed + 22);
                    char* space = strchr(rest, ' ');
                    if (space) {
                        *space = '\0';
                        strncpy(custom_filters[custom_filter_count].pattern_open, rest,
                                sizeof(custom_filters[custom_filter_count].pattern_open) - 1);
                        strncpy(custom_filters[custom_filter_count].pattern_close, trim(space + 1),
                                sizeof(custom_filters[custom_filter_count].pattern_close) - 1);
                        custom_filter_count++;
                    }
                }
            }
            // extension
            else if (strncmp(trimmed, "extension ", 10) == 0) {
                strncpy(lang->extensions, trim(trimmed + 10), sizeof(lang->extensions) - 1);
            }
            // filename
            else if (strncmp(trimmed, "filename ", 9) == 0) {
                strncpy(lang->filenames, trim(trimmed + 9), sizeof(lang->filenames) - 1);
            }
            // shebang
            else if (strncmp(trimmed, "shebang ", 8) == 0) {
                strncpy(lang->shebangs, trim(trimmed + 8), sizeof(lang->shebangs) - 1);
            }
            // string_delimiters
            else if (strncmp(trimmed, "string_delimiters ", 18) == 0) {
                strncpy(lang->str_delimiters, trim(trimmed + 18), sizeof(lang->str_delimiters) - 1);
            }
            // string_escape
            else if (strncmp(trimmed, "string_escape ", 14) == 0) {
                strncpy(lang->str_escape, trim(trimmed + 14), sizeof(lang->str_escape) - 1);
            }
        } else {
            // Non-indented line after block = new language or end
            break;
        }
    }

    // Copy custom filters to language's GenericFilter array
    lang->filter_count = custom_filter_count;
    for (int i = 0; i < custom_filter_count; i++) {
        lang->filters[i].type = custom_filters[i].type;
        // Note: pattern_open/pattern_close are const char*, we cast from local buffer
        lang->filters[i].pattern_open = lang->filter_patterns_open[i];
        lang->filters[i].pattern_close = lang->filter_patterns_close[i];
        strcpy(lang->filter_patterns_open[i], custom_filters[i].pattern_open);
        if (custom_filters[i].type == FILTER_REMOVE_BETWEEN) {
            strcpy(lang->filter_patterns_close[i], custom_filters[i].pattern_close);
        }
    }

    return in_block;
}

int lang_defs_load_file(const char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return -1;

    int loaded = 0;
    CustomLanguage lang;

    while (!feof(fp)) {
        if (parse_lang_block(fp, &lang)) {
            if (strlen(lang.name) > 0 && lang_defs_register_custom(&lang) == 0) {
                loaded++;
            }
        }
    }

    fclose(fp);
    return loaded;
}

int lang_defs_register_custom(const CustomLanguage* lang) {
    if (!lang || g_custom_language_count >= MAX_CUSTOM_LANGUAGES) return -1;

    memcpy(&g_custom_languages[g_custom_language_count], lang, sizeof(CustomLanguage));
    g_custom_language_count++;
    return 0;
}

const Language* lang_defs_find_by_name(const char* name) {
    if (!name) return NULL;

    // Check custom languages first
    for (int i = 0; i < g_custom_language_count; i++) {
        if (strcmp(g_custom_languages[i].name, name) == 0) {
            // Return pointer to custom language (cast to Language*)
            // Note: This is safe because CustomLanguage has compatible layout
            return (const Language*)&g_custom_languages[i];
        }
    }

    // Check built-in languages
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        if (strcmp(g_languages[i].name, name) == 0) {
            return &g_languages[i];
        }
    }

    return NULL;
}

void lang_defs_clear_custom(void) {
    g_custom_language_count = 0;
    memset(g_custom_languages, 0, sizeof(g_custom_languages));
}

/* Print language info: name, extensions, filters */
int lang_show(const char* name) {
    if (name) {
        const Language* lang = lang_defs_find_by_name(name);
        if (!lang) {
            fprintf(stderr, "Error: Unknown language '%s'\n", name);
            return -1;
        }
        printf("%s\n", lang->name);
        printf("  extensions: %s\n", lang->extensions ? lang->extensions : "(none)");
        if (lang->filenames) printf("  filenames:  %s\n", lang->filenames);
        if (lang->shebangs)  printf("  shebangs:   %s\n", lang->shebangs);
        printf("  filters:    %zu\n", lang->generic_filter_count);
        for (size_t i = 0; i < lang->generic_filter_count; i++) {
            if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE) {
                printf("    inline: %s\n", lang->generic_filters[i].pattern_open);
            } else if (lang->generic_filters[i].type == FILTER_REMOVE_BETWEEN) {
                printf("    between: %s ... %s\n", lang->generic_filters[i].pattern_open,
                       lang->generic_filters[i].pattern_close);
            } else {
                printf("    remove: %s\n", lang->generic_filters[i].pattern_open);
            }
        }
        return 0;
    }
    printf("Supported languages:\n");
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        printf("  %-12s  %s\n", g_languages[i].name, g_languages[i].extensions);
    }
    if (g_custom_language_count > 0) {
        printf("Custom languages:\n");
        for (int i = 0; i < g_custom_language_count; i++) {
            printf("  %-12s  %s\n", g_custom_languages[i].name,
                   g_custom_languages[i].extensions);
        }
    }
    return 0;
}

/* Print extension to language mapping */
int ext_show(const char* ext) {
    if (ext) {
        if (ext[0] == '.') ext++;
        int found = 0;
        for (int i = 0; i < NUM_LANGUAGES; i++) {
            if (!g_languages[i].extensions) continue;
            char buf[256];
            strncpy(buf, g_languages[i].extensions, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            char* token = strtok(buf, ",");
            while (token) {
                if (strcasecmp(token, ext) == 0) {
                    printf("%s -> %s\n", ext, g_languages[i].name);
                    found = 1;
                    break;
                }
                token = strtok(NULL, ",");
            }
            if (found) break;
        }
        if (!found) {
            for (int i = 0; i < g_custom_language_count; i++) {
                if (g_custom_languages[i].extensions[0] == '\0') continue;
                char buf[256];
                strncpy(buf, g_custom_languages[i].extensions, sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                char* token = strtok(buf, ",");
                while (token) {
                    if (strcasecmp(token, ext) == 0) {
                        printf("%s -> %s (custom)\n", ext, g_custom_languages[i].name);
                        found = 1;
                        break;
                    }
                    token = strtok(NULL, ",");
                }
                if (found) break;
            }
        }
        if (!found) {
            fprintf(stderr, "Error: Unknown extension '%s'\n", ext);
            return -1;
        }
        return 0;
    }
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        if (!g_languages[i].extensions) continue;
        char buf[256];
        strncpy(buf, g_languages[i].extensions, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        char* token = strtok(buf, ",");
        while (token) {
            printf("  %-8s -> %s\n", token, g_languages[i].name);
            token = strtok(NULL, ",");
        }
    }
    if (g_custom_language_count > 0) {
        for (int i = 0; i < g_custom_language_count; i++) {
            if (g_custom_languages[i].extensions[0] == '\0') continue;
            char buf[256];
            strncpy(buf, g_custom_languages[i].extensions, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            char* token = strtok(buf, ",");
            while (token) {
                printf("  %-8s -> %s (custom)\n", token, g_custom_languages[i].name);
                token = strtok(NULL, ",");
            }
        }
    }
    return 0;
}
