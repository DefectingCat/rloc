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

/* Helper: Aggregate stats by language */
static void aggregate_by_language(const FileStats* files, int n_files,
                                   LanguageStats* lang_stats, int* n_languages) {
    *n_languages = 0;

    // First pass: collect all unique languages
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;  // Skip filtered/unknown files

        const char* lang_name = files[i].lang->name;
        int found = 0;

        for (int j = 0; j < *n_languages; j++) {
            if (strcmp(lang_stats[j].name, lang_name) == 0) {
                found = 1;
                break;
            }
        }

        if (!found && *n_languages < MAX_LANGUAGES) {
            lang_stats[*n_languages].name = lang_name;
            lang_stats[*n_languages].files = 0;
            lang_stats[*n_languages].blank = 0;
            lang_stats[*n_languages].comment = 0;
            lang_stats[*n_languages].code = 0;
            (*n_languages)++;
        }
    }

    // Second pass: aggregate counts
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;

        const char* lang_name = files[i].lang->name;
        for (int j = 0; j < *n_languages; j++) {
            if (strcmp(lang_stats[j].name, lang_name) == 0) {
                lang_stats[j].files++;
                lang_stats[j].blank += files[i].counts.blank;
                lang_stats[j].comment += files[i].counts.comment;
                lang_stats[j].code += files[i].counts.code;
                break;
            }
        }
    }
}

/* Helper: Calculate totals */
static void calculate_totals(const LanguageStats* lang_stats, int n_languages,
                              int* total_files, int* total_blank,
                              int* total_comment, int* total_code) {
    *total_files = 0;
    *total_blank = 0;
    *total_comment = 0;
    *total_code = 0;

    for (int i = 0; i < n_languages; i++) {
        *total_files += lang_stats[i].files;
        *total_blank += lang_stats[i].blank;
        *total_comment += lang_stats[i].comment;
        *total_code += lang_stats[i].code;
    }
}

/* Helper: Count valid files (with language) */
static int count_valid_files(const FileStats* files, int n_files) {
    int count = 0;
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang != NULL) count++;
    }
    return count;
}

void output_text(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank,
                     &total_comment, &total_code);

    // Count files including unknown
    int n_total = n_files;
    int n_valid = count_valid_files(files, n_files);

    // Calculate files/s and lines/s
    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    double lines_per_sec =
        (elapsed_sec > 0) ? ((total_blank + total_comment + total_code) / elapsed_sec) : 0.0;

    // Print header
    printf("%5d text file%s.\n", n_total, (n_total != 1) ? "s" : "");
    printf("%5d unique file%s.\n", n_valid, (n_valid != 1) ? "s" : "");
    printf("%5d files ignored.\n", n_total - n_valid);
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

/* Output per-file text format */
void output_text_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    int n_valid = count_valid_files(files, n_files);

    printf("%5d text file%s.\n", n_files, (n_files != 1) ? "s" : "");
    printf("%5d unique file%s.\n", n_valid, (n_valid != 1) ? "s" : "");
    printf("\n");

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    printf("https://github.com/rloc/rloc v 0.1.0  T=%.2f s (%.1f files/s)\n",
           elapsed_sec, files_per_sec);
    printf("\n");

    printf("%-40s %10s %10s %10s %10s\n", "File", "blank", "comment", "code", "language");
    printf("-----------------------------------------------------------------------------\n");

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        const char* filename = files[i].filepath;
        // Truncate long filenames
        if (strlen(filename) > 38) {
            printf("...%-37s %10d %10d %10d %s\n",
                   filename + strlen(filename) - 35,
                   files[i].counts.blank, files[i].counts.comment,
                   files[i].counts.code, files[i].lang->name);
        } else {
            printf("%-40s %10d %10d %10d %s\n",
                   filename, files[i].counts.blank, files[i].counts.comment,
                   files[i].counts.code, files[i].lang->name);
        }
    }
    printf("\n");
}

