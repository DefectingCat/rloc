#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../temp_manager.h"
#include "test_framework.h"

TEST(test_create_manager) {
    TempManager mgr;
    int result = temp_manager_create(&mgr, 1024 * 1024);
    ASSERT_EQ(result, 0);
    ASSERT_EQ((int)mgr.max_total_size, 1024 * 1024);
    ASSERT_EQ(mgr.n_dirs, 0);
    ASSERT_EQ(mgr.n_files, 0);
    temp_manager_destroy(&mgr);
}

TEST(test_create_dir) {
    TempManager mgr;
    temp_manager_create(&mgr, 1024 * 1024 * 1024);
    char* dir = temp_manager_create_dir(&mgr, "test");
    ASSERT_TRUE(dir != NULL);
    ASSERT_EQ(mgr.n_dirs, 1);
    ASSERT_TRUE(access(dir, F_OK) == 0);
    temp_manager_cleanup_all(&mgr);
    ASSERT_TRUE(access(dir, F_OK) != 0);
    temp_manager_destroy(&mgr);
}

TEST(test_create_file) {
    TempManager mgr;
    temp_manager_create(&mgr, 1024 * 1024 * 1024);
    char* file = temp_manager_create_file(&mgr, "test");
    ASSERT_TRUE(file != NULL);
    ASSERT_EQ(mgr.n_files, 1);
    ASSERT_TRUE(access(file, F_OK) == 0);
    temp_manager_cleanup_all(&mgr);
    ASSERT_TRUE(access(file, F_OK) != 0);
    temp_manager_destroy(&mgr);
}

TEST(test_register_dir) {
    TempManager mgr;
    temp_manager_create(&mgr, 1024 * 1024 * 1024);
    char tmpl[] = "/tmp/test_manual.XXXXXX";
    char* dir = mkdtemp(tmpl);
    ASSERT_TRUE(dir != NULL);
    int result = temp_manager_register_dir(&mgr, dir);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(mgr.n_dirs, 1);
    temp_manager_cleanup_all(&mgr);
    ASSERT_TRUE(access(dir, F_OK) != 0);
    temp_manager_destroy(&mgr);
}

TEST(test_multiple_items) {
    TempManager mgr;
    temp_manager_create(&mgr, 1024 * 1024 * 1024);
    char* dir1 = temp_manager_create_dir(&mgr, "test1");
    char* dir2 = temp_manager_create_dir(&mgr, "test2");
    char* file1 = temp_manager_create_file(&mgr, "test1");
    ASSERT_EQ(mgr.n_dirs, 2);
    ASSERT_EQ(mgr.n_files, 1);
    temp_manager_cleanup_all(&mgr);
    ASSERT_TRUE(access(dir1, F_OK) != 0);
    ASSERT_TRUE(access(dir2, F_OK) != 0);
    ASSERT_TRUE(access(file1, F_OK) != 0);
    temp_manager_destroy(&mgr);
}

int main(void) {
    register_test("create_manager", test_func_test_create_manager);
    register_test("create_dir", test_func_test_create_dir);
    register_test("create_file", test_func_test_create_file);
    register_test("register_dir", test_func_test_register_dir);
    register_test("multiple_items", test_func_test_multiple_items);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}