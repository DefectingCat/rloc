#ifndef FILELIST_H
#define FILELIST_H

typedef struct {
    int no_recurse;
    long max_file_size;
    char** exclude_dirs;
    int n_exclude_dirs;
    char** exclude_patterns;  // Glob patterns from --exclude-list-file
    int n_exclude_patterns;
    char* match_pattern;        // Regex pattern for --match-f
    char* not_match_pattern;    // Regex pattern for --not-match-f
    char* match_d_pattern;      // Regex pattern for --match-d
    char* not_match_d_pattern;  // Regex pattern for --not-match-d
    int fullpath;               // Match full path instead of basename
    int follow_links;           // Follow symlinks to directories
} FilelistConfig;

typedef struct {
    char** paths;
    int count;
    int capacity;
} FileList;

void filelist_init(FileList* list);
void filelist_free(FileList* list);
int filelist_scan(const char* path, const FilelistConfig* config, FileList* list);

// Add file path to list (public API)
void filelist_append(FileList* list, const char* path);

// Load exclude patterns from file (one pattern per line, # for comments)
int filelist_load_exclude_patterns(const char* filepath, FilelistConfig* config);

// Free exclude patterns loaded by filelist_load_exclude_patterns
void filelist_free_exclude_patterns(FilelistConfig* config);

#endif
