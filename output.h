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

// Alignment type for file pairs across two references
typedef enum {
    ALIGN_ADDED,      // File exists only in ref2 (new file)
    ALIGN_REMOVED,    // File exists only in ref1 (deleted file)
    ALIGN_MODIFIED,   // File exists in both refs, content changed
    ALIGN_IDENTICAL   // File exists in both refs, unchanged
} AlignmentType;

// A single entry in the alignment table (already-paired file across two refs)
typedef struct {
    const char* filepath;      // File path, or "(added)" / "(removed)" for unpaired
    AlignmentType type;        // Classification of this entry
    int added;                 // Lines added in ref2 vs ref1 (0 if removed/identical)
    int removed;               // Lines removed from ref1 vs ref2 (0 if added/identical)
    const char* language;      // Language name, or "(identical)" / "(none)" for special cases
} AlignmentEntry;

// Output file alignment table (cloc --diff-alignment compatible)
// Text or JSON format based on json_output flag
void output_alignment(const AlignmentEntry* entries, int n_entries,
                      const char* ref1, const char* ref2, int json_output);

// Output file alignment table in JSON format (called internally by output_alignment)
void output_alignment_json(const AlignmentEntry* entries, int n_entries,
                           const char* ref1, const char* ref2);

// Output diff results (added/removed lines between commits)
void output_diff(const DiffFileStats* files, int n_files, const char* commit1, const char* commit2,
                 int by_file);

#endif