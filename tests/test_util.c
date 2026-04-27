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

/* Test buffer_new with default capacity */
TEST(test_buffer_new_default) {
    Buffer* buf = buffer_new(0);
    ASSERT_TRUE(buf != NULL);
    ASSERT_TRUE(buf->capacity >= 1024);
    ASSERT_TRUE(buf->size == 0);
    ASSERT_TRUE(buf->data != NULL);
    buffer_free(buf);
}

/* Test buffer_new with explicit capacity */
TEST(test_buffer_new_explicit) {
    Buffer* buf = buffer_new(2048);
    ASSERT_TRUE(buf != NULL);
    ASSERT_TRUE(buf->capacity == 2048);
    ASSERT_TRUE(buf->size == 0);
    buffer_free(buf);
}

/* Test buffer_append and basic usage */
TEST(test_buffer_append_basic) {
    Buffer* buf = buffer_new(1024);
    const char* data = "hello world";
    ASSERT_EQ(buffer_append(buf, data, 11), 0);
    ASSERT_EQ(buf->size, 11);
    ASSERT_STR(buf->data, "hello world");
    buffer_free(buf);
}

/* Test create 1KB buffer, append 100KB data without truncation */
TEST(test_buffer_large_append) {
    Buffer* buf = buffer_new(1024);
    size_t big_size = 100 * 1024; // 100KB
    char* big_data = malloc(big_size);
    ASSERT_TRUE(big_data != NULL);

    // Fill with pattern
    for (size_t i = 0; i < big_size; i++) {
        big_data[i] = (char)('A' + (i % 26));
    }

    // Append all at once
    ASSERT_EQ(buffer_append(buf, big_data, big_size), 0);
    ASSERT_EQ(buf->size, big_size);

    // Verify no truncation
    int ok = 1;
    for (size_t i = 0; i < big_size; i++) {
        if (buf->data[i] != big_data[i]) {
            ok = 0;
            break;
        }
    }
    ASSERT_TRUE(ok == 1);

    free(big_data);
    buffer_free(buf);
}

/* Test buffer_append with NULL buf */
TEST(test_buffer_append_null_buf) {
    ASSERT_EQ(buffer_append(NULL, "data", 4), -1);
}

/* Test buffer_append with NULL data */
TEST(test_buffer_append_null_data) {
    Buffer* buf = buffer_new(1024);
    ASSERT_EQ(buffer_append(buf, NULL, 10), -1);
    buffer_free(buf);
}

/* Test buffer_append with zero length */
TEST(test_buffer_append_zero_len) {
    Buffer* buf = buffer_new(1024);
    const char* data = "hello";
    ASSERT_EQ(buffer_append(buf, data, 0), -1);
    ASSERT_EQ(buf->size, 0);
    buffer_free(buf);
}

/* Test buffer_steal transfers ownership */
TEST(test_buffer_steal) {
    Buffer* buf = buffer_new(1024);
    const char* data = "steal me";
    buffer_append(buf, data, 10);

    size_t out_size = 0;
    char* stolen = buffer_steal(buf, &out_size);
    ASSERT_TRUE(stolen != NULL);
    ASSERT_EQ(out_size, 10);
    ASSERT_STR(stolen, "steal me");

    free(stolen);
}

/* Test buffer_steal NULL buf */
TEST(test_buffer_steal_null) {
    size_t out_size = 999;
    char* result = buffer_steal(NULL, &out_size);
    ASSERT_TRUE(result == NULL);
    ASSERT_EQ(out_size, 0);
}

/* Test buffer_clear resets size */
TEST(test_buffer_clear) {
    Buffer* buf = buffer_new(1024);
    const char* data = "to be cleared";
    buffer_append(buf, data, 13);
    size_t cap = buf->capacity;

    buffer_clear(buf);
    ASSERT_EQ(buf->size, 0);
    ASSERT_EQ(buf->capacity, cap);
    ASSERT_TRUE(buf->data[0] == '\0');

    buffer_free(buf);
}

/* Test buffer_reserve */
TEST(test_buffer_reserve) {
    Buffer* buf = buffer_new(1024);
    ASSERT_EQ(buffer_reserve(buf, 4096), 0);
    ASSERT_TRUE(buf->capacity >= 4096);
    buffer_free(buf);
}

/* Test buffer_reserve already sufficient */
TEST(test_buffer_reserve_already_sufficient) {
    Buffer* buf = buffer_new(4096);
    ASSERT_EQ(buffer_reserve(buf, 1024), 0);
    ASSERT_EQ(buf->capacity, 4096);
    buffer_free(buf);
}

/* Test buffer_reserve NULL buf */
TEST(test_buffer_reserve_null) {
    ASSERT_EQ(buffer_reserve(NULL, 1024), -1);
}

/* Test multiple appends trigger growth */
TEST(test_buffer_multiple_appends) {
    Buffer* buf = buffer_new(64);
    for (int i = 0; i < 100; i++) {
        const char* chunk = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        ASSERT_EQ(buffer_append(buf, chunk, 26), 0);
    }
    ASSERT_EQ(buf->size, 2600);
    buffer_free(buf);
}

/* Mock large repository test: simulate 100K file list */
TEST(test_buffer_mock_repo_large) {
    Buffer* buf = buffer_new(4096);

    // Simulate git ls-files -z output with 100K entries
    // Each entry: "path/to/file_NNNNNN.c\0" ~ 25 bytes each => ~2.5MB total
    char entry[64];
    int total_entries = 100000;
    int entry_count = 0;
    int failed = 0;

    for (int i = 0; i < total_entries; i++) {
        int len = snprintf(entry, sizeof(entry), "src/file_%06d.c", i);
        entry[len] = '\0';
        if (buffer_append(buf, entry, len + 1) != 0) {
            failed = 1;
            break;
        }
        entry_count++;
    }

    ASSERT_TRUE(failed == 0);
    ASSERT_EQ(entry_count, 100000);

    // Verify we can count null separators
    int null_count = 0;
    for (size_t i = 0; i < buf->size; i++) {
        if (buf->data[i] == '\0') null_count++;
    }
    ASSERT_EQ(null_count, 100000);

    buffer_free(buf);
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

    /* Buffer tests */
    register_test("test_buffer_new_default", test_func_test_buffer_new_default);
    register_test("test_buffer_new_explicit", test_func_test_buffer_new_explicit);
    register_test("test_buffer_append_basic", test_func_test_buffer_append_basic);
    register_test("test_buffer_large_append", test_func_test_buffer_large_append);
    register_test("test_buffer_append_null_buf", test_func_test_buffer_append_null_buf);
    register_test("test_buffer_append_null_data", test_func_test_buffer_append_null_data);
    register_test("test_buffer_append_zero_len", test_func_test_buffer_append_zero_len);
    register_test("test_buffer_steal", test_func_test_buffer_steal);
    register_test("test_buffer_steal_null", test_func_test_buffer_steal_null);
    register_test("test_buffer_clear", test_func_test_buffer_clear);
    register_test("test_buffer_reserve", test_func_test_buffer_reserve);
    register_test("test_buffer_reserve_already_sufficient", test_func_test_buffer_reserve_already_sufficient);
    register_test("test_buffer_reserve_null", test_func_test_buffer_reserve_null);
    register_test("test_buffer_multiple_appends", test_func_test_buffer_multiple_appends);
    register_test("test_buffer_mock_repo_large", test_func_test_buffer_mock_repo_large);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
