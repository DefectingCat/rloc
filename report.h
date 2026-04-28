#ifndef REPORT_H
#define REPORT_H

#include "output.h"

/* Write unique file list (files that were counted). */
int report_write_unique_file(const FileStats* files, int n_files, const char* filepath);

/* Write ignored file list with reasons. */
int report_write_ignored_file(const FileStats* files, int n_files, const char* filepath);

/* Write found file list (all discovered files). */
int report_write_found_file(const FileStats* files, int n_files, const char* filepath);

/* Write counted file list (files with detected language). */
int report_write_counted_file(const FileStats* files, int n_files, const char* filepath);

/* Write categorized file list with size, language, and category. */
int report_write_categorized_file(const FileStats* files, int n_files, const char* filepath);

#endif /* REPORT_H */
