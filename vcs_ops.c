#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "diff.h"
#include "filelist.h"
#include "output.h"
#include "temp_manager.h"
#include "util.h"
#include "vcs.h"
#include "vcs_ops.h"

void vcs_ops_init(VcsOpsContext* ctx, const CliArgs* args, TempManager* temp_mgr,
                  FileList* filelist) {
    ctx->args = args;
    ctx->temp_mgr = temp_mgr;
    ctx->filelist = filelist;
    ctx->error_count = 0;
}

// Check if path contains any excluded directory segment
static int path_contains_excluded_dir(const char* path, const CliArgs* args) {
    if (!args->exclude_dirs || args->n_exclude_dirs == 0) return 0;

    // Copy path to avoid modifying the original
    char path_copy[2048];
    snprintf(path_copy, sizeof(path_copy), "%s", path);

    // Check each path segment
    char* saveptr = NULL;
    char* segment = strtok_r(path_copy, "/", &saveptr);
    while (segment != NULL) {
        for (int i = 0; i < args->n_exclude_dirs; i++) {
            if (strcmp(segment, args->exclude_dirs[i]) == 0) {
                return 1;
            }
        }
        segment = strtok_r(NULL, "/", &saveptr);
    }
    return 0;
}

int vcs_ops_handle_git_ref(VcsOpsContext* ctx) {
    const CliArgs* args = ctx->args;
    FileList* filelist = ctx->filelist;
    TempManager* temp_mgr = ctx->temp_mgr;

    if (args->git_ref == NULL) return 0;

    const char* repo_path = (args->n_input_files > 0) ? args->input_files[0] : ".";
    if (!is_directory(repo_path)) repo_path = ".";

    int n_git_files = 0;
    char** git_files = vcs_get_files_at_commit(repo_path, args->git_ref, &n_git_files);
    if (!git_files || n_git_files == 0) {
        fprintf(stderr, "Error: Cannot get files at git ref '%s'\n", args->git_ref);
        return -1;
    }

    char* git_extract_dir = temp_manager_create_dir(temp_mgr, "git_extract");
    if (!git_extract_dir) {
        fprintf(stderr, "Error: Cannot create temp directory for git extraction\n");
        vcs_free_files(git_files, n_git_files);
        return -1;
    }

    for (int j = 0; j < n_git_files; j++) {
        size_t content_len = 0;
        char* content = vcs_get_file_at_commit(repo_path, args->git_ref, git_files[j], &content_len);
        if (content && content_len > 0) {
            char temp_path[2048];
            snprintf(temp_path, sizeof(temp_path), "%s/%s", git_extract_dir, git_files[j]);

            char* last_slash = strrchr(temp_path, '/');
            if (last_slash && last_slash != temp_path) {
                char dir_path[2048];
                strncpy(dir_path, temp_path, last_slash - temp_path);
                dir_path[last_slash - temp_path] = '\0';
                mkdir_p(dir_path, 0755);
            }

            FILE* fp = fopen(temp_path, "w");
            if (fp) {
                fwrite(content, 1, content_len, fp);
                fclose(fp);

                filelist_append(filelist, temp_path);
            }
            free(content);
        }
    }
    vcs_free_files(git_files, n_git_files);
    return 0;
}

VcsType vcs_ops_detect_effective(const CliArgs* args) {
    VcsType effective_vcs = args->vcs;
    if (args->vcs == VCS_AUTO) {
        for (int i = 0; i < args->n_input_files; i++) {
            const char* path = args->input_files[i];
            if (is_directory(path)) {
                effective_vcs = vcs_detect(path);
                if (effective_vcs != VCS_NONE) break;
            }
        }
    }
    return effective_vcs;
}

