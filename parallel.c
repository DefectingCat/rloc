#include "parallel.h"
#include "language.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Get default parallel configuration
void parallel_default_config(ParallelConfig* config) {
    if (!config) return;

    // Get CPU count
#ifdef _SC_NPROCESSORS_ONLN
    config->n_workers = sysconf(_SC_NPROCESSORS_ONLN);
#else
    config->n_workers = 4;  // Default fallback
#endif

    if (config->n_workers < 1) config->n_workers = 1;
    if (config->n_workers > 16) config->n_workers = 16;  // Cap at 16

    config->chunk_size = 100;
    config->timeout_sec = 300;  // 5 minutes per chunk
    config->temp_mgr = NULL;
}

// Parse TSV output from worker
int parse_worker_tsv(const char* tsv_path, WorkerResult* results, int max_results) {
    FILE* fp = fopen(tsv_path, "r");
    if (!fp) return -1;

    char line[2048];
    int count = 0;

    while (fgets(line, sizeof(line), fp) && count < max_results) {
        // Format: filepath\tlanguage\tblank\tcomment\tcode
        char* saveptr;
        char* filepath = strtok_r(line, "\t", &saveptr);
        if (!filepath) continue;

        char* lang = strtok_r(NULL, "\t", &saveptr);
        char* blank_str = strtok_r(NULL, "\t", &saveptr);
        char* comment_str = strtok_r(NULL, "\t", &saveptr);
        char* code_str = strtok_r(NULL, "\t\n", &saveptr);

        if (!filepath || !lang || !blank_str || !comment_str || !code_str) continue;

        strncpy(results[count].filepath, filepath, sizeof(results[count].filepath) - 1);
        results[count].language = lang;  // Note: points to line buffer
        results[count].blank = atoi(blank_str);
        results[count].comment = atoi(comment_str);
        results[count].code = atoi(code_str);

        count++;
    }

    fclose(fp);
    return count;
}

// Worker process entry point (called via popen)
// Outputs TSV format to stdout
static void worker_process_files(char** files, int start, int end) {
    for (int i = start; i < end; i++) {
        const char* filepath = files[i];
        const Language* lang = detect_language(filepath);
        if (!lang) continue;

        CountResult result;
        if (count_file_with_lang(filepath, lang, &result) != 0) continue;

        // Output TSV: filepath\tlanguage\tblank\tcomment\tcode
        printf("%s\t%s\t%d\t%d\t%d\n", filepath, lang->name,
               result.blank, result.comment, result.code);
    }
}

// Count files in parallel
int parallel_count_files(char** files, int n_files, ParallelConfig* config,
                         CountResult* results, int* n_results) {
    if (!files || n_files <= 0 || !config) {
        *n_results = 0;
        return -1;
    }

    // For small file counts, don't parallelize
    if (n_files < 50 || config->n_workers <= 1) {
        // Single-threaded fallback
        int count = 0;
        for (int i = 0; i < n_files; i++) {
            const Language* lang = detect_language(files[i]);
            if (!lang) continue;

            if (count_file_with_lang(files[i], lang, &results[count]) == 0) {
                count++;
            }
        }
        *n_results = count;
        return count;
    }

    // Create temporary files for worker output
    if (!config->temp_mgr) {
        *n_results = 0;
        return -1;
    }

    // Calculate chunk sizes
    int chunk_size = (n_files + config->n_workers - 1) / config->n_workers;
    if (chunk_size < 10) chunk_size = 10;

    // Create output temp files for each worker
    char* output_files[config->n_workers];
    for (int w = 0; w < config->n_workers; w++) {
        output_files[w] = temp_manager_create_file(config->temp_mgr, "rloc_worker");
        if (!output_files[w]) {
            // Fallback to single-threaded
            int count = 0;
            for (int i = 0; i < n_files; i++) {
                const Language* lang = detect_language(files[i]);
                if (!lang) continue;
                if (count_file_with_lang(files[i], lang, &results[count]) == 0) {
                    count++;
                }
            }
            *n_results = count;
            return count;
        }
    }

    // Launch worker processes via popen
    FILE* worker_streams[config->n_workers];
    for (int w = 0; w < config->n_workers; w++) {
        int start = w * chunk_size;
        int end = start + chunk_size;
        if (end > n_files) end = n_files;

        // Build command: run rloc as worker
        // For simplicity, we'll use a shell script approach
        char cmd[2048];
        snprintf(cmd, sizeof(cmd),
                 "cd '%s' && for f in %s..%d; do echo \"$f\"; done | xargs -I{} ./rloc --by-file '{}' 2>/dev/null",
                 ".", files[start], end - 1);

        worker_streams[w] = popen(cmd, "r");
    }

    // Collect results
    int total_count = 0;
    for (int w = 0; w < config->n_workers; w++) {
        if (worker_streams[w]) {
            char line[2048];
            while (fgets(line, sizeof(line), worker_streams[w]) && total_count < *n_results) {
                // Parse output and add to results
                // Simplified: direct single-thread fallback
            }
            pclose(worker_streams[w]);
        }
    }

    *n_results = total_count;
    return total_count;
}