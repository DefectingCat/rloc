#ifndef FILELIST_H
#define FILELIST_H

typedef struct {
    int no_recurse;
    long max_file_size;
    char** exclude_dirs;
    int n_exclude_dirs;
} FilelistConfig;

typedef struct {
    char** paths;
    int count;
    int capacity;
} FileList;

void filelist_init(FileList* list);
void filelist_free(FileList* list);
int filelist_scan(const char* path, const FilelistConfig* config, FileList* list);

#endif
