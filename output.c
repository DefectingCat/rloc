#include "output.h"
#include "version.h"

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
static void aggregate_by_language(const FileStats* files, int n_files, LanguageStats* lang_stats,
                                  int* n_languages) {
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
static void calculate_totals(const LanguageStats* lang_stats, int n_languages, int* total_files,
                             int* total_blank, int* total_comment, int* total_code) {
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

/* Helper: Count duplicate files (ignore_reason == "duplicate") */
static int count_duplicates(const FileStats* files, int n_files) {
    int count = 0;
    for (int i = 0; i < n_files; i++) {
        if (files[i].ignore_reason && strcmp(files[i].ignore_reason, "duplicate") == 0) count++;
    }
    return count;
}

void output_text(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);

    int n_total = n_files;
    int n_valid = count_valid_files(files, n_files);
    int n_duplicates = count_duplicates(files, n_files);

    // Calculate files/s and lines/s
    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    double lines_per_sec =
        (elapsed_sec > 0) ? ((total_blank + total_comment + total_code) / elapsed_sec) : 0.0;

    // Print header
    printf("%5d text file%s.\n", n_total, (n_total != 1) ? "s" : "");
    printf("%5d unique file%s.\n", n_valid, (n_valid != 1) ? "s" : "");
    printf("%5d files ignored.\n", n_total - n_valid);
    if (n_duplicates > 0) {
        printf("%5d duplicate file%s.\n", n_duplicates, (n_duplicates != 1) ? "s" : "");
    }
    printf("\n");

    // Print URL line
    printf(
        "https://github.com/rloc/rloc v %s  T=%.2f s (%.1f files/s, %.1f "
        "lines/s)\n",
        RLOC_VERSION, elapsed_sec, files_per_sec, lines_per_sec);

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
    int n_duplicates = count_duplicates(files, n_files);

    printf("%5d text file%s.\n", n_files, (n_files != 1) ? "s" : "");
    printf("%5d unique file%s.\n", n_valid, (n_valid != 1) ? "s" : "");
    if (n_duplicates > 0) {
        printf("%5d duplicate file%s.\n", n_duplicates, (n_duplicates != 1) ? "s" : "");
    }

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    printf("https://github.com/rloc/rloc v %s  T=%.2f s (%.1f files/s)\n", RLOC_VERSION, elapsed_sec,
           files_per_sec);
    printf("\n");

    printf("%-40s %10s %10s %10s %10s\n", "File", "blank", "comment", "code", "language");
    printf("-----------------------------------------------------------------------------\n");

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        const char* filename = files[i].filepath;
        // Truncate long filenames
        if (strlen(filename) > 38) {
            printf("...%-37s %10d %10d %10d %s\n", filename + strlen(filename) - 35,
                   files[i].counts.blank, files[i].counts.comment, files[i].counts.code,
                   files[i].lang->name);
        } else {
            printf("%-40s %10d %10d %10d %s\n", filename, files[i].counts.blank,
                   files[i].counts.comment, files[i].counts.code, files[i].lang->name);
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
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);

    int n_valid = count_valid_files(files, n_files);
    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    double lines_per_sec =
        (elapsed_sec > 0) ? ((total_blank + total_comment + total_code) / elapsed_sec) : 0.0;

    printf("{\n");
    printf("  \"header\" : {\n");
    printf("    \"cloc_url\"           : \"github.com/rloc/rloc\",\n");
    printf("    \"cloc_version\"       : \"%s\",\n", RLOC_VERSION);
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

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;

    printf("{\n");
    printf("  \"header\" : {\n");
    printf("    \"cloc_url\"           : \"github.com/rloc/rloc\",\n");
    printf("    \"cloc_version\"       : \"%s\",\n", RLOC_VERSION);
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
    (void)elapsed_sec;  // API consistency, not used in CSV output
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    printf("files,language,blank,comment,code\n");
    for (int i = 0; i < n_languages; i++) {
        printf("%d,%s,%d,%d,%d\n", lang_stats[i].files, lang_stats[i].name, lang_stats[i].blank,
               lang_stats[i].comment, lang_stats[i].code);
    }

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);
    printf("%d,SUM,%d,%d,%d\n", total_files, total_blank, total_comment, total_code);
}

