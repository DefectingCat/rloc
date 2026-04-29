#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"
#include "coro_scanner.h"
#include "filelist.h"
#include "util.h"
#include "vcs_ops.h"

int scanner_scan_input_paths(const CliArgs* args, FilelistConfig* config, FileList* filelist,
                              TempManager* temp_mgr, int* error_count) {
    VcsType effective_vcs = vcs_ops_detect_effective(args);

    if (effective_vcs == VCS_GIT || effective_vcs == VCS_SVN) {
        VcsOpsContext vcs_ctx;
        vcs_ops_init(&vcs_ctx, args, temp_mgr, filelist);
        return vcs_ops_handle_vcs(&vcs_ctx);
    }

    for (int i = 0; i < args->n_input_files; i++) {
        const char* path = args->input_files[i];
        if (is_directory(path)) {
            if (args->use_coro) {
                if (coro_scan_directory(path, config, filelist) != 0) {
                    fprintf(stderr, "Error: Cannot scan directory '%s' (coro mode)\n", path);
                    (*error_count)++;
                }
            } else if (filelist_scan(path, config, filelist) != 0) {
                fprintf(stderr, "Error: Cannot scan directory '%s'\n", path);
                (*error_count)++;
            }
        } else if (is_regular_file(path)) {
            if (archive_is_archive(path)) {
                char* extract_dir = archive_extract(path, temp_mgr);
                if (extract_dir) {
                    if (filelist_scan(extract_dir, config, filelist) != 0) {
                        fprintf(stderr, "Warning: Cannot scan extracted archive '%s'\n", extract_dir);
                    }
                } else {
                    fprintf(stderr, "Warning: Cannot extract archive '%s'\n", path);
                }
            } else {
                filelist_append(filelist, path);
            }
        } else {
            fprintf(stderr, "Error: Path '%s' is not a valid file or directory\n", path);
            (*error_count)++;
        }
    }
    return 0;
}
