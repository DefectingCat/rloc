#include <stdio.h>
#include <string.h>

#include "../vcs.h"
#include "test_framework.h"

TEST(test_git_available) {
    // Check if git is available on the system
    int available = vcs_check_git_available();
    // This test passes regardless - it's just checking git presence
    ASSERT_TRUE(available == 0 || available == 1);
}

TEST(test_detect_git_repo) {
    // Current directory should be a git repo (this project)
    int is_git = vcs_is_git_repo(".");
    ASSERT_TRUE(is_git == 1);  // This project is in a git repo
}

TEST(test_detect_no_git_repo) {
    // /tmp should not be a git repo
    int is_git = vcs_is_git_repo("/tmp");
    ASSERT_TRUE(is_git == 0);
}

TEST(test_vcs_detect) {
    VcsType vcs = vcs_detect(".");
    ASSERT_TRUE(vcs == VCS_GIT);
}

int main(void) {
    register_test("git_available", test_func_test_git_available);
    register_test("detect_git_repo", test_func_test_detect_git_repo);
    register_test("detect_no_git_repo", test_func_test_detect_no_git_repo);
    register_test("vcs_detect", test_func_test_vcs_detect);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}