/* Output CSV with per-file breakdown */
void output_csv_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    (void)elapsed_sec;  // API consistency, not used in CSV output
    printf("language,filename,blank,comment,code\n");
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("%s,%s,%d,%d,%d\n", files[i].lang->name, files[i].filepath, files[i].counts.blank,
               files[i].counts.comment, files[i].counts.code);
    }
}

/* Output Markdown format */
void output_md(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    int n_valid = count_valid_files(files, n_files);

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    printf("cloc|github.com/rloc/rloc v %s  T=%.2f s (%.1f files/s)\n", RLOC_VERSION, elapsed_sec,
           files_per_sec);
    printf("--- | ---\n\n");

    printf("| Language | files | blank | comment | code |\n");
    printf("|:-------|:-------:|:-------:|:-------:|:-------:|\n");

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    for (int i = 0; i < n_languages; i++) {
        printf("| %s | %d | %d | %d | %d |\n", lang_stats[i].name, lang_stats[i].files,
               lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);
    printf("| SUM: | %d | %d | %d | %d |\n", total_files, total_blank, total_comment, total_code);
}

/* Output Markdown with per-file breakdown */
void output_md_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    int n_valid = count_valid_files(files, n_files);

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    printf("cloc|github.com/rloc/rloc v %s  T=%.2f s (%.1f files/s)\n", RLOC_VERSION, elapsed_sec,
           files_per_sec);
    printf("--- | ---\n\n");

    printf("| File | blank | comment | code | language |\n");
    printf("|:-------|:-------:|:-------:|:-------:|:-------|\n");

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("| %s | %d | %d | %d | %s |\n", files[i].filepath, files[i].counts.blank,
               files[i].counts.comment, files[i].counts.code, files[i].lang->name);
    }
}

/* Output diff results */
void output_diff(const DiffFileStats* files, int n_files, const char* commit1, const char* commit2,
                 int by_file) {
    int total_added = 0, total_removed = 0;
    for (int i = 0; i < n_files; i++) {
        total_added += files[i].added;
        total_removed += files[i].removed;
    }

    printf("Comparing %s..%s\n\n", commit1, commit2);
    printf("%d files changed, %d lines added, %d lines removed\n\n", n_files, total_added,
           total_removed);

    if (by_file) {
        printf("%-40s %10s %10s\n", "File", "Added", "Removed");
        printf("-------------------------------------------------------\n");
        for (int i = 0; i < n_files; i++) {
            printf("%-40s %10d %10d\n", files[i].filepath, files[i].added, files[i].removed);
        }
    }
}

/* Helper: Escape a string for JSON output */
static void json_write_string(FILE* fp, const char* str) {
    fputc('"', fp);
    for (const char* p = str; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", fp); break;
            case '\\': fputs("\\\\", fp); break;
            case '\b': fputs("\\b", fp);  break;
            case '\f': fputs("\\f", fp);  break;
            case '\n': fputs("\\n", fp);  break;
            case '\r': fputs("\\r", fp);  break;
            case '\t': fputs("\\t", fp);  break;
            default:   fputc(*p, fp);     break;
        }
    }
    fputc('"', fp);
}