/* Output JSON format (cloc-like structure) */
void output_json(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank,
                     &total_comment, &total_code);

    int n_valid = count_valid_files(files, n_files);
    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    double lines_per_sec = (elapsed_sec > 0) ?
        ((total_blank + total_comment + total_code) / elapsed_sec) : 0.0;

    printf("{\n");
    printf("  \"header\" : {\n");
    printf("    \"cloc_url\"           : \"github.com/rloc/rloc\",\n");
    printf("    \"cloc_version\"       : \"0.1.0\",\n");
    printf("    \"elapsed_seconds\"    : %.3f,\n", elapsed_sec);
    printf("    \"n_files\"            : %d,\n", n_valid);
    printf("    \"n_lines\"            : %d,\n", total_blank + total_comment + total_code);
    printf("    \"files_per_second\"   : %.1f,\n", files_per_sec);
    printf("    \"lines_per_second\"   : %.1f\n", lines_per_sec);
    printf("  },\n");

    // Sort languages alphabetically
    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    for (int i = 0; i < n_languages; i++) {
        printf("  \"%s\" : {\n", lang_stats[i].name);
        printf("    \"nFiles\" : %d,\n", lang_stats[i].files);
        printf("    \"blank\"  : %d,\n", lang_stats[i].blank);
        printf("    \"comment\": %d,\n", lang_stats[i].comment);
        printf("    \"code\"   : %d\n", lang_stats[i].code);
        if (i < n_languages - 1) {
            printf("  },\n");
        } else {
            printf("  }\n");
        }
    }

    printf("}\n");
}

/* Output JSON with per-file breakdown */
void output_json_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    int n_valid = count_valid_files(files, n_files);

    int total_blank = 0, total_comment = 0, total_code = 0;
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang != NULL) {
            total_blank += files[i].counts.blank;
            total_comment += files[i].counts.comment;
            total_code += files[i].counts.code;
        }
    }

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;

    printf("{\n");
    printf("  \"header\" : {\n");
    printf("    \"cloc_url\"           : \"github.com/rloc/rloc\",\n");
    printf("    \"cloc_version\"       : \"0.1.0\",\n");
    printf("    \"elapsed_seconds\"    : %.3f,\n", elapsed_sec);
    printf("    \"n_files\"            : %d,\n", n_valid);
    printf("    \"files_per_second\"   : %.1f\n", files_per_sec);
    printf("  },\n");

    int first = 1;
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;

        if (!first) printf(",\n");
        first = 0;

        printf("  \"%s\" : {\n", files[i].filepath);
        printf("    \"blank\"   : %d,\n", files[i].counts.blank);
        printf("    \"comment\" : %d,\n", files[i].counts.comment);
        printf("    \"code\"    : %d,\n", files[i].counts.code);
        printf("    \"language\": \"%s\"\n", files[i].lang->name);
        printf("  }");
    }
    printf("\n}\n");
}

/* Output CSV format */
void output_csv(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    printf("files,language,blank,comment,code\n");
    for (int i = 0; i < n_languages; i++) {
        printf("%d,%s,%d,%d,%d\n",
               lang_stats[i].files, lang_stats[i].name,
               lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank,
                     &total_comment, &total_code);
    printf("%d,SUM,%d,%d,%d\n", total_files, total_blank, total_comment, total_code);
}

/* Output CSV with per-file breakdown */
void output_csv_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    printf("language,filename,blank,comment,code\n");
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("%s,%s,%d,%d,%d\n",
               files[i].lang->name, files[i].filepath,
               files[i].counts.blank, files[i].counts.comment, files[i].counts.code);
    }
}

/* Output Markdown format */
void output_md(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    int n_valid = count_valid_files(files, n_files);

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    printf("cloc|github.com/rloc/rloc v 0.1.0  T=%.2f s (%.1f files/s)\n", elapsed_sec, files_per_sec);
    printf("--- | ---\n\n");

    printf("| Language | files | blank | comment | code |\n");
    printf("|:-------|:-------:|:-------:|:-------:|:-------:|\n");

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    for (int i = 0; i < n_languages; i++) {
        printf("| %s | %d | %d | %d | %d |\n",
               lang_stats[i].name, lang_stats[i].files,
               lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank,
                     &total_comment, &total_code);
    printf("| SUM: | %d | %d | %d | %d |\n", total_files, total_blank, total_comment, total_code);
}

/* Output Markdown with per-file breakdown */
void output_md_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    int n_valid = count_valid_files(files, n_files);

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    printf("cloc|github.com/rloc/rloc v 0.1.0  T=%.2f s (%.1f files/s)\n", elapsed_sec, files_per_sec);
    printf("--- | ---\n\n");

    printf("| File | blank | comment | code | language |\n");
    printf("|:-------|:-------:|:-------:|:-------:|:-------|\n");

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("| %s | %d | %d | %d | %s |\n",
               files[i].filepath, files[i].counts.blank,
               files[i].counts.comment, files[i].counts.code, files[i].lang->name);
    }
}