#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "config.h"
#include "counter.h"
#include "counter_ops.h"
#include "file_processor.h"
#include "language.h"
#include "output.h"
#include "parallel.h"
#include "threaded_counter.h"

int counter_ops_count_files(FileStats* files, int n_files, const CliArgs* args, int* error_count) {
    int n_count_files = 0;
    for (int i = 0; i < n_files; i++) {
        if (files[i].lang != NULL) n_count_files++;
    }
    if (n_count_files == 0) return 0;

    int n_workers = args->processes;
    if (n_workers == 0) {
        ParallelConfig auto_config;
        parallel_default_config(&auto_config);
        n_workers = auto_config.n_workers;
    }

    if (n_workers > 1 && n_count_files >= PARALLEL_MIN_FILES) {
        if (args->use_threads) {
            ThreadInputFile* thread_inputs = malloc(n_count_files * sizeof(ThreadInputFile));
            if (!thread_inputs) return -1;

            int idx = 0;
            for (int i = 0; i < n_files; i++) {
                if (files[i].lang != NULL) {
                    thread_inputs[idx].filepath = files[i].filepath;
                    thread_inputs[idx].lang = files[i].lang;
                    idx++;
                }
            }

            ThreadConfig thread_config;
            thread_default_config(&thread_config);
            thread_config.n_threads = n_workers;

            FileStats* thread_results = malloc(n_count_files * sizeof(FileStats));
            if (!thread_results) {
                free(thread_inputs);
                return -1;
            }

            int n_thread_results = n_count_files;
            threaded_count_files(thread_inputs, n_count_files, &thread_config,
                                 args->skip_leading_exts, args->n_skip_leading_exts,
                                 args->skip_leading, thread_results, &n_thread_results);

            for (int i = 0; i < n_thread_results; i++) {
                for (int j = 0; j < n_files; j++) {
                    if (strcmp(files[j].filepath, thread_results[i].filepath) == 0) {
                        files[j].counts = thread_results[i].counts;
                        break;
                    }
                }
            }
            free(thread_results);
            free(thread_inputs);
        } else {
            ParallelInputFile* count_paths = malloc(n_count_files * sizeof(ParallelInputFile));
            if (!count_paths) return -1;

            int idx = 0;
            for (int i = 0; i < n_files; i++) {
                if (files[i].lang != NULL) {
                    count_paths[idx].filepath = files[i].filepath;
                    count_paths[idx].lang = files[i].lang;
                    idx++;
                }
            }

            ParallelConfig par_config;
            par_config.n_workers = n_workers;
            par_config.chunk_size = PARALLEL_CHUNK_SIZE;
            par_config.timeout_sec = PARALLEL_TIMEOUT_SEC;

            ParallelResult* par_results = malloc(n_count_files * sizeof(ParallelResult));
            if (!par_results) {
                free(count_paths);
                return -1;
            }

            int n_par_results = n_count_files;
            parallel_count_files(count_paths, n_count_files, &par_config,
                                 args->skip_leading_exts, args->n_skip_leading_exts,
                                 args->skip_leading, par_results, &n_par_results);

            for (int i = 0; i < n_par_results; i++) {
                for (int j = 0; j < n_files; j++) {
                    if (strcmp(files[j].filepath, par_results[i].filepath) == 0) {
                        files[j].counts = par_results[i].counts;
                        break;
                    }
                }
            }
            free(par_results);
            free(count_paths);
        }
    } else {
        for (int i = 0; i < n_files; i++) {
            if (files[i].lang == NULL) continue;

            int skip_lines = 0;
            if (args->skip_leading > 0) {
                if (args->n_skip_leading_exts > 0) {
                    const char* ext = fp_get_extension(files[i].filepath);
                    if (fp_extension_matches(ext, args->skip_leading_exts, args->n_skip_leading_exts)) {
                        skip_lines = args->skip_leading;
                    }
                } else {
                    skip_lines = args->skip_leading;
                }
            }

            if (count_file_with_lang(files[i].filepath, files[i].lang, skip_lines,
                                     &files[i].counts) != 0) {
                fprintf(stderr, "Error: Cannot read file '%s'\n", files[i].filepath);
                files[i].counts.blank = 0;
                files[i].counts.comment = 0;
                files[i].counts.code = 0;
                files[i].counts.total = 0;
                (*error_count)++;
            }
        }
    }
    return 0;
}

void counter_ops_generate_stripped(FileStats* files, int n_files, const CliArgs* args) {
    if (!args->strip_comments && !args->strip_code) return;

    for (int i = 0; i < n_files; i++) {
        if (files[i].lang == NULL || files[i].ignore_reason != NULL) continue;

        FILE* src_fp = fopen(files[i].filepath, "r");
        if (!src_fp) continue;

        char out_path[2048];
        if (args->original_dir) {
            snprintf(out_path, sizeof(out_path), "%s.%s", files[i].filepath,
                     args->strip_comments ? args->strip_comments : args->strip_code);
        } else {
            const char* basename = strrchr(files[i].filepath, '/');
            basename = basename ? basename + 1 : files[i].filepath;
            snprintf(out_path, sizeof(out_path), "%s.%s", basename,
                     args->strip_comments ? args->strip_comments : args->strip_code);
        }

        FILE* out_fp = fopen(out_path, "w");
        if (!out_fp) {
            fclose(src_fp);
            continue;
        }

        char line[4096];
        while (fgets(line, sizeof(line), src_fp) != NULL) {
            size_t len = strlen(line);
            while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
                line[len - 1] = '\0';
                len--;
            }

            int is_blank = 1;
            for (size_t j = 0; j < len; j++) {
                if ((unsigned char)line[j] > 127 || !isspace((unsigned char)line[j])) {
                    is_blank = 0;
                    break;
                }
            }

            int is_comment = 0;
            if (files[i].lang && files[i].lang->generic_filters) {
                for (size_t f = 0; f < files[i].lang->generic_filter_count; f++) {
                    const GenericFilter* gf = &files[i].lang->generic_filters[f];
                    if (gf->type == FILTER_REMOVE_INLINE) {
                        size_t plen = strlen(gf->pattern_open);
                        const char* trimmed = line;
                        while (*trimmed && isspace((unsigned char)*trimmed)) trimmed++;
                        if (strncmp(trimmed, gf->pattern_open, plen) == 0) {
                            is_comment = 1;
                            break;
                        }
                    }
                }
            }

            if (args->strip_comments) {
                if (!is_blank && !is_comment) fprintf(out_fp, "%s\n", line);
            } else if (args->strip_code) {
                if (is_blank || is_comment) fprintf(out_fp, "%s\n", line);
            }
        }
        fclose(src_fp);
        fclose(out_fp);
    }
}