/* Output file alignment table (cloc --diff-alignment compatible) */
void output_alignment(const AlignmentEntry* entries, int n_entries,
                      const char* ref1, const char* ref2, int json_output) {
    if (json_output) {
        output_alignment_json(entries, n_entries, ref1, ref2);
        return;
    }

    /* Count totals and categories */
    int total_added = 0, total_removed = 0;
    int n_added = 0, n_removed = 0, n_modified = 0, n_identical = 0;

    for (int i = 0; i < n_entries; i++) {
        total_added += entries[i].added;
        total_removed += entries[i].removed;
        switch (entries[i].type) {
            case ALIGN_ADDED:    n_added++;    break;
            case ALIGN_REMOVED:  n_removed++;  break;
            case ALIGN_MODIFIED: n_modified++; break;
            case ALIGN_IDENTICAL: n_identical++; break;
        }
    }

    int n_files_changed = n_added + n_removed + n_modified;

    printf("Comparing %s..%s\n\n", ref1, ref2);
    printf("%d file%s, %d lines added, %d lines removed\n",
           n_files_changed, n_files_changed != 1 ? "s" : "",
           total_added, total_removed);
    if (n_added > 0)   printf("  %d new\n", n_added);
    if (n_removed > 0) printf("  %d removed\n", n_removed);
    if (n_modified > 0) printf("  %d modified\n", n_modified);
    if (n_identical > 0) printf("  %d identical\n", n_identical);
    printf("\n");

    /* Determine max file path width for alignment */
    int max_path_len = 10;  /* minimum width */
    for (int i = 0; i < n_entries; i++) {
        int len = (int)strlen(entries[i].filepath);
        if (len > max_path_len) max_path_len = len;
    }
    if (max_path_len > 50) max_path_len = 50;  /* cap to keep line reasonable */

    int path_width = max_path_len + 2;

    /* Print header */
    printf("%-*s %10s %10s  %s\n", path_width, "File", "Added", "Removed", "Language");
    for (int i = 0; i < path_width + 24; i++) fputc('-', stdout);
    fputc('\n', stdout);

    /* Print entries: added, removed, modified, then identical */
    int has_added_header = 0, has_removed_header = 0, has_modified_header = 0;

    /* Added files */
    for (int i = 0; i < n_entries; i++) {
        if (entries[i].type != ALIGN_ADDED) continue;
        if (!has_added_header) {
            printf("--- new files\n");
            has_added_header = 1;
        }
        const char* lang = entries[i].language ? entries[i].language : "(none)";
        printf("%-*s %10d %10d  %s\n", path_width,
               entries[i].filepath, entries[i].added, entries[i].removed, lang);
    }

    /* Removed files */
    for (int i = 0; i < n_entries; i++) {
        if (entries[i].type != ALIGN_REMOVED) continue;
        if (!has_removed_header) {
            printf("--- removed files\n");
            has_removed_header = 1;
        }
        const char* lang = entries[i].language ? entries[i].language : "(none)";
        printf("%-*s %10d %10d  %s\n", path_width,
               entries[i].filepath, entries[i].added, entries[i].removed, lang);
    }

    /* Modified files */
    for (int i = 0; i < n_entries; i++) {
        if (entries[i].type != ALIGN_MODIFIED) continue;
        if (!has_modified_header) {
            printf("--- modified files\n");
            has_modified_header = 1;
        }
        const char* lang = entries[i].language ? entries[i].language : "(none)";
        printf("%-*s %10d %10d  %s\n", path_width,
               entries[i].filepath, entries[i].added, entries[i].removed, lang);
    }

    /* Identical files (only if --identical was set) */
    if (n_identical > 0) {
        printf("--- identical\n");
        for (int i = 0; i < n_entries; i++) {
            if (entries[i].type != ALIGN_IDENTICAL) continue;
            printf("%-*s %10d %10d  %s\n", path_width,
                   entries[i].filepath, entries[i].added, entries[i].removed,
                   entries[i].language ? entries[i].language : "(identical)");
        }
    }

    for (int i = 0; i < path_width + 24; i++) fputc('-', stdout);
    fputc('\n', stdout);
    printf("SUM: %d files, %d lines added, %d lines removed\n",
           n_files_changed, total_added, total_removed);
}

