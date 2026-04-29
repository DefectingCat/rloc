#include "diff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* Helper: execute git command and read all output into a Buffer */
static Buffer* run_git_cmd(const char* repo_path, const char* git_args) {
    // Escape repo_path for shell safety
    char* escaped_repo = escape_shell_arg(repo_path);
    if (!escaped_repo) return NULL;

    char cmd[4096];
    snprintf(cmd, sizeof(cmd), "cd %s && %s 2>/dev/null", escaped_repo, git_args);
    free(escaped_repo);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    Buffer* buf = buffer_new(65536);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    char read_buf[8192];
    size_t chunk;
    while ((chunk = fread(read_buf, 1, sizeof(read_buf), fp)) > 0) {
        if (buffer_append(buf, read_buf, chunk) != 0) break;
    }
    pclose(fp);

    if (buf->size == 0) {
        buffer_free(buf);
        return NULL;
    }
    return buf;
}

/* Helper: parse git diff --numstat -z output (null-separated) */
static DiffFileStats* parse_numstat_z(Buffer* buf, int* n_files) {
    *n_files = 0;
    if (!buf) return NULL;

    size_t total = buf->size;

    /* Count entries (null-separated filenames) */
    int count = 0;
    for (size_t i = 0; i < total; i++) {
        if (buf->data[i] == '\0') count++;
    }
    if (total > 0 && buf->data[total - 1] != '\0') count++;

    if (count == 0) return NULL;

    DiffFileStats* files = calloc(count, sizeof(DiffFileStats));
    if (!files) return NULL;

    int idx = 0;
    char* line_start = buf->data;

    for (size_t i = 0; i < total && idx < count; i++) {
        if (buf->data[i] == '\0' || (i == total - 1 && buf->data[i] != '\n')) {
            size_t len = (buf->data[i] == '\0') ? (i - (size_t)(line_start - buf->data))
                                                : (total - (size_t)(line_start - buf->data));

            /* Copy line for parsing */
            char line[2048];
            if (len >= sizeof(line)) len = sizeof(line) - 1;
            memcpy(line, line_start, len);
            line[len] = '\0';

            /* Parse: added<TAB>removed<TAB>filename */
            char* tab1 = strchr(line, '\t');
            if (tab1) {
                char* tab2 = strchr(tab1 + 1, '\t');
                if (tab2) {
                    char* added_str = line;
                    char* removed_str = tab1 + 1;
                    char* filename = tab2 + 1;

                    *tab1 = '\0';
                    *tab2 = '\0';

                    /* Binary files show as "-" */
                    int added = 0, removed = 0;
                    if (strcmp(added_str, "-") != 0) added = atoi(added_str);
                    if (strcmp(removed_str, "-") != 0) removed = atoi(removed_str);

                    files[idx].filepath = strdup(filename);
                    files[idx].added = added;
                    files[idx].removed = removed;
                    files[idx].lang = NULL;
                    idx++;
                }
            }

            if (buf->data[i] == '\0')
                line_start = buf->data + i + 1;
            else
                break;
        }
    }

    *n_files = idx;
    return files;
}

/* Helper: compare two strings, return 1 if equal */
static int str_eq(const char* a, const char* b) {
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    return strcmp(a, b) == 0;
}

/* Helper: merge two file lists, removing duplicates */
static DiffFileStats* merge_unique(DiffFileStats* a, int n_a, DiffFileStats* b, int n_b,
                                   int* n_out) {
    int cap = n_a + n_b;
    DiffFileStats* merged = calloc(cap, sizeof(DiffFileStats));
    if (!merged) {
        *n_out = 0;
        return NULL;
    }

    int idx = 0;

    /* Copy all from a */
    for (int i = 0; i < n_a; i++) {
        if (a[i].filepath) {
            merged[idx] = a[i];
            a[i].filepath = NULL; /* transfer ownership */
            idx++;
        }
    }

    /* Copy from b, skipping duplicates */
    for (int i = 0; i < n_b; i++) {
        if (!b[i].filepath) continue;
        int dup = 0;
        for (int j = 0; j < idx; j++) {
            if (merged[j].filepath && str_eq(merged[j].filepath, b[i].filepath)) {
                dup = 1;
                break;
            }
        }
        if (!dup) {
            merged[idx] = b[i];
            b[i].filepath = NULL;
            idx++;
        }
    }

    *n_out = idx;
    return merged;
}

