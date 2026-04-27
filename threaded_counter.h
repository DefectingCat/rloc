/**
 * Threaded file counter - parallel counting using multiple threads
 */

#ifndef THREADED_COUNTER_H
#define THREADED_COUNTER_H

#include "counter.h"
#include "language.h"
#include "output.h"

/* Input file with pre-detected language */
typedef struct {
    const char* filepath;
    const Language* lang;
} ThreadInputFile;

/* Thread configuration */
typedef struct {
    int n_threads;
    int threshold;
    int max_languages;
} ThreadConfig;

/* Get default thread configuration */
void thread_default_config(ThreadConfig* config);

/* Count files in parallel using threads
 * files: array of ThreadInputFile with pre-detected languages
 * Returns: number of successfully counted files, -1 on severe error
 */
int threaded_count_files(ThreadInputFile* files, int n_files, ThreadConfig* config,
                         char** skip_leading_exts, int n_skip_leading_exts, int skip_leading,
                         FileStats* results, int* n_results);

#endif /* THREADED_COUNTER_H */