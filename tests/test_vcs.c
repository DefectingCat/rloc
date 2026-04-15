#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "../vcs.h"
#include "test_framework.h"

/* Helper: Create a temporary directory */
static char* create_temp_dir(void) {
    char* temp_dir = strdup("/tmp/vcs_test_XXXXXX");
    if (mkdtemp(temp_dir) == NULL) {
        free(temp_dir);
        return NULL;
    }
    return temp_dir;
}

/* Helper: Create .git subdirectory to simulate git repo */
static int create_git_repo_sim(const char* base_dir) {
    char git_path[1024];
    snprintf(git_path, sizeof(git_path), "%s/.git", base_dir);
    return mkdir(git_path, 0755);
}

/* Helper: Create .svn subdirectory to simulate svn repo */
static int create_svn_repo_sim(const char* base_dir) {
    char svn_path[1024];
    snprintf(svn_path, sizeof(svn_path), "%s/.svn", base_dir);
    return mkdir(svn_path, 0755);
}

/* Helper: Clean up temporary directory */
static void cleanup_temp_dir(const char* path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", path);
    system(cmd);
}

TEST(test_vcs_is_git_repo_true) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_EQ(create_git_repo_sim(temp_dir), 0);
    ASSERT_TRUE(vcs_is_git_repo(temp_dir) == 1);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_is_git_repo_false) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_TRUE(vcs_is_git_repo(temp_dir) == 0);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_is_svn_repo_true) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_EQ(create_svn_repo_sim(temp_dir), 0);
    ASSERT_TRUE(vcs_is_svn_repo(temp_dir) == 1);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_is_svn_repo_false) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_TRUE(vcs_is_svn_repo(temp_dir) == 0);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_detect_git) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_EQ(create_git_repo_sim(temp_dir), 0);
    VcsType vcs = vcs_detect(temp_dir);
    ASSERT_EQ(vcs, VCS_GIT);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_detect_svn) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_EQ(create_svn_repo_sim(temp_dir), 0);
    VcsType vcs = vcs_detect(temp_dir);
    ASSERT_EQ(vcs, VCS_SVN);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_detect_none) {
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    VcsType vcs = vcs_detect(temp_dir);
    ASSERT_EQ(vcs, VCS_NONE);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_detect_precedence) {
    /* When both .git and .svn exist, git should take precedence */
    char* temp_dir = create_temp_dir();
    ASSERT_TRUE(temp_dir != NULL);

    ASSERT_EQ(create_git_repo_sim(temp_dir), 0);
    ASSERT_EQ(create_svn_repo_sim(temp_dir), 0);
    VcsType vcs = vcs_detect(temp_dir);
    ASSERT_EQ(vcs, VCS_GIT);

    cleanup_temp_dir(temp_dir);
    free(temp_dir);
}

TEST(test_vcs_check_git_available) {
    int available = vcs_check_git_available();
    /* Should return 0 or 1 */
    ASSERT_TRUE(available == 0 || available == 1);
}

int main(void) {
    register_test("vcs_is_git_repo_true", test_func_test_vcs_is_git_repo_true);
    register_test("vcs_is_git_repo_false", test_func_test_vcs_is_git_repo_false);
    register_test("vcs_is_svn_repo_true", test_func_test_vcs_is_svn_repo_true);
    register_test("vcs_is_svn_repo_false", test_func_test_vcs_is_svn_repo_false);
    register_test("vcs_detect_git", test_func_test_vcs_detect_git);
    register_test("vcs_detect_svn", test_func_test_vcs_detect_svn);
    register_test("vcs_detect_none", test_func_test_vcs_detect_none);
    register_test("vcs_detect_precedence", test_func_test_vcs_detect_precedence);
    register_test("vcs_check_git_available", test_func_test_vcs_check_git_available);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}
