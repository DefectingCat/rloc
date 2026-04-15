#ifndef OUTPUT_H
#define OUTPUT_H
#include "counter.h"
#include "diff.h"
#include "language.h"

typedef struct {
    const char* filepath;
    const Language* lang;  // Language pointer (was: const char *language)
    CountResult counts;
    const char* ignore_reason;  // NULL if counted, otherwise reason
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

// Output results in YAML format
void output_yaml(const FileStats* files, int n_files, double elapsed_sec);

// Output results in YAML format with per-file breakdown
void output_yaml_by_file(const FileStats* files, int n_files, double elapsed_sec);

// Output results in XML format
void output_xml(const FileStats* files, int n_files, double elapsed_sec);

// Output results in XML format with per-file breakdown
void output_xml_by_file(const FileStats* files, int n_files, double elapsed_sec);

// Output results in HTML format
void output_html(const FileStats* files, int n_files, double elapsed_sec);

// Output results in HTML format with per-file breakdown
void output_html_by_file(const FileStats* files, int n_files, double elapsed_sec);

// Output results in SQL format (SQLite-compatible)
void output_sql(const FileStats* files, int n_files, double elapsed_sec, const char* project);

// Output results in SQL format with per-file breakdown
void output_sql_by_file(const FileStats* files, int n_files, double elapsed_sec, const char* project);

// Output results by file and language combined
void output_by_file_by_lang(const FileStats* files, int n_files, double elapsed_sec);

// Output diff results (added/removed lines between commits)
void output_diff(const DiffFileStats* files, int n_files, const char* commit1, const char* commit2,
                 int by_file);

#endif