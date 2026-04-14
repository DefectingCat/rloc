#include "output.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LANGUAGES 50

typedef struct {
    const char* name;
    int files;
    int blank;
    int comment;
    int code;
} LanguageStats;

static int compare_language_names(const void* a, const void* b) {
    const LanguageStats* la = (const LanguageStats*)a;
    const LanguageStats* lb = (const LanguageStats*)b;
    return strcmp(la->name, lb->name);
}

void output_text(const FileStats* files, int n_files, double elapsed_sec) {
    // Count unique files (all files are unique in M1)
    int n_unique_files = n_files;
    int n_ignored_files = 0;

    // Aggregate statistics by language
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;

    // First pass: collect all unique languages
    for (int i = 0; i < n_files; i++) {
        const char* lang_name = (files[i].lang != NULL) ? files[i].lang->name : "Unknown";
        int found = 0;

        // Check if we already have this language
        for (int j = 0; j < n_languages; j++) {
            if (strcmp(lang_stats[j].name, lang_name) == 0) {
                found = 1;
                break;
            }
        }

        // Add new language if not found
        if (!found && n_languages < MAX_LANGUAGES) {
            lang_stats[n_languages].name = lang_name;
            lang_stats[n_languages].files = 0;
            lang_stats[n_languages].blank = 0;
            lang_stats[n_languages].comment = 0;
            lang_stats[n_languages].code = 0;
            n_languages++;
        }
    }

    // Second pass: aggregate counts
    for (int i = 0; i < n_files; i++) {
        const char* lang_name = (files[i].lang != NULL) ? files[i].lang->name : "Unknown";

        for (int j = 0; j < n_languages; j++) {
            if (strcmp(lang_stats[j].name, lang_name) == 0) {
                lang_stats[j].files++;
                lang_stats[j].blank += files[i].counts.blank;
                lang_stats[j].comment += files[i].counts.comment;
                lang_stats[j].code += files[i].counts.code;
                break;
            }
        }
    }

    // Calculate totals
    int total_files = 0;
    int total_blank = 0;
    int total_comment = 0;
    int total_code = 0;

    for (int i = 0; i < n_languages; i++) {
        total_files += lang_stats[i].files;
        total_blank += lang_stats[i].blank;
        total_comment += lang_stats[i].comment;
        total_code += lang_stats[i].code;
    }

    // Calculate files/s and lines/s
    double files_per_sec = (elapsed_sec > 0) ? (n_files / elapsed_sec) : 0.0;
    double lines_per_sec =
        (elapsed_sec > 0) ? ((total_blank + total_comment + total_code) / elapsed_sec) : 0.0;

    // Print header
    printf("%5d text file%s.\n", n_files, (n_files != 1) ? "s" : "");
    printf("%5d unique file%s.\n", n_unique_files, (n_unique_files != 1) ? "s" : "");
    printf("%5d files ignored.\n", n_ignored_files);
    printf("\n");

    // Print URL line
    printf(
        "https://github.com/rloc/rloc v 0.1.0  T=%.2f s (%.1f files/s, %.1f "
        "lines/s)\n",
        elapsed_sec, files_per_sec, lines_per_sec);

    // Print separator line
    printf(
        "---------------------------------------------------------------------"
        "----------\n");

    // Print column headers
    printf("%-15s %10s %10s %10s %10s\n", "Language", "files", "blank", "comment", "code");

    // Print separator line
    printf(
        "---------------------------------------------------------------------"
        "----------\n");

    // Sort languages alphabetically
    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    // Print each language
    for (int i = 0; i < n_languages; i++) {
        printf("%-15s %10d %10d %10d %10d\n", lang_stats[i].name, lang_stats[i].files,
               lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    // Print separator line
    printf(
        "---------------------------------------------------------------------"
        "----------\n");

    // Print SUM row
    printf("%-15s %10d %10d %10d %10d\n", "SUM", total_files, total_blank, total_comment,
           total_code);

    // Print separator line
    printf(
        "---------------------------------------------------------------------"
        "----------\n");
    printf("\n");
}