/* Helper: sort file array by filepath */
static int compare_filepath(const void* a, const void* b) {
    const DiffFileStats* fa = (const DiffFileStats*)a;
    const DiffFileStats* fb = (const DiffFileStats*)b;
    const char* sa = fa->filepath ? fa->filepath : "";
    const char* sb = fb->filepath ? fb->filepath : "";
    return strcmp(sa, sb);
}

/* Helper: find file in array by name, return index or -1 */
static int find_file(const DiffFileStats* arr, int n, const char* name) {
    for (int i = 0; i < n; i++) {
        if (arr[i].filepath && strcmp(arr[i].filepath, name) == 0) return i;
    }
    return -1;
}

DiffFileStats* diff_get_files(const char* repo_path, const char* commit1, const char* commit2,
                              int* n_files) {
    *n_files = 0;

    if (!repo_path || !commit1 || !commit2) return NULL;

    /* Early exit: same commit */
    if (strcmp(commit1, commit2) == 0) return NULL;

    // Escape commit refs for shell safety
    char* escaped_c1 = escape_shell_arg(commit1);
    char* escaped_c2 = escape_shell_arg(commit2);
    if (!escaped_c1 || !escaped_c2) {
        free(escaped_c1);
        free(escaped_c2);
        return NULL;
    }

    char cmd[4096];
    snprintf(cmd, sizeof(cmd), "git diff --numstat -z %s %s", escaped_c1, escaped_c2);

    free(escaped_c1);
    free(escaped_c2);

    Buffer* buf = run_git_cmd(repo_path, cmd);
    if (!buf) return NULL;

    DiffFileStats* files = parse_numstat_z(buf, n_files);
    buffer_free(buf);
    return files;
}

