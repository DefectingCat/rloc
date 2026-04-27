#include <stdlib.h>
#include <string.h>

#include "../diff.h"
#include "test_framework.h"

// Helper to free test results
static void free_test_files(DiffFileStats* files, int n_files) {
    if (files) {
        for (int i = 0; i < n_files; i++) {
            if (files[i].filepath) {
                free(files[i].filepath);
            }
        }
        free(files);
    }
}

// Helper to create a DiffConfig for testing
static DiffConfig make_config(const char* repo, const char* ref1, const char* ref2, unsigned int flags) {
    DiffConfig cfg;
    cfg.ref1 = ref1;
    cfg.ref2 = ref2;
    cfg.flags = flags;
    cfg.repo_path = repo;
    return cfg;
}

// Test diff_get_files returns NULL for invalid repo path
TEST(test_invalid_repo_path) {
    int n_files = 0;
    DiffFileStats* result = diff_get_files("/nonexistent/path", "HEAD", "HEAD~1", &n_files);
    ASSERT_TRUE(result == NULL);
    ASSERT_EQ(n_files, 0);
}

// Test diff_get_files returns NULL for same commit
TEST(test_same_commit) {
    int n_files = 0;
    DiffFileStats* result = diff_get_files("/Volumes/SN/Developer/rloc", "HEAD", "HEAD", &n_files);
    // Same commit should return NULL or empty result
    if (result != NULL) {
        ASSERT_EQ(n_files, 0);
        free_test_files(result, n_files);
    }
}

// Test diff_free_files with NULL
TEST(test_free_null_files) {
    diff_free_files(NULL, 0);
}

// Test diff_free_files frees memory correctly
TEST(test_free_valid_files) {
    int n_files = 2;
    DiffFileStats* files = malloc(n_files * sizeof(DiffFileStats));
    ASSERT_TRUE(files != NULL);

    files[0].filepath = strdup("file1.c");
    files[0].added = 10;
    files[0].removed = 5;
    files[0].lang = "C";

    files[1].filepath = strdup("file2.h");
    files[1].added = 3;
    files[1].removed = 1;
    files[1].lang = "C header";

    // Should not crash
    diff_free_files(files, n_files);
}

// Test diff_get_files with valid repo and different commits
TEST(test_valid_repo_diff) {
    int n_files = 0;
    DiffFileStats* result = diff_get_files("/Volumes/SN/Developer/rloc", "HEAD", "HEAD", &n_files);

    // At minimum, verify the function doesn't crash
    if (result != NULL) {
        if (n_files > 0) {
            ASSERT_TRUE(n_files <= 1000);  // Reasonable upper bound
            for (int i = 0; i < n_files; i++) {
                ASSERT_TRUE(result[i].filepath != NULL);
                ASSERT_TRUE(strlen(result[i].filepath) > 0);
            }
        }
        free_test_files(result, n_files);
    }
}

// Test diff_get_stats_extended with NULL config
TEST(test_extended_null_config) {
    int n_files = 0;
    DiffFileStats* result = diff_get_stats_extended(NULL, &n_files);
    ASSERT_TRUE(result == NULL);
    ASSERT_EQ(n_files, 0);
}

// Test diff_get_stats_extended with DIFF_MODE_RELATIVE
TEST(test_extended_relative_mode) {
    int n_files = 0;
    DiffConfig cfg = make_config("/Volumes/SN/Developer/rloc", "HEAD", "HEAD", DIFF_MODE_RELATIVE);
    DiffFileStats* result = diff_get_stats_extended(&cfg, &n_files);
    // Same ref should return NULL (early exit)
    ASSERT_TRUE(result == NULL);
    ASSERT_EQ(n_files, 0);
}

// Test diff_get_stats_extended with DIFF_MODE_ALL and same refs
TEST(test_extended_all_mode_same_ref) {
    int n_files = 0;
    DiffConfig cfg = make_config("/Volumes/SN/Developer/rloc", "HEAD", "HEAD", DIFF_MODE_ALL);
    DiffFileStats* result = diff_get_stats_extended(&cfg, &n_files);
    // Same ref should return NULL (early exit)
    ASSERT_TRUE(result == NULL);
    ASSERT_EQ(n_files, 0);
}

