#ifndef OUTPUT_H
#define OUTPUT_H
#include "counter.h"
#include "language.h"

typedef struct {
    const char* filepath;
    const Language* lang;  // Language pointer (was: const char *language)
    CountResult counts;
} FileStats;

// Output results as text table (default cloc-like format)
// elapsed_sec: time taken for processing
void output_text(const FileStats* files, int n_files, double elapsed_sec);

#endif