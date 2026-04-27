/**
 * Threaded file counter implementation
 */

#include "threaded_counter.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "counter.h"
#include "language.h"
#include "thread_compat.h"

#define DEFAULT_THRESHOLD 50
#define DEFAULT_MAX_LANGUAGES 50

void thread_default_config(ThreadConfig* config) {
    if (!config) return;
    config->n_threads = rloc_get_cpu_count();
    if (config->n_threads < 1) config->n_threads = 1;
    if (config->n_threads > 16) config->n_threads = 16;
    config->threshold = DEFAULT_THRESHOLD;
    config->max_languages = DEFAULT_MAX_LANGUAGES;
}

/* Per-file result stored by worker */
typedef struct {
    const char* filepath;
    const Language* lang;
    CountResult counts;
    int valid;
} ThreadFileResult;

/* Thread worker arguments */
typedef struct {
    ThreadInputFile* files;
    int start;
    int end;
    char** skip_leading_exts;
    int n_skip_leading_exts;
    int skip_leading;
    ThreadFileResult* results;
    int* n_results;
    int* error_count;
} ThreadWorkerArgs;

/* Worker thread function */
static void* worker_thread_func(void* arg) {
    ThreadWorkerArgs* args = (ThreadWorkerArgs*)arg;
    int count = 0;

    for (int i = args->start; i < args->end; i++) {
        const char* filepath = args->files[i].filepath;
        const Language* lang = args->files[i].lang;
        if (!lang) continue;

        /* Calculate skip_lines - match main.c logic */
        int skip_lines = 0;
        if (args->skip_leading > 0) {
            if (args->n_skip_leading_exts > 0) {
                const char* ext = strrchr(filepath, '.');
                if (ext && ext != filepath)
                    ext++;
                else
                    ext = NULL;
                for (int j = 0; j < args->n_skip_leading_exts; j++) {
                    const char* p = args->skip_leading_exts[j];
                    if (p[0] == '.') p++;
                    if (ext && strcasecmp(ext, p) == 0) {
                        skip_lines = args->skip_leading;
                        break;
                    }
                }
            } else {
                /* No extension filter means apply to all files */
                skip_lines = args->skip_leading;
            }
        }

        /* Count file */
        CountResult counts;
        int ret = count_file_with_lang(filepath, lang, skip_lines, &counts);
        if (ret != 0) {
            rloc_atomic_increment(args->error_count);
            continue;
        }

        /* Store result */
        args->results[count].filepath = filepath;
        args->results[count].lang = lang;
        args->results[count].counts = counts;
        args->results[count].valid = 1;
        count++;
    }

    *args->n_results = count;
    return NULL;
}

int threaded_count_files(ThreadInputFile* files, int n_files, ThreadConfig* config,
                         char** skip_leading_exts, int n_skip_leading_exts, int skip_leading,
                         FileStats* results, int* n_results) {
    if (!files || n_files <= 0 || !config || !results) {
        if (n_results) *n_results = 0;
        return -1;
    }

    /* Fallback to sequential for small file sets */
    if (n_files < config->threshold || config->n_threads <= 1) {
        int count = 0;
        for (int i = 0; i < n_files; i++) {
            if (!files[i].lang) continue;

            int skip = 0;
            if (skip_leading > 0) {
                if (n_skip_leading_exts > 0) {
                    const char* ext = strrchr(files[i].filepath, '.');
                    if (ext && ext != files[i].filepath)
                        ext++;
                    else
                        ext = NULL;
                    for (int j = 0; j < n_skip_leading_exts; j++) {
                        const char* p = skip_leading_exts[j];
                        if (p[0] == '.') p++;
                        if (ext && strcasecmp(ext, p) == 0) {
                            skip = skip_leading;
                            break;
                        }
                    }
                } else {
                    skip = skip_leading;
                }
            }

            if (count_file_with_lang(files[i].filepath, files[i].lang, skip,
                                     &results[count].counts) == 0) {
                results[count].filepath = files[i].filepath;
                results[count].lang = files[i].lang;
                results[count].ignore_reason = NULL;
                count++;
            }
        }
        *n_results = count;
        return count;
    }

    /* Allocate thread resources */
    int n_workers = config->n_threads;
    rloc_thread_t* threads = malloc(n_workers * sizeof(rloc_thread_t));
    ThreadWorkerArgs* args = malloc(n_workers * sizeof(ThreadWorkerArgs));
    ThreadFileResult** worker_results = malloc(n_workers * sizeof(ThreadFileResult*));
    int* worker_n_results = malloc(n_workers * sizeof(int));
    int error_count = 0;

    if (!threads || !args || !worker_results || !worker_n_results) {
        free(threads);
        free(args);
        free(worker_results);
        free(worker_n_results);
        *n_results = 0;
        return -1;
    }

    /* Calculate chunk sizes */
    int chunk_size = (n_files + n_workers - 1) / n_workers;

    /* Initialize per-thread result buffers */
    for (int i = 0; i < n_workers; i++) {
        int my_chunk = chunk_size;
        int start = i * chunk_size;
        if (start + my_chunk > n_files) my_chunk = n_files - start;
        if (my_chunk < 0) my_chunk = 0;

        worker_results[i] = malloc(my_chunk * sizeof(ThreadFileResult));
        worker_n_results[i] = 0;

        if (!worker_results[i]) {
            for (int j = 0; j < i; j++) free(worker_results[j]);
            free(threads);
            free(args);
            free(worker_results);
            free(worker_n_results);
            *n_results = 0;
            return -1;
        }
    }

    /* Create threads */
    int active_threads = 0;
    for (int i = 0; i < n_workers; i++) {
        int start = i * chunk_size;
        int end = start + chunk_size;
        if (end > n_files) end = n_files;
        if (start >= n_files) break;

        args[i].files = files;
        args[i].start = start;
        args[i].end = end;
        args[i].skip_leading_exts = skip_leading_exts;
        args[i].n_skip_leading_exts = n_skip_leading_exts;
        args[i].skip_leading = skip_leading;
        args[i].results = worker_results[i];
        args[i].n_results = &worker_n_results[i];
        args[i].error_count = &error_count;

        if (rloc_thread_create(&threads[i], worker_thread_func, &args[i]) != 0) {
            continue;
        }
        active_threads++;
    }

    if (active_threads == 0) {
        for (int i = 0; i < n_workers; i++) free(worker_results[i]);
        free(threads);
        free(args);
        free(worker_results);
        free(worker_n_results);
        *n_results = 0;
        return -1;
    }

    /* Wait for all threads */
    for (int i = 0; i < active_threads; i++) {
        rloc_thread_join(threads[i]);
    }

    /* Merge results */
    int total_count = 0;
    for (int w = 0; w < n_workers; w++) {
        for (int i = 0; i < worker_n_results[w] && total_count < *n_results; i++) {
            ThreadFileResult* wr = &worker_results[w][i];
            results[total_count].filepath = wr->filepath;
            results[total_count].lang = wr->lang;
            results[total_count].counts = wr->counts;
            results[total_count].ignore_reason = NULL;
            total_count++;
        }
    }

    *n_results = total_count;

    /* Cleanup */
    for (int i = 0; i < n_workers; i++) free(worker_results[i]);
    free(threads);
    free(args);
    free(worker_results);
    free(worker_n_results);

    return total_count;
}