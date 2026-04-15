#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "test_framework.h"
#include "../util.h"

/* Test directory for temporary files */
static const char* test_dir = "/tmp/rloc_test_util";

/* Helper to create a temporary test directory */
static void create_test_dir(void) {
    mkdir(test_dir, 0755);
}

/* Helper to clean up test directory */
static void cleanup_test_dir(void) {
    /* Remove test file if exists */
    char* test_file = path_join(test_dir, "test_file.txt");
    if (test_file) {
        unlink(test_file);
        free(test_file);
    }
    char* bin_file = path_join(test_dir, "test_bin.bin");
    if (bin_file) {
        unlink(bin_file);
        free(bin_file);
    }
    char* link = path_join(test_dir, "test_link");
    if (link) {
        unlink(link);
        free(link);
    }
    rmdir(test_dir);
}

/* Test is_regular_file */
TEST(test_is_regular_file_positive) {
    create_test_dir();
    char* path = path_join(test_dir, "test_file.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fwrite("hello", 1, 5, f);
        fclose(f);
        ASSERT_TRUE(is_regular_file(path) == 1);
    }
    cleanup_test_dir();
    free(path);
}

TEST(test_is_regular_file_negative_directory) {
    create_test_dir();
    ASSERT_TRUE(is_regular_file(test_dir) == 0);
    cleanup_test_dir();
}

TEST(test_is_regular_file_nonexistent) {
    ASSERT_TRUE(is_regular_file("/tmp/nonexistent_file_xyz123") == 0);
}

/* Test is_directory */
TEST(test_is_directory_positive) {
    create_test_dir();
    ASSERT_TRUE(is_directory(test_dir) == 1);
    cleanup_test_dir();
}

TEST(test_is_directory_negative_file) {
    create_test_dir();
    char* path = path_join(test_dir, "test_file.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fwrite("hello", 1, 5, f);
        fclose(f);
    }
    ASSERT_TRUE(is_directory(path) == 0);
    cleanup_test_dir();
    free(path);
}

TEST(test_is_directory_nonexistent) {
    ASSERT_TRUE(is_directory("/tmp/nonexistent_dir_xyz123") == 0);
}

/* Test is_symlink */
TEST(test_is_symlink_positive) {
    create_test_dir();
    char* target = path_join(test_dir, "target_file.txt");
    char* link = path_join(test_dir, "symlink_file");

    FILE* f = fopen(target, "w");
    if (f) {
        fwrite("hello", 1, 5, f);
        fclose(f);
    }

    symlink(target, link);
    ASSERT_TRUE(is_symlink(link) == 1);

    unlink(link);
    unlink(target);
    cleanup_test_dir();
    free(target);
    free(link);
}

TEST(test_is_symlink_negative_regular_file) {
    create_test_dir();
    char* path = path_join(test_dir, "test_file.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fwrite("hello", 1, 5, f);
        fclose(f);
    }
    ASSERT_TRUE(is_symlink(path) == 0);
    unlink(path);
    cleanup_test_dir();
    free(path);
}

TEST(test_is_symlink_negative_directory) {
    create_test_dir();
    ASSERT_TRUE(is_symlink(test_dir) == 0);
    cleanup_test_dir();
}

/* Test get_file_size */
TEST(test_get_file_size_positive) {
    create_test_dir();
    char* path = path_join(test_dir, "test_file.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fwrite("hello world", 1, 11, f);
        fclose(f);
        ASSERT_EQ(get_file_size(path), 11);
    }
    unlink(path);
    cleanup_test_dir();
    free(path);
}

TEST(test_get_file_size_empty) {
    create_test_dir();
    char* path = path_join(test_dir, "empty_file.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fclose(f);
        ASSERT_EQ(get_file_size(path), 0);
    }
    unlink(path);
    cleanup_test_dir();
    free(path);
}

TEST(test_get_file_size_nonexistent) {
    ASSERT_EQ(get_file_size("/tmp/nonexistent_file_xyz123"), -1);
}

/* Test path_join */
TEST(test_path_join_with_trailing_slash) {
    char* result = path_join("/tmp/dir/", "file.txt");
    if (result) {
        ASSERT_STR(result, "/tmp/dir/file.txt");
        free(result);
    }
}

