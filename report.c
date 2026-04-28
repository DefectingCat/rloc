#include <stdio.h>
#include <stdlib.h>

#include "report.h"
#include "util.h"

int report_write_unique_file(const FileStats* files, int n_files, const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return -1;

    for (int i = 0; i < n_files; i++) {
        if (files[i].ignore_reason == NULL) {
            fprintf(fp, "%s\n", files[i].filepath);
        }
    }
    fclose(fp);
    return 0;
}

int report_write_ignored_file(const FileStats* files, int n_files, const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return -1;

    for (int i = 0; i < n_files; i++) {
        if (files[i].ignore_reason != NULL) {
            fprintf(fp, "%s\t%s\n", files[i].filepath, files[i].ignore_reason);
        }
    }
    fclose(fp);
    return 0;
}

int report_write_found_file(const FileStats* files, int n_files, const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return -1;

    for (int i = 0; i < n_files; i++) {
        fprintf(fp, "%s\n", files[i].filepath);
    }
    fclose(fp);
    return 0;
}

int report_write_counted_file(const FileStats* files, int n_files, const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return -1;

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang != NULL) {
            fprintf(fp, "%s\n", files[i].filepath);
        }
    }
    fclose(fp);
    return 0;
}

int report_write_categorized_file(const FileStats* files, int n_files, const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (!fp) return -1;

    fprintf(fp, "filepath\tsize\tlanguage\tcategory\n");
    for (int i = 0; i < n_files; i++) {
        long size = get_file_size(files[i].filepath);
        const char* lang_name = files[i].lang ? files[i].lang->name : "(unknown)";
        const char* category = files[i].ignore_reason ? files[i].ignore_reason : "counted";
        fprintf(fp, "%s\t%ld\t%s\t%s\n", files[i].filepath, size, lang_name, category);
    }
    fclose(fp);
    return 0;
}
