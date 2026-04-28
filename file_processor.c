#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config.h"
#include "file_processor.h"
#include "language.h"
#include "unique.h"
#include "util.h"

const char* fp_get_extension(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext && ext != filepath) return ext + 1;
    return NULL;
}

int fp_is_in_string_array(const char* str, char** array, int n) {
    for (int i = 0; i < n; i++) {
        if (strcasecmp(str, array[i]) == 0) return 1;
    }
    return 0;
}

int fp_extension_matches(const char* ext, char** array, int n) {
    if (!ext) return 0;
    for (int i = 0; i < n; i++) {
        const char* p = array[i];
        if (p[0] == '.') p++;
        if (strcasecmp(ext, p) == 0) return 1;
    }
    return 0;
}

int fp_context_init(FileProcessorContext* ctx, const CliArgs* args, FileList* filelist,
                    FileStats* files, int n_files) {
    ctx->args = args;
    ctx->filelist = filelist;
    ctx->files = files;
    ctx->n_files = n_files;
    ctx->force_lang_all = NULL;
    ctx->force_lang_ext = NULL;
    ctx->force_lang_for_ext = NULL;

    /* Parse --force-lang option if present */
    if (args->force_lang != NULL) {
        char* comma = strchr(args->force_lang, ',');
        if (comma) {
            char* lang_name = strndup(args->force_lang, comma - args->force_lang);
            ctx->force_lang_for_ext = get_language_by_name(lang_name);
            free(lang_name);
            ctx->force_lang_ext = strdup(comma + 1);
        } else {
            ctx->force_lang_all = get_language_by_name(args->force_lang);
        }
    }
    return 0;
}

void fp_context_free(FileProcessorContext* ctx) {
    if (ctx->force_lang_ext) {
        free(ctx->force_lang_ext);
        ctx->force_lang_ext = NULL;
    }
}

void fp_check_uniqueness(FileStats* files, int n_files, int skip_uniqueness) {
    if (skip_uniqueness || n_files == 0) return;

    UniqueTable unique_tbl;
    int table_cap = n_files < UNIQUE_TABLE_MAX ? n_files : UNIQUE_TABLE_MAX;
    if (unique_table_init(&unique_tbl, table_cap) == 0) {
        for (int i = 0; i < n_files; i++) {
            uint8_t md5[16];
            if (compute_file_md5(files[i].filepath, md5) == 0) {
                if (unique_table_insert(&unique_tbl, md5) != 0) {
                    files[i].ignore_reason = "duplicate";
                }
            }
        }
        unique_table_free(&unique_tbl);
    }
}

