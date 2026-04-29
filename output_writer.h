#ifndef OUTPUT_WRITER_H
#define OUTPUT_WRITER_H

#include <stdio.h>

#include "cli.h"
#include "counter_interface.h"
#include "language.h"

/* FileStats structure for output */
typedef struct {
    const char* filepath;
    const Language* lang;
    CountResult counts;
    const char* ignore_reason;
} OutputFileStats;

/* Output Writer interface - abstract interface for output implementations */
typedef struct OutputWriter {
    /* Write header (called before results) */
    void (*write_header)(struct OutputWriter* self);

    /* Write a single result */
    void (*write_result)(struct OutputWriter* self, const OutputFileStats* stats);

    /* Write footer/summary (called after all results) */
    void (*write_footer)(struct OutputWriter* self, double elapsed_sec);

    /* Free writer resources */
    void (*free)(struct OutputWriter* self);

    /* Implementation-specific data */
    void* data;

    /* Common fields */
    int format;  /* Uses OutputFormat from cli.h */
    int by_file;
    int by_file_by_lang;
    FILE* out;
} OutputWriter;

/* Create output writer for specified format */
OutputWriter* output_writer_new(int format, int by_file, int by_file_by_lang);

#endif
