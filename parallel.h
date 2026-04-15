#ifndef PARALLEL_H
#define PARALLEL_H

#include "counter.h"
#include "temp_manager.h"

// Worker process result (TSV format)
// filepath\tlanguage\tblank\tcomment\tcode
typedef struct {
    char filepath[1024];
    const char* language;
    int blank;
    int comment;
    int code;
} WorkerResult;

// Parallel count configuration
typedef struct {
    int n_workers;          // Number of worker processes (default: CPU count)
    size_t chunk_size;      // Files per chunk (default: 100)
    int timeout_sec;        // Worker timeout in seconds
    TempManager* temp_mgr;  // Temp manager for worker output files
} ParallelConfig;

// Count files in parallel using multiple worker processes
// Returns aggregate results in provided CountResult array
// Returns number of files counted, -1 on error
int parallel_count_files(char** files, int n_files, ParallelConfig* config,
                         CountResult* results, int* n_results);

// Get default parallel configuration (CPU count)
void parallel_default_config(ParallelConfig* config);

// Parse TSV output from worker process
// Returns number of results parsed, -1 on error
int parse_worker_tsv(const char* tsv_path, WorkerResult* results, int max_results);

#endif