/* Output file alignment table in JSON format */
void output_alignment_json(const AlignmentEntry* entries, int n_entries,
                           const char* ref1, const char* ref2) {
    int total_added = 0, total_removed = 0;
    int n_added = 0, n_removed = 0, n_modified = 0, n_identical = 0;

    for (int i = 0; i < n_entries; i++) {
        total_added += entries[i].added;
        total_removed += entries[i].removed;
        switch (entries[i].type) {
            case ALIGN_ADDED:    n_added++;    break;
            case ALIGN_REMOVED:  n_removed++;  break;
            case ALIGN_MODIFIED: n_modified++; break;
            case ALIGN_IDENTICAL: n_identical++; break;
        }
    }

    printf("{\n");
    printf("  \"header\" : {\n");
    printf("    \"cloc_url\"           : \"github.com/rloc/rloc\",\n");
    printf("    \"cloc_version\"       : \"%s\",\n", RLOC_VERSION);
    printf("    \"diff\"               : \"%s..%s\",\n", ref1, ref2);
    printf("    \"n_files\"            : %d,\n", n_added + n_removed + n_modified);
    printf("    \"n_added\"            : %d,\n", n_added);
    printf("    \"n_removed\"          : %d,\n", n_removed);
    printf("    \"n_modified\"         : %d,\n", n_modified);
    printf("    \"n_identical\"        : %d,\n", n_identical);
    printf("    \"total_added\"        : %d,\n", total_added);
    printf("    \"total_removed\"      : %d\n", total_removed);
    printf("  },\n");
    printf("  \"files\" : [\n");

    int first = 1;
    for (int i = 0; i < n_entries; i++) {
        if (!first) printf(",\n");
        first = 0;

        const char* type_str;
        switch (entries[i].type) {
            case ALIGN_ADDED:    type_str = "added";    break;
            case ALIGN_REMOVED:  type_str = "removed";  break;
            case ALIGN_MODIFIED: type_str = "modified"; break;
            case ALIGN_IDENTICAL: type_str = "identical"; break;
            default:             type_str = "unknown";  break;
        }

        const char* lang = entries[i].language
            ? entries[i].language
            : (entries[i].type == ALIGN_IDENTICAL ? "(identical)" : "(none)");

        printf("    {\n");
        printf("      \"name\"    : ");
        json_write_string(stdout, entries[i].filepath);
        printf(",\n");
        printf("      \"type\"    : \"%s\",\n", type_str);
        printf("      \"added\"   : %d,\n", entries[i].added);
        printf("      \"removed\" : %d,\n", entries[i].removed);
        printf("      \"language\": ");
        json_write_string(stdout, lang);
        printf("\n");
        printf("    }");
    }

    printf("\n  ]\n");
    printf("}\n");
}

/* Output YAML format */
void output_yaml(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    int n_valid = count_valid_files(files, n_files);

    printf("---\n");
    printf("header:\n");
    printf("  cloc_url: github.com/rloc/rloc\n");
    printf("  cloc_version: %s\n", RLOC_VERSION);
    printf("  elapsed_seconds: %.2f\n", elapsed_sec);
    printf("  n_files: %d\n", n_valid);
    printf("  files_per_second: %.1f\n", (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0);
    printf("languages:\n");

    for (int i = 0; i < n_languages; i++) {
        printf("  - name: %s\n", lang_stats[i].name);
        printf("    files: %d\n", lang_stats[i].files);
        printf("    blank: %d\n", lang_stats[i].blank);
        printf("    comment: %d\n", lang_stats[i].comment);
        printf("    code: %d\n", lang_stats[i].code);
    }

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);
    printf("SUM:\n");
    printf("  files: %d\n", total_files);
    printf("  blank: %d\n", total_blank);
    printf("  comment: %d\n", total_comment);
    printf("  code: %d\n", total_code);
}

/* Output YAML with per-file breakdown */
void output_yaml_by_file(const FileStats* files, int n_files, double elapsed_sec) {
    int n_valid = count_valid_files(files, n_files);

    printf("---\n");
    printf("header:\n");
    printf("  cloc_url: github.com/rloc/rloc\n");
    printf("  cloc_version: %s\n", RLOC_VERSION);
    printf("  elapsed_seconds: %.2f\n", elapsed_sec);
    printf("  n_files: %d\n", n_valid);
    printf("  files_per_second: %.1f\n", (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0);
    printf("files:\n");

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("  - path: %s\n", files[i].filepath);
        printf("    language: %s\n", files[i].lang->name);
        printf("    blank: %d\n", files[i].counts.blank);
        printf("    comment: %d\n", files[i].counts.comment);
        printf("    code: %d\n", files[i].counts.code);
    }
}

/* Output by file and language combined */
void output_by_file_by_lang(const FileStats* files, int n_files, double elapsed_sec) {
    (void)elapsed_sec;  // API consistency, not used in this format
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);

    printf("%-40s %-15s %8s %8s %8s %8s\n", "File", "Language", "Blank", "Comment", "Code", "");
    printf("------------------------------------------------------------------------------\n");

    // Sort files by language then by filename
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("%-40s %-15s %8d %8d %8d\n", files[i].filepath, files[i].lang->name,
               files[i].counts.blank, files[i].counts.comment, files[i].counts.code);
    }

    printf("------------------------------------------------------------------------------\n");
    printf("%-40s %-15s %8d %8d %8d\n", "SUM:", "", total_blank, total_comment, total_code);
}