int vcs_ops_handle_vcs(VcsOpsContext* ctx) {
    const CliArgs* args = ctx->args;
    FileList* filelist = ctx->filelist;
    VcsType effective_vcs = vcs_ops_detect_effective(args);

    if (effective_vcs != VCS_GIT && effective_vcs != VCS_SVN) return 0;

    for (int i = 0; i < args->n_input_files; i++) {
        const char* path = args->input_files[i];
        if (is_directory(path)) {
            int n_vcs_files = 0;
            char** vcs_files = NULL;
            if (effective_vcs == VCS_GIT)
                vcs_files = vcs_get_files_git_ex(path, args->include_submodules, &n_vcs_files);
            else if (effective_vcs == VCS_SVN)
                vcs_files = vcs_get_files_svn(path, &n_vcs_files);

            if (!vcs_files || n_vcs_files == 0) {
                fprintf(stderr, "Error: Cannot get %s files from '%s'\n",
                        effective_vcs == VCS_GIT ? "git" : "svn", path);
                ctx->error_count++;
                continue;
            }

            for (int j = 0; j < n_vcs_files; j++) {
                // Check if path is in excluded directory
                if (path_contains_excluded_dir(vcs_files[j], args)) {
                    continue;
                }

                char full[2048];
                snprintf(full, sizeof(full), "%s/%s", path, vcs_files[j]);

                // Skip submodule directories (git ls-files returns submodule path as entry)
                // A submodule has .git as a file (not directory) pointing to parent's .git/modules/
                if (is_directory(full)) {
                    continue;
                }

                filelist_append(filelist, full);
            }
            vcs_free_files(vcs_files, n_vcs_files);
        } else {
            filelist_append(filelist, path);
        }
    }
    return ctx->error_count > 0 ? -1 : 0;
}

int vcs_ops_is_diff_mode(const CliArgs* args) {
    return (args->diff_refs != NULL || (args->diff_commit1 != NULL && args->diff_commit2 != NULL));
}

int vcs_ops_handle_diff(VcsOpsContext* ctx) {
    const CliArgs* args = ctx->args;

    if (!vcs_ops_is_diff_mode(args)) return 0;

    const char* repo_path = (args->n_input_files > 0) ? args->input_files[0] : ".";
    if (!is_directory(repo_path)) repo_path = ".";

    DiffConfig diff_cfg;
    memset(&diff_cfg, 0, sizeof(diff_cfg));
    diff_cfg.repo_path = repo_path;

    if (args->diff_refs != NULL) {
        diff_cfg.ref1 = args->diff_commit1;
        diff_cfg.ref2 = args->diff_commit2;
        diff_cfg.flags = args->diff_flags;
    } else {
        diff_cfg.ref1 = args->diff_commit1;
        diff_cfg.ref2 = args->diff_commit2;
        diff_cfg.flags = DIFF_MODE_RELATIVE;
    }

    int n_diff_files = 0;
    DiffFileStats* diff_files = diff_get_stats_extended(&diff_cfg, &n_diff_files);

    if (!diff_files || n_diff_files == 0) {
        printf("No changes between %s and %s\n", diff_cfg.ref1, diff_cfg.ref2);
        diff_free_config(&diff_cfg);
        return 0;
    }

    if (diff_cfg.flags & DIFF_SHOW_ALIGNMENT) {
        AlignmentEntry* entries = malloc(n_diff_files * sizeof(AlignmentEntry));
        if (entries) {
            for (int i = 0; i < n_diff_files; i++) {
                entries[i].filepath = diff_files[i].filepath;
                entries[i].type =
                    (diff_files[i].added > 0 && diff_files[i].removed == 0)   ? ALIGN_ADDED
                    : (diff_files[i].removed > 0 && diff_files[i].added == 0) ? ALIGN_REMOVED
                                                                              : ALIGN_MODIFIED;
                entries[i].added = diff_files[i].added;
                entries[i].removed = diff_files[i].removed;
                entries[i].language = diff_files[i].lang ? diff_files[i].lang : "(unknown)";
            }
            int json_output = (args->output_format == FORMAT_JSON);
            output_alignment(entries, n_diff_files, diff_cfg.ref1, diff_cfg.ref2, json_output);
            free(entries);
        }
    } else {
        output_diff(diff_files, n_diff_files, diff_cfg.ref1, diff_cfg.ref2, args->by_file);
    }

    diff_free_files(diff_files, n_diff_files);
    diff_free_config(&diff_cfg);
    return 0;
}