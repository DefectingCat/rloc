#include "diff.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DiffFileStats* diff_get_files(const char* repo_path, const char* commit1, const char* commit2,
                              int* n_files) {
    *n_files = 0;

    // Build git diff --numstat command
    // --numstat outputs: added removed filename
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "cd '%s' && git diff --numstat '%s' '%s' 2>/dev/null", repo_path,
             commit1, commit2);

    FILE* fp = popen(cmd, "r");
    if (!fp) {
        return NULL;
    }

    // Read all output into a buffer
    char buffer[65536];  // 64KB buffer
    size_t total_read = 0;
    size_t chunk;

    while ((chunk = fread(buffer + total_read, 1, sizeof(buffer) - total_read - 1, fp)) > 0) {
        total_read += chunk;
        if (total_read >= sizeof(buffer) - 1) break;
    }
    pclose(fp);

    if (total_read == 0) {
        return NULL;
    }

    // Count lines (newline-separated)
    int count = 0;
    for (size_t i = 0; i < total_read; i++) {
        if (buffer[i] == '\n') count++;
    }
    // Handle last line if not newline-terminated
    if (total_read > 0 && buffer[total_read - 1] != '\n') count++;

    if (count == 0) {
        return NULL;
    }

    // Allocate file array
    DiffFileStats* files = malloc(count * sizeof(DiffFileStats));
    if (!files) {
        return NULL;
    }

    // Parse each line: added removed filename
    int idx = 0;
    char* line_start = buffer;
    for (size_t i = 0; i < total_read && idx < count; i++) {
        if (buffer[i] == '\n' || i == total_read - 1) {
            size_t len = (buffer[i] == '\n') ? (i - (line_start - buffer))
                                             : (total_read - (line_start - buffer));
            if (len > 0) {
                // Parse the line
                char line[1024];
                if (len >= sizeof(line)) len = sizeof(line) - 1;
                memcpy(line, line_start, len);
                line[len] = '\0';

                // Format: added removed filename (TAB-separated)
                // Binary files show as "- - filename"
                char* tab1 = strchr(line, '\t');
                if (tab1) {
                    char* tab2 = strchr(tab1 + 1, '\t');
                    if (tab2) {
                        // Extract added count
                        char added_str[32];
                        size_t added_len = tab1 - line;
                        if (added_len >= sizeof(added_str)) added_len = sizeof(added_str) - 1;
                        memcpy(added_str, line, added_len);
                        added_str[added_len] = '\0';

                        // Extract removed count
                        char removed_str[32];
                        size_t removed_len = tab2 - tab1 - 1;
                        if (removed_len >= sizeof(removed_str))
                            removed_len = sizeof(removed_str) - 1;
                        memcpy(removed_str, tab1 + 1, removed_len);
                        removed_str[removed_len] = '\0';

                        // Extract filename
                        char* filename = tab2 + 1;

                        // Handle binary files (- -)
                        int added = (strcmp(added_str, "-") == 0) ? 0 : atoi(added_str);
                        int removed = (strcmp(removed_str, "-") == 0) ? 0 : atoi(removed_str);

                        files[idx].filepath = strdup(filename);
                        files[idx].added = added;
                        files[idx].removed = removed;
                        files[idx].lang = NULL;  // Will be set by caller if needed
                        idx++;
                    }
                }
            }
            line_start = buffer + i + 1;
        }
    }

    *n_files = idx;
    return files;
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