/* Helper: Escape special XML characters */
static void xml_escape(const char* str, char* buf, size_t buf_size) {
    size_t j = 0;
    for (size_t i = 0; str[i] && j < buf_size - 1; i++) {
        if (str[i] == '&') {
            if (j + 5 < buf_size) { buf[j++] = '&'; buf[j++] = 'a'; buf[j++] = 'm'; buf[j++] = 'p'; buf[j++] = ';'; }
        } else if (str[i] == '<') {
            if (j + 4 < buf_size) { buf[j++] = '&'; buf[j++] = 'l'; buf[j++] = 't'; buf[j++] = ';'; }
        } else if (str[i] == '>') {
            if (j + 4 < buf_size) { buf[j++] = '&'; buf[j++] = 'g'; buf[j++] = 't'; buf[j++] = ';'; }
        } else if (str[i] == '"') {
            if (j + 6 < buf_size) { buf[j++] = '&'; buf[j++] = 'q'; buf[j++] = 'u'; buf[j++] = 'o'; buf[j++] = 't'; buf[j++] = ';'; }
        } else if (str[i] == '\'') {
            if (j + 6 < buf_size) { buf[j++] = '&'; buf[j++] = 'a'; buf[j++] = 'p'; buf[j++] = 'o'; buf[j++] = 's'; buf[j++] = ';'; }
        } else {
            buf[j++] = str[i];
        }
    }
    buf[j] = '\0';
}

