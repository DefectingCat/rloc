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

// Output results as text table with per-file breakdown
void output_text_by_file(const FileStats* files, int n_files, double elapsed_sec);

// Output results in JSON format
void output_json(const FileStats* files, int n_files, double elapsed_sec);

// Output results in JSON format with per-file breakdown
void output_json_by_file(const FileStats* files, int n_files, double elapsed_sec);

// Output results in CSV format
void output_csv(const FileStats* files, int n_files, double elapsed_sec);

// Output results in CSV format with per-file breakdown
void output_csv_by_file(const FileStats* files, int n_files, double elapsed_sec);

// Output results in Markdown format
void output_md(const FileStats* files, int n_files, double elapsed_sec);

// Output results in Markdown format with per-file breakdown
void output_md_by_file(const FileStats* files, int n_files, double elapsed_sec);

#endif