void fp_detect_languages_and_filter(FileProcessorContext* ctx) {
    const CliArgs* args = ctx->args;
    FileStats* files = ctx->files;
    int n_files = ctx->n_files;

    for (int i = 0; i < n_files; i++) {
        if (files[i].ignore_reason != NULL) continue;

        const char* filepath = files[i].filepath;
        const Language* lang = NULL;

        /* Apply --force-lang override */
        if (ctx->force_lang_all != NULL) {
            lang = ctx->force_lang_all;
        } else if (ctx->force_lang_ext != NULL && ctx->force_lang_for_ext != NULL) {
            const char* ext = fp_get_extension(filepath);
            if (ext && strcasecmp(ext, ctx->force_lang_ext) == 0) {
                lang = ctx->force_lang_for_ext;
            } else {
                lang = detect_language(filepath);
            }
        } else {
            lang = detect_language(filepath);
        }

        /* Apply --lang-no-ext for files without extension */
        if (lang == NULL && args->lang_no_ext) {
            const char* ext = fp_get_extension(filepath);
            if (ext == NULL) {
                lang = get_language_by_name(args->lang_no_ext);
            }
        }

        /* Apply --script-lang for shebang language override */
        if (lang == NULL && args->n_script_lang > 0) {
            FILE* fp = fopen(filepath, "r");
            if (fp) {
                char line[256];
                if (fgets(line, sizeof(line), fp)) {
                    fclose(fp);
                    if (line[0] == '#' && line[1] == '!') {
                        char* interp = line + 2;
                        while (*interp && isspace((unsigned char)*interp)) interp++;
                        char* interp_end = interp;
                        while (*interp_end && !isspace((unsigned char)*interp_end)) interp_end++;
                        *interp_end = '\0';
                        const char* interp_name = strrchr(interp, '/');
                        if (interp_name)
                            interp_name++;
                        else
                            interp_name = interp;
                        for (int j = 0; j < args->n_script_lang; j++) {
                            if (strcmp(interp_name, args->script_names[j]) == 0 ||
                                strstr(interp_name, args->script_names[j]) == interp_name) {
                                lang = get_language_by_name(args->script_langs[j]);
                                break;
                            }
                        }
                    }
                } else {
                    fclose(fp);
                }
            }
        }

        /* Apply --include-lang filter */
        if (args->n_include_langs > 0) {
            if (lang == NULL ||
                !fp_is_in_string_array(lang->name, args->include_langs, args->n_include_langs)) {
                files[i].ignore_reason = "include-lang filter";
                continue;
            }
        }

        /* Apply --exclude-lang filter */
        if (lang != NULL && args->n_exclude_langs > 0) {
            if (fp_is_in_string_array(lang->name, args->exclude_langs, args->n_exclude_langs)) {
                files[i].ignore_reason = "exclude-lang filter";
                continue;
            }
        }

        /* Apply --include-ext filter */
        if (args->n_include_exts > 0) {
            const char* ext = fp_get_extension(filepath);
            if (!fp_extension_matches(ext, args->include_exts, args->n_include_exts)) {
                files[i].ignore_reason = "include-ext filter";
                continue;
            }
        }

        /* Apply --exclude-ext filter */
        if (args->n_exclude_exts > 0) {
            const char* ext = fp_get_extension(filepath);
            if (fp_extension_matches(ext, args->exclude_exts, args->n_exclude_exts)) {
                files[i].ignore_reason = "exclude-ext filter";
                continue;
            }
        }

        files[i].lang = lang;
        if (lang == NULL && !args->quiet) {
            fprintf(stderr, "Warning: Unrecognized language for file '%s'\n", filepath);
        }
    }
}

void fp_filter_by_content(FileStats* files, int n_files, const char* include_content,
                          const char* exclude_content) {
    if (!include_content && !exclude_content) return;

    regex_t include_regex, exclude_regex;
    int has_include = 0, has_exclude = 0;

    if (include_content) {
        if (regcomp(&include_regex, include_content, REG_EXTENDED | REG_NOSUB) == 0) {
            has_include = 1;
        }
    }
    if (exclude_content) {
        if (regcomp(&exclude_regex, exclude_content, REG_EXTENDED | REG_NOSUB) == 0) {
            has_exclude = 1;
        }
    }

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;

        FILE* fp = fopen(files[i].filepath, "r");
        if (!fp) continue;

        int matched_include = 0, matched_exclude = 0;
        char line[4096];
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (has_include && regexec(&include_regex, line, 0, NULL, 0) == 0) {
                matched_include = 1;
            }
            if (has_exclude && regexec(&exclude_regex, line, 0, NULL, 0) == 0) {
                matched_exclude = 1;
            }
            if (matched_include && matched_exclude) break;
        }
        fclose(fp);

        if (has_include && !matched_include) {
            files[i].ignore_reason = "include-content filter";
            files[i].lang = NULL;
        }
        if (has_exclude && matched_exclude) {
            files[i].ignore_reason = "exclude-content filter";
            files[i].lang = NULL;
        }
    }

    if (has_include) regfree(&include_regex);
    if (has_exclude) regfree(&exclude_regex);
}

void fp_filter_by_timeout(FileStats* files, int n_files, int timeout_sec) {
    if (timeout_sec <= 0) return;

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;
        long size = get_file_size(files[i].filepath);
        if (size > timeout_sec * 1024 * 1024) {
            files[i].ignore_reason = "timeout estimate";
            files[i].lang = NULL;
        }
    }
}
