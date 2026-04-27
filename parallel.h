#ifndef PARALLEL_H
#define PARALLEL_H

#include "counter.h"
#include "language.h"

// Input file with pre-detected language
typedef struct {
    const char* filepath;
    const Language* lang;
} ParallelInputFile;

// Result from parallel counting
typedef struct {
    char filepath[1024];
    const Language* language;
    CountResult counts;
} ParallelResult;

// Parallel count configuration
typedef struct {
    int n_workers;      // Number of worker processes (default: CPU count)
    size_t chunk_size;  // Files per chunk (default: 100)
    int timeout_sec;    // Worker timeout in seconds
} ParallelConfig;

// Get default parallel configuration (CPU count)
void parallel_default_config(ParallelConfig* config);

// Count files in parallel using multiple worker processes
// Returns number of files counted, -1 on error
int parallel_count_files(ParallelInputFile* files, int n_files, ParallelConfig* config,
                         char** skip_leading_exts, int n_skip_leading_exts, int skip_leading,
                         ParallelResult* results, int* n_results);

#endif