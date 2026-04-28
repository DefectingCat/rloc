#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include "cli.h"
#include "filelist.h"
#include "language.h"
#include "output.h"
#include "unique.h"

/* Get file extension (without dot). Returns NULL if no extension. */
const char* fp_get_extension(const char* filepath);

/* Check if string is in array (case-insensitive). */
int fp_is_in_string_array(const char* str, char** array, int n);

/* Check if extension matches any in array (handles leading dots). */
int fp_extension_matches(const char* ext, char** array, int n);

/* Context for file processing */
typedef struct {
    const CliArgs* args;
    FileList* filelist;
    FileStats* files;
    int n_files;
    const Language* force_lang_all;
    char* force_lang_ext;
    const Language* force_lang_for_ext;
} FileProcessorContext;

/* Initialize file processor context. Parses --force-lang option. */
int fp_context_init(FileProcessorContext* ctx, const CliArgs* args, FileList* filelist,
                    FileStats* files, int n_files);

/* Free resources in file processor context. */
void fp_context_free(FileProcessorContext* ctx);

/* Pass 1: Check file uniqueness via MD5.
 * Marks duplicates in files[].ignore_reason. */
void fp_check_uniqueness(FileStats* files, int n_files, int skip_uniqueness);

/* Pass 2: Detect languages and apply filters.
 * Applies --force-lang, --include-lang, --exclude-lang, --include-ext, --exclude-ext,
 * --lang-no-ext, --script-lang filters.
 * Sets files[].lang and files[].ignore_reason. */
void fp_detect_languages_and_filter(FileProcessorContext* ctx);

/* Apply content filtering (--include-content/--exclude-content).
 * Uses regex to filter files by content. */
void fp_filter_by_content(FileStats* files, int n_files, const char* include_content,
                          const char* exclude_content);

/* Apply timeout filtering based on file size estimate.
 * Marks files exceeding timeout limit. */
void fp_filter_by_timeout(FileStats* files, int n_files, int timeout_sec);

#endif /* FILE_PROCESSOR_H */