TEST(test_path_join_without_trailing_slash) {
    char* result = path_join("/tmp/dir", "file.txt");
    if (result) {
        ASSERT_STR(result, "/tmp/dir/file.txt");
        free(result);
    }
}

TEST(test_path_join_empty_dir) {
    char* result = path_join("", "file.txt");
    if (result) {
        ASSERT_STR(result, "/file.txt");
        free(result);
    }
}

TEST(test_path_join_empty_filename) {
    char* result = path_join("/tmp/dir", "");
    if (result) {
        ASSERT_STR(result, "/tmp/dir/");
        free(result);
    }
}

TEST(test_path_join_both_empty) {
    char* result = path_join("", "");
    if (result) {
        ASSERT_STR(result, "/");
        free(result);
    }
}

/* Test is_binary_file */
TEST(test_is_binary_file_binary) {
    create_test_dir();
    char* path = path_join(test_dir, "test_bin.bin");
    FILE* f = fopen(path, "wb");
    if (f) {
        /* Write some null bytes to make it binary */
        unsigned char bytes[] = {'h', 'e', 'l', 'l', 'o', '\0', 'w', 'o'};
        fwrite(bytes, 1, sizeof(bytes), f);
        fclose(f);
        ASSERT_TRUE(is_binary_file(path) == 1);
    }
    unlink(path);
    cleanup_test_dir();
    free(path);
}

TEST(test_is_binary_file_text) {
    create_test_dir();
    char* path = path_join(test_dir, "test_text.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fwrite("hello world\nthis is text\n", 1, 25, f);
        fclose(f);
        ASSERT_TRUE(is_binary_file(path) == 0);
    }
    unlink(path);
    cleanup_test_dir();
    free(path);
}

TEST(test_is_binary_file_empty) {
    create_test_dir();
    char* path = path_join(test_dir, "empty.txt");
    FILE* f = fopen(path, "w");
    if (f) {
        fclose(f);
        ASSERT_TRUE(is_binary_file(path) == 0);
    }
    unlink(path);
    cleanup_test_dir();
    free(path);
}

TEST(test_is_binary_file_nonexistent) {
    ASSERT_TRUE(is_binary_file("/tmp/nonexistent_file_xyz123") == 0);
}

int main(void) {
    /* Register all tests */
    register_test("test_is_regular_file_positive", test_func_test_is_regular_file_positive);
    register_test("test_is_regular_file_negative_directory", test_func_test_is_regular_file_negative_directory);
    register_test("test_is_regular_file_nonexistent", test_func_test_is_regular_file_nonexistent);
    register_test("test_is_directory_positive", test_func_test_is_directory_positive);
    register_test("test_is_directory_negative_file", test_func_test_is_directory_negative_file);
    register_test("test_is_directory_nonexistent", test_func_test_is_directory_nonexistent);
    register_test("test_is_symlink_positive", test_func_test_is_symlink_positive);
    register_test("test_is_symlink_negative_regular_file", test_func_test_is_symlink_negative_regular_file);
    register_test("test_is_symlink_negative_directory", test_func_test_is_symlink_negative_directory);
    register_test("test_get_file_size_positive", test_func_test_get_file_size_positive);
    register_test("test_get_file_size_empty", test_func_test_get_file_size_empty);
    register_test("test_get_file_size_nonexistent", test_func_test_get_file_size_nonexistent);
    register_test("test_path_join_with_trailing_slash", test_func_test_path_join_with_trailing_slash);
    register_test("test_path_join_without_trailing_slash", test_func_test_path_join_without_trailing_slash);
    register_test("test_path_join_empty_dir", test_func_test_path_join_empty_dir);
    register_test("test_path_join_empty_filename", test_func_test_path_join_empty_filename);
    register_test("test_path_join_both_empty", test_func_test_path_join_both_empty);
    register_test("test_is_binary_file_binary", test_func_test_is_binary_file_binary);
    register_test("test_is_binary_file_text", test_func_test_is_binary_file_text);
    register_test("test_is_binary_file_empty", test_func_test_is_binary_file_empty);
    register_test("test_is_binary_file_nonexistent", test_func_test_is_binary_file_nonexistent);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