/* Output XML format */
void output_xml(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    int n_valid = count_valid_files(files, n_files);
    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;
    double lines_per_sec = (elapsed_sec > 0) ? ((total_blank + total_comment + total_code) / elapsed_sec) : 0.0;

    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<results>\n");
    printf("  <header>\n");
    printf("    <cloc_url>github.com/rloc/rloc</cloc_url>\n");
    printf("    <cloc_version>%s</cloc_version>\n", RLOC_VERSION);
    printf("    <elapsed_seconds>%.3f</elapsed_seconds>\n", elapsed_sec);
    printf("    <n_files>%d</n_files>\n", n_valid);
    printf("    <n_lines>%d</n_lines>\n", total_blank + total_comment + total_code);
    printf("    <files_per_second>%.1f</files_per_second>\n", files_per_sec);
    printf("    <lines_per_second>%.1f</lines_per_second>\n", lines_per_sec);
    printf("  </header>\n");
    printf("  <languages>\n");

    char escaped[256];
    for (int i = 0; i < n_languages; i++) {
        xml_escape(lang_stats[i].name, escaped, sizeof(escaped));
        printf("    <language name=\"%s\" files=\"%d\" blank=\"%d\" comment=\"%d\" code=\"%d\"/>\n",
               escaped, lang_stats[i].files, lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    printf("    <total sum_files=\"%d\" blank=\"%d\" comment=\"%d\" code=\"%d\"/>\n",
           total_files, total_blank, total_comment, total_code);
    printf("  </languages>\n");
    printf("</results>\n");
}

/* Output XML with per-file breakdown */
void output_xml_by_file(const FileStats* files, int n_files, double elapsed_sec) {
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

    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<results>\n");
    printf("  <header>\n");
    printf("    <cloc_url>github.com/rloc/rloc</cloc_url>\n");
    printf("    <cloc_version>%s</cloc_version>\n", RLOC_VERSION);
    printf("    <elapsed_seconds>%.3f</elapsed_seconds>\n", elapsed_sec);
    printf("    <n_files>%d</n_files>\n", n_valid);
    printf("    <files_per_second>%.1f</files_per_second>\n", files_per_sec);
    printf("  </header>\n");
    printf("  <files>\n");

    char escaped_path[512];
    char escaped_lang[256];
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        xml_escape(files[i].filepath, escaped_path, sizeof(escaped_path));
        xml_escape(files[i].lang->name, escaped_lang, sizeof(escaped_lang));
        printf("    <file path=\"%s\" language=\"%s\" blank=\"%d\" comment=\"%d\" code=\"%d\"/>\n",
               escaped_path, escaped_lang, files[i].counts.blank, files[i].counts.comment, files[i].counts.code);
    }

    printf("    <total blank=\"%d\" comment=\"%d\" code=\"%d\"/>\n",
           total_blank, total_comment, total_code);
    printf("  </files>\n");
    printf("</results>\n");
}

/* Output HTML format */
void output_html(const FileStats* files, int n_files, double elapsed_sec) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    int n_valid = count_valid_files(files, n_files);
    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);

    double files_per_sec = (elapsed_sec > 0) ? (n_valid / elapsed_sec) : 0.0;

    printf("<!DOCTYPE html>\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>rloc Report</title>\n");
    printf("<style>\n");
    printf("table { border-collapse: collapse; width: 100%%; }\n");
    printf("th, td { border: 1px solid #ddd; padding: 8px; text-align: right; }\n");
    printf("th { background-color: #4CAF50; color: white; }\n");
    printf("tr:nth-child(even) { background-color: #f2f2f2; }\n");
    printf(".sum { font-weight: bold; background-color: #e7e7e7; }\n");
    printf(".lang { text-align: left; }\n");
    printf("</style>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<h1>rloc Report v%s</h1>\n", RLOC_VERSION);
    printf("<p>Processed %d files in %.2f seconds (%.1f files/s)</p>\n", n_valid, elapsed_sec, files_per_sec);
    printf("<table>\n");
    printf("<tr><th class=\"lang\">Language</th><th>files</th><th>blank</th><th>comment</th><th>code</th></tr>\n");

    for (int i = 0; i < n_languages; i++) {
        printf("<tr><td class=\"lang\">%s</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",
               lang_stats[i].name, lang_stats[i].files, lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    printf("<tr class=\"sum\"><td class=\"lang\">SUM</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",
           total_files, total_blank, total_comment, total_code);
    printf("</table>\n");
    printf("</body>\n");
    printf("</html>\n");
}

/* Output HTML with per-file breakdown */
void output_html_by_file(const FileStats* files, int n_files, double elapsed_sec) {
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

    printf("<!DOCTYPE html>\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>rloc Report</title>\n");
    printf("<style>\n");
    printf("table { border-collapse: collapse; width: 100%%; }\n");
    printf("th, td { border: 1px solid #ddd; padding: 8px; text-align: right; }\n");
    printf("th { background-color: #4CAF50; color: white; }\n");
    printf("tr:nth-child(even) { background-color: #f2f2f2; }\n");
    printf(".sum { font-weight: bold; background-color: #e7e7e7; }\n");
    printf(".file { text-align: left; }\n");
    printf(".lang { text-align: left; }\n");
    printf("</style>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<h1>rloc Report v%s</h1>\n", RLOC_VERSION);
    printf("<p>Processed %d files in %.2f seconds (%.1f files/s)</p>\n", n_valid, elapsed_sec, files_per_sec);
    printf("<table>\n");
    printf("<tr><th class=\"file\">File</th><th class=\"lang\">Language</th><th>blank</th><th>comment</th><th>code</th></tr>\n");

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        printf("<tr><td class=\"file\">%s</td><td class=\"lang\">%s</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",
               files[i].filepath, files[i].lang->name, files[i].counts.blank, files[i].counts.comment, files[i].counts.code);
    }

    printf("<tr class=\"sum\"><td class=\"file\">SUM</td><td class=\"lang\"></td><td>%d</td><td>%d</td><td>%d</td></tr>\n",
           total_blank, total_comment, total_code);
    printf("</table>\n");
    printf("</body>\n");
    printf("</html>\n");
}

/* Helper: Escape SQL string (single quotes) */
static void sql_escape(const char* str, char* buf, size_t buf_size) {
    size_t j = 0;
    for (size_t i = 0; str[i] && j < buf_size - 1; i++) {
        if (str[i] == '\'') {
            if (j + 2 < buf_size) { buf[j++] = '\''; buf[j++] = '\''; }
        } else {
            buf[j++] = str[i];
        }
    }
    buf[j] = '\0';
}