DiffFileStats* diff_get_stats_extended(const DiffConfig* config, int* n_files) {
    *n_files = 0;
    if (!config || !config->repo_path || !config->ref1) return NULL;

    const char* repo = config->repo_path;
    const char* ref1 = config->ref1;
    const char* ref2 = config->ref2 ? config->ref2 : "HEAD";

    /* Early exit: same reference */
    if (strcmp(ref1, ref2) == 0) return NULL;

    // Escape refs for shell safety
    char* escaped_ref1 = escape_shell_arg(ref1);
    char* escaped_ref2 = escape_shell_arg(ref2);
    if (!escaped_ref1 || !escaped_ref2) {
        free(escaped_ref1);
        free(escaped_ref2);
        return NULL;
    }

    /* DIFF_MODE_RELATIVE: only changed files via git diff --numstat */
    if (config->flags & DIFF_MODE_RELATIVE) {
        char cmd[4096];
        char* extra = "";
        if (config->flags & DIFF_IGNORE_WHITESPACE) extra = " -w";
        snprintf(cmd, sizeof(cmd), "git diff --numstat -z%s %s %s", extra, escaped_ref1, escaped_ref2);

        free(escaped_ref1);
        free(escaped_ref2);

        Buffer* buf = run_git_cmd(repo, cmd);
        if (!buf) return NULL;

        DiffFileStats* files = parse_numstat_z(buf, n_files);
        buffer_free(buf);
        return files;
    }

    /* DIFF_MODE_ALL: get all files from both refs, then merge with diff stats */
    char cmd[4096];

    /* Get files at ref1 */
    snprintf(cmd, sizeof(cmd), "git ls-tree -r --name-only -z %s", escaped_ref1);
    Buffer* buf_ref1 = run_git_cmd(repo, cmd);
    if (!buf_ref1) return NULL;

    /* Parse ref1 file list */
    int n_ref1 = 0;
    size_t total1 = buf_ref1->size;
    for (size_t i = 0; i < total1; i++) {
        if (buf_ref1->data[i] == '\0') n_ref1++;
    }
    if (total1 > 0 && buf_ref1->data[total1 - 1] != '\0') n_ref1++;

    DiffFileStats* files1 = NULL;
    if (n_ref1 > 0) {
        files1 = calloc(n_ref1, sizeof(DiffFileStats));
        if (!files1) {
            buffer_free(buf_ref1);
            return NULL;
        }

        int idx = 0;
        char* start = buf_ref1->data;
        for (size_t i = 0; i < total1 && idx < n_ref1; i++) {
            if (buf_ref1->data[i] == '\0') {
                size_t len = i - (size_t)(start - buf_ref1->data);
                if (len > 0) {
                    files1[idx].filepath = malloc(len + 1);
                    if (files1[idx].filepath) {
                        memcpy(files1[idx].filepath, start, len);
                        files1[idx].filepath[len] = '\0';
                        files1[idx].added = 0;
                        files1[idx].removed = 0;
                        files1[idx].lang = NULL;
                        idx++;
                    }
                }
                start = buf_ref1->data + i + 1;
            }
        }
        n_ref1 = idx;
    }
    buffer_free(buf_ref1);

    /* Get files at ref2 */
    snprintf(cmd, sizeof(cmd), "git ls-tree -r --name-only -z %s", escaped_ref2);
    Buffer* buf_ref2 = run_git_cmd(repo, cmd);
    if (!buf_ref2) {
        diff_free_files(files1, n_ref1);
        return NULL;
    }

    int n_ref2 = 0;
    size_t total2 = buf_ref2->size;
    for (size_t i = 0; i < total2; i++) {
        if (buf_ref2->data[i] == '\0') n_ref2++;
    }
    if (total2 > 0 && buf_ref2->data[total2 - 1] != '\0') n_ref2++;

    DiffFileStats* files2 = NULL;
    if (n_ref2 > 0) {
        files2 = calloc(n_ref2, sizeof(DiffFileStats));
        if (!files2) {
            diff_free_files(files1, n_ref1);
            buffer_free(buf_ref2);
            return NULL;
        }

        int idx = 0;
        char* start = buf_ref2->data;
        for (size_t i = 0; i < total2 && idx < n_ref2; i++) {
            if (buf_ref2->data[i] == '\0') {
                size_t len = i - (size_t)(start - buf_ref2->data);
                if (len > 0) {
                    files2[idx].filepath = malloc(len + 1);
                    if (files2[idx].filepath) {
                        memcpy(files2[idx].filepath, start, len);
                        files2[idx].filepath[len] = '\0';
                        files2[idx].added = 0;
                        files2[idx].removed = 0;
                        files2[idx].lang = NULL;
                        idx++;
                    }
                }
                start = buf_ref2->data + i + 1;
            }
        }
        n_ref2 = idx;
    }
    buffer_free(buf_ref2);

    /* Get diff stats with rename detection */
    char diff_cmd[4096];
    int diff_cmd_len = 0;
    diff_cmd_len = snprintf(diff_cmd, sizeof(diff_cmd), "git diff --numstat -z -M");
    if (config->flags & DIFF_IGNORE_WHITESPACE) {
        diff_cmd_len += snprintf(diff_cmd + diff_cmd_len, sizeof(diff_cmd) - diff_cmd_len, " -w");
    }
    diff_cmd_len += snprintf(diff_cmd + diff_cmd_len, sizeof(diff_cmd) - diff_cmd_len, " %s %s",
                             escaped_ref1, escaped_ref2);

    // Free escaped refs after building all commands
    free(escaped_ref1);
    free(escaped_ref2);

    Buffer* diff_buf = run_git_cmd(repo, diff_cmd);
    if (!diff_buf) {
        diff_free_files(files1, n_ref1);
        diff_free_files(files2, n_ref2);
        return NULL;
    }

    /* Parse diff stats */
    int n_diff = 0;
    DiffFileStats* diff_stats = parse_numstat_z(diff_buf, &n_diff);
    buffer_free(diff_buf);

    /* Merge all unique files */
    int n_merged = 0;
    DiffFileStats* merged = merge_unique(files1, n_ref1, files2, n_ref2, &n_merged);

    /* Clean up original arrays (filepaths already transferred) */
    free(files1);
    free(files2);

    if (!merged) {
        diff_free_files(diff_stats, n_diff);
        return NULL;
    }

    /* Sort merged list for easier lookup */
    qsort(merged, n_merged, sizeof(DiffFileStats), compare_filepath);

    /* Apply diff stats to merged list */
    for (int i = 0; i < n_diff; i++) {
        int idx = find_file(merged, n_merged, diff_stats[i].filepath);
        if (idx >= 0) {
            merged[idx].added = diff_stats[i].added;
            merged[idx].removed = diff_stats[i].removed;
        }
    }

    diff_free_files(diff_stats, n_diff);

    *n_files = n_merged;
    return merged;
}

void diff_free_config(DiffConfig* config) {
    if (!config) return;
    if (config->ref1) {
        free((char*)config->ref1);
        config->ref1 = NULL;
    }
    if (config->ref2) {
        free((char*)config->ref2);
        config->ref2 = NULL;
    }
    if (config->repo_path) {
        free((char*)config->repo_path);
        config->repo_path = NULL;
    }
    config->flags = 0;
}

void diff_free_files(DiffFileStats* files, int n_files) {
    if (!files) return;
    for (int i = 0; i < n_files; i++) {
        if (files[i].filepath) {
            free(files[i].filepath);
        }
    }
    free(files);
}
