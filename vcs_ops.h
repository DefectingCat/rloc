#ifndef VCS_OPS_H
#define VCS_OPS_H

#include "cli.h"
#include "filelist.h"
#include "output.h"
#include "temp_manager.h"
#include "vcs.h"

/* Context for VCS operations */
typedef struct {
    const CliArgs* args;
    TempManager* temp_mgr;
    FileList* filelist;
    int error_count;
} VcsOpsContext;

/* Initialize VCS operations context. */
void vcs_ops_init(VcsOpsContext* ctx, const CliArgs* args, TempManager* temp_mgr,
                  FileList* filelist);

/* Handle --git mode: extract files at specific commit/branch/tag.
 * Returns 0 on success, non-zero on error.
 * Files are extracted to temp directory and added to filelist. */
int vcs_ops_handle_git_ref(VcsOpsContext* ctx);

/* Handle --vcs mode: get files from git/svn repository.
 * Uses vcs_get_files_git or vcs_get_files_svn.
 * Returns 0 on success, non-zero on error. */
int vcs_ops_handle_vcs(VcsOpsContext* ctx);

/* Detect effective VCS type (resolves VCS_AUTO). */
VcsType vcs_ops_detect_effective(const CliArgs* args);

/* Check if diff mode is active (--diff-refs or --diff-commit1/2). */
int vcs_ops_is_diff_mode(const CliArgs* args);

/* Handle diff mode: get changed files between commits.
 * Returns 0 on success, non-zero on error.
 * Outputs results directly via output_diff/output_alignment. */
int vcs_ops_handle_diff(VcsOpsContext* ctx);

#endif /* VCS_OPS_H */
