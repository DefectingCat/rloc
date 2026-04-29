#ifndef SCANNER_H
#define SCANNER_H

#include "cli.h"
#include "filelist.h"
#include "temp_manager.h"

/* Scan input paths and populate filelist */
int scanner_scan_input_paths(const CliArgs* args, FilelistConfig* config, FileList* filelist,
                              TempManager* temp_mgr, int* error_count);

#endif
