#include "parallel.h"

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <unistd.h>

#include "counter.h"
#include "language.h"

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
}

// Worker process function - counts files and writes results to pipe
static void worker_process(ParallelInputFile* files, int start, int end, int write_fd,
                           char** skip_leading_exts, int n_skip_leading_exts, int skip_leading) {
    FILE* out = fdopen(write_fd, "w");
    if (!out) {
        close(write_fd);
        return;
    }

    for (int i = start; i < end; i++) {
        const char* filepath = files[i].filepath;
        const Language* lang = files[i].lang;
        if (!lang) continue;

        // Determine skip_lines
        int skip_lines = 0;
        if (skip_leading > 0) {
            if (n_skip_leading_exts > 0) {
                const char* ext = strrchr(filepath, '.');
                if (ext) ext++;
                for (int j = 0; j < n_skip_leading_exts; j++) {
                    const char* p = skip_leading_exts[j];
                    if (p[0] == '.') p++;
                    if (strcasecmp(ext ? ext : "", p) == 0) {
                        skip_lines = skip_leading;
                        break;
                    }
                }
            } else {
                skip_lines = skip_leading;
            }
        }

        CountResult result;
        if (count_file_with_lang(filepath, lang, skip_lines, &result) == 0) {
            // Output TSV: filepath\tlanguage\tblank\tcomment\tcode
            fprintf(out, "%s\t%s\t%d\t%d\t%d\n", filepath, lang->name, result.blank, result.comment,
                    result.code);
        }
    }

    fclose(out);
}

// Count files in parallel using fork
int parallel_count_files(ParallelInputFile* files, int n_files, ParallelConfig* config,
                         char** skip_leading_exts, int n_skip_leading_exts, int skip_leading,
                         ParallelResult* results, int* n_results) {
    if (!files || n_files <= 0 || !config || !results) {
        *n_results = 0;
        return -1;
    }

    // For small file counts or single worker, don't parallelize
    if (n_files < 50 || config->n_workers <= 1) {
        // Single-threaded fallback
        int count = 0;
        for (int i = 0; i < n_files; i++) {
            const Language* lang = files[i].lang;
            if (!lang) continue;

            int skip_lines_worker = 0;
            if (skip_leading > 0) {
                if (n_skip_leading_exts > 0) {
                    const char* ext = strrchr(files[i].filepath, '.');
                    if (ext) ext++;
                    for (int j = 0; j < n_skip_leading_exts; j++) {
                        const char* p = skip_leading_exts[j];
                        if (p[0] == '.') p++;
                        if (strcasecmp(ext ? ext : "", p) == 0) {
                            skip_lines_worker = skip_leading;
                            break;
                        }
                    }
                } else {
                    skip_lines_worker = skip_leading;
                }
            }

            if (count_file_with_lang(files[i].filepath, lang, skip_lines_worker,
                                     &results[count].counts) == 0) {
                strncpy(results[count].filepath, files[i].filepath,
                        sizeof(results[count].filepath) - 1);
                results[count].language = lang;
                count++;
            }
        }
        *n_results = count;
        return count;
    }

    // Calculate chunk sizes
    int chunk_size = (n_files + config->n_workers - 1) / config->n_workers;
    if (chunk_size < 10) chunk_size = 10;

    // Allocate pipes and pids dynamically (avoid VLA for stack safety)
    int (*pipes)[2] = malloc(config->n_workers * sizeof(int[2]));
    pid_t* pids = malloc(config->n_workers * sizeof(pid_t));
    if (!pipes || !pids) {
        free(pipes);
        free(pids);
        *n_results = 0;
        return -1;
    }

    // Block SIGCHLD temporarily
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    // Fork workers
    int active_workers = 0;
    for (int w = 0; w < config->n_workers; w++) {
        int start = w * chunk_size;
        int end = start + chunk_size;
        if (end > n_files) end = n_files;
        if (start >= end) break;

        if (pipe(pipes[w]) < 0) {
            // Pipe creation failed, skip this worker
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            // Fork failed, close pipes
            close(pipes[w][0]);
            close(pipes[w][1]);
            continue;
        } else if (pid == 0) {
            // Child process
            close(pipes[w][0]);  // Close read end
            worker_process(files, start, end, pipes[w][1], skip_leading_exts, n_skip_leading_exts,
                           skip_leading);
            exit(0);
        } else {
            // Parent process
            close(pipes[w][1]);  // Close write end
            pids[w] = pid;
            active_workers++;
        }
    }

    // Read results from all workers
    int total_count = 0;
    char line[2048];

    for (int w = 0; w < active_workers; w++) {
        FILE* in = fdopen(pipes[w][0], "r");
        if (!in) {
            close(pipes[w][0]);
            continue;
        }

        while (fgets(line, sizeof(line), in) && total_count < *n_results) {
            // Parse TSV: filepath\tlanguage\tblank\tcomment\tcode
            char* saveptr;
            char* filepath = strtok_r(line, "\t", &saveptr);
            if (!filepath) continue;

            char* lang_name = strtok_r(NULL, "\t", &saveptr);
            char* blank_str = strtok_r(NULL, "\t", &saveptr);
            char* comment_str = strtok_r(NULL, "\t", &saveptr);
            char* code_str = strtok_r(NULL, "\t\n", &saveptr);

            if (!filepath || !lang_name || !blank_str || !comment_str || !code_str) continue;

            strncpy(results[total_count].filepath, filepath,
                    sizeof(results[total_count].filepath) - 1);
            results[total_count].filepath[sizeof(results[total_count].filepath) - 1] = '\0';
            results[total_count].counts.blank = atoi(blank_str);
            results[total_count].counts.comment = atoi(comment_str);
            results[total_count].counts.code = atoi(code_str);
            results[total_count].counts.total = results[total_count].counts.blank +
                                                results[total_count].counts.comment +
                                                results[total_count].counts.code;

            // Find language by name
            results[total_count].language = get_language_by_name(lang_name);
            total_count++;
        }

        fclose(in);
    }

    // Wait for all workers to complete
    for (int w = 0; w < active_workers; w++) {
        waitpid(pids[w], NULL, 0);
    }

    // Restore signal mask
    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    // Free dynamically allocated arrays
    free(pipes);
    free(pids);

    *n_results = total_count;
    return total_count;
}