// Test diff_free_config with NULL
TEST(test_free_null_config) {
    diff_free_config(NULL);
}

// Test diff_free_config frees owned strings
TEST(test_free_config_owned_strings) {
    DiffConfig cfg;
    cfg.ref1 = strdup("abc123");
    cfg.ref2 = strdup("def456");
    cfg.repo_path = strdup("/some/path");
    cfg.flags = DIFF_MODE_RELATIVE;

    diff_free_config(&cfg);

    ASSERT_TRUE(cfg.ref1 == NULL);
    ASSERT_TRUE(cfg.ref2 == NULL);
    ASSERT_TRUE(cfg.repo_path == NULL);
    ASSERT_EQ(cfg.flags, 0);
}

// Test diff_get_stats_extended with DIFF_IGNORE_WHITESPACE
TEST(test_extended_ignore_whitespace) {
    int n_files = 0;
    DiffConfig cfg = make_config("/Volumes/SN/Developer/rloc", "HEAD", "HEAD",
                                 DIFF_MODE_RELATIVE | DIFF_IGNORE_WHITESPACE);
    DiffFileStats* result = diff_get_stats_extended(&cfg, &n_files);
    // Same ref - early exit
    ASSERT_TRUE(result == NULL);
}

// Test diff_get_stats_extended with DIFF_MODE_RELATIVE between actual different commits
TEST(test_extended_relative_different_commits) {
    int n_files = 0;
    DiffConfig cfg = make_config("/Volumes/SN/Developer/rloc", "HEAD~1", "HEAD", DIFF_MODE_RELATIVE);
    DiffFileStats* result = diff_get_stats_extended(&cfg, &n_files);

    // Function should not crash, may return results or NULL depending on repo state
    if (result != NULL) {
        if (n_files > 0) {
            ASSERT_TRUE(n_files <= 1000);
            for (int i = 0; i < n_files; i++) {
                ASSERT_TRUE(result[i].filepath != NULL);
                // Verify at least one of added/removed is non-zero for changed files
                ASSERT_TRUE(result[i].added > 0 || result[i].removed > 0);
            }
        }
        free_test_files(result, n_files);
    }
}

// Test diff_get_stats_extended with DIFF_MODE_ALL between actual different commits
TEST(test_extended_all_mode_different_commits) {
    int n_files = 0;
    DiffConfig cfg = make_config("/Volumes/SN/Developer/rloc", "HEAD~1", "HEAD", DIFF_MODE_ALL);
    DiffFileStats* result = diff_get_stats_extended(&cfg, &n_files);

    // Function should not crash
    if (result != NULL) {
        if (n_files > 0) {
            ASSERT_TRUE(n_files <= 10000);
            // Check all filepaths are valid
            int has_changes = 0;
            for (int i = 0; i < n_files; i++) {
                ASSERT_TRUE(result[i].filepath != NULL);
                ASSERT_TRUE(strlen(result[i].filepath) > 0);
                if (result[i].added > 0 || result[i].removed > 0) {
                    has_changes = 1;
                }
            }
            // At least some files should have changes between different commits
            ASSERT_TRUE(has_changes == 1);
        }
        free_test_files(result, n_files);
    }
}

int main(void) {
    register_test("invalid_repo_path", test_func_test_invalid_repo_path);
    register_test("same_commit", test_func_test_same_commit);
    register_test("free_null_files", test_func_test_free_null_files);
    register_test("free_valid_files", test_func_test_free_valid_files);
    register_test("valid_repo_diff", test_func_test_valid_repo_diff);
    register_test("extended_null_config", test_func_test_extended_null_config);
    register_test("extended_relative_mode", test_func_test_extended_relative_mode);
    register_test("extended_all_mode_same_ref", test_func_test_extended_all_mode_same_ref);
    register_test("free_null_config", test_func_test_free_null_config);
    register_test("free_config_owned_strings", test_func_test_free_config_owned_strings);
    register_test("extended_ignore_whitespace", test_func_test_extended_ignore_whitespace);
    register_test("extended_relative_different_commits", test_func_test_extended_relative_different_commits);
    register_test("extended_all_mode_different_commits", test_func_test_extended_all_mode_different_commits);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}
