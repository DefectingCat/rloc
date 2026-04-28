#ifndef COUNTER_OPS_H
#define COUNTER_OPS_H

#include "cli.h"
#include "output.h"

/* Count files using parallel or sequential processing.
 * Returns 0 on success, -1 on memory allocation failure.
 * Increments *error_count on file read errors. */
int counter_ops_count_files(FileStats* files, int n_files, const CliArgs* args, int* error_count);

/* Generate stripped files (--strip-comments or --strip-code).
 * Writes stripped output files based on args settings. */
void counter_ops_generate_stripped(FileStats* files, int n_files, const CliArgs* args);

#endif /* COUNTER_OPS_H */
