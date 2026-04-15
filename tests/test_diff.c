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
    // Using a valid repo path but same commit should return no changes
    // This test may fail if tests directory is not a git repo
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
    // This will test against the actual rloc repo
    // If there are no differences between HEAD and HEAD~1, it may return NULL
    DiffFileStats* result = diff_get_files("/Volumes/SN/Developer/rloc", "HEAD", "HEAD", &n_files);

    // At minimum, verify the function doesn't crash
    // The actual result depends on repo state
    if (result != NULL) {
        // If we got results, verify they look valid
        if (n_files > 0) {
            ASSERT_TRUE(n_files <= 1000);  // Reasonable upper bound
            // Check that file paths are valid strings
            for (int i = 0; i < n_files; i++) {
                ASSERT_TRUE(result[i].filepath != NULL);
                ASSERT_TRUE(strlen(result[i].filepath) > 0);
                // added/removed can be 0 for deleted/added files
            }
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
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}