/* Output SQL format (SQLite-compatible) */
void output_sql(const FileStats* files, int n_files, double elapsed_sec, const char* project) {
    LanguageStats lang_stats[MAX_LANGUAGES];
    int n_languages = 0;
    aggregate_by_language(files, n_files, lang_stats, &n_languages);

    qsort(lang_stats, n_languages, sizeof(LanguageStats), compare_language_names);

    int total_files, total_blank, total_comment, total_code;
    calculate_totals(lang_stats, n_languages, &total_files, &total_blank, &total_comment,
                     &total_code);

    char escaped_project[256];
    sql_escape(project ? project : "rloc", escaped_project, sizeof(escaped_project));

    printf("-- rloc SQL output\n");
    printf("-- Generated at %.2f seconds elapsed\n\n", elapsed_sec);

    printf("CREATE TABLE IF NOT EXISTS metadata (\n");
    printf("    id INTEGER PRIMARY KEY,\n");
    printf("    timestamp TEXT,\n");
    printf("    project TEXT,\n");
    printf("    elapsed_s REAL\n");
    printf(");\n\n");

    printf("CREATE TABLE IF NOT EXISTS results (\n");
    printf("    id INTEGER,\n");
    printf("    project TEXT,\n");
    printf("    language TEXT,\n");
    printf("    nFiles INTEGER,\n");
    printf("    nBlank INTEGER,\n");
    printf("    nComment INTEGER,\n");
    printf("    nCode INTEGER\n");
    printf(");\n\n");

    printf("INSERT INTO metadata (id, timestamp, project, elapsed_s) VALUES (1, datetime('now'), '%s', %.3f);\n",
           escaped_project, elapsed_sec);

    char escaped_lang[256];
    for (int i = 0; i < n_languages; i++) {
        sql_escape(lang_stats[i].name, escaped_lang, sizeof(escaped_lang));
        printf("INSERT INTO results (id, project, language, nFiles, nBlank, nComment, nCode) VALUES (1, '%s', '%s', %d, %d, %d, %d);\n",
               escaped_project, escaped_lang, lang_stats[i].files, lang_stats[i].blank, lang_stats[i].comment, lang_stats[i].code);
    }

    printf("-- SUM: %d files, %d blank, %d comment, %d code\n",
           total_files, total_blank, total_comment, total_code);
}

/* Output SQL with per-file breakdown */
void output_sql_by_file(const FileStats* files, int n_files, double elapsed_sec, const char* project) {
    char escaped_project[256];
    sql_escape(project ? project : "rloc", escaped_project, sizeof(escaped_project));

    printf("-- rloc SQL output (by file)\n");
    printf("-- Generated at %.2f seconds elapsed\n\n", elapsed_sec);

    printf("CREATE TABLE IF NOT EXISTS metadata (\n");
    printf("    id INTEGER PRIMARY KEY,\n");
    printf("    timestamp TEXT,\n");
    printf("    project TEXT,\n");
    printf("    elapsed_s REAL\n");
    printf(");\n\n");

    printf("CREATE TABLE IF NOT EXISTS files (\n");
    printf("    id INTEGER,\n");
    printf("    project TEXT,\n");
    printf("    filepath TEXT,\n");
    printf("    language TEXT,\n");
    printf("    nBlank INTEGER,\n");
    printf("    nComment INTEGER,\n");
    printf("    nCode INTEGER\n");
    printf(");\n\n");

    printf("INSERT INTO metadata (id, timestamp, project, elapsed_s) VALUES (1, datetime('now'), '%s', %.3f);\n",
           escaped_project, elapsed_sec);

    char escaped_path[512];
    char escaped_lang[256];
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL) continue;
        sql_escape(files[i].filepath, escaped_path, sizeof(escaped_path));
        sql_escape(files[i].lang->name, escaped_lang, sizeof(escaped_lang));
        printf("INSERT INTO files (id, project, filepath, language, nBlank, nComment, nCode) VALUES (1, '%s', '%s', '%s', %d, %d, %d);\n",
               escaped_project, escaped_path, escaped_lang, files[i].counts.blank, files[i].counts.comment, files[i].counts.code);
    }

    printf("-- Total: %d files\n", count_valid_files(files, n_files));
}
