#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "../unique.h"
#include "test_framework.h"

// Helper to create a temporary file with content
static int create_temp_file(const char* content, char* path_out, size_t path_size) {
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";

    snprintf(path_out, path_size, "%s/rloc_test_temp_XXXXXX", tmpdir);
    int fd = mkstemp(path_out);
    if (fd < 0) return -1;

    ssize_t written = write(fd, content, strlen(content));
    close(fd);

    if (written != (ssize_t)strlen(content)) {
        unlink(path_out);
        return -1;
    }

    return 0;
}


/* Use ASSERT_TRUE for negations instead of ASSERT_NEQ */
#define ASSERT_NOT_NULL(a) ASSERT_TRUE((a) != NULL)

TEST(test_unique_table_init_free) {
    UniqueTable table;

    // Test successful init
    ASSERT_EQ(unique_table_init(&table, 1024), 0);
    ASSERT_NOT_NULL(table.entries);
    ASSERT_EQ(table.capacity, 1024);
    ASSERT_EQ(table.count, 0);

    // Test free
    unique_table_free(&table);
    ASSERT_TRUE(table.entries == NULL);
    ASSERT_EQ(table.capacity, 0);
    ASSERT_EQ(table.count, 0);

    // Test init with invalid capacity
    ASSERT_EQ(unique_table_init(&table, 0), -1);
    ASSERT_EQ(unique_table_init(&table, -1), -1);
}

TEST(test_unique_table_insert_and_contains) {
    UniqueTable table;
    ASSERT_EQ(unique_table_init(&table, 1024), 0);

    uint8_t md5_1[16] = {0};
    uint8_t md5_2[16] = {0};
    uint8_t md5_3[16] = {0};

    // Set some distinct values (not real MD5, just distinct bytes)
    md5_1[0] = 0x01;
    md5_2[0] = 0x02;
    md5_3[0] = 0x03;

    // Insert first MD5 - should return 0 (not duplicate)
    ASSERT_EQ(unique_table_insert(&table, md5_1), 0);

    // Contains should return 1
    ASSERT_EQ(unique_table_contains(&table, md5_1), 1);

    // Insert same MD5 again - should return 1 (duplicate)
    ASSERT_EQ(unique_table_insert(&table, md5_1), 1);

    // Insert second MD5 - should return 0
    ASSERT_EQ(unique_table_insert(&table, md5_2), 0);
    ASSERT_EQ(unique_table_contains(&table, md5_2), 1);
    ASSERT_EQ(table.count, 2);

    // Third MD5
    ASSERT_EQ(unique_table_insert(&table, md5_3), 0);
    ASSERT_EQ(unique_table_contains(&table, md5_3), 1);
    ASSERT_EQ(table.count, 3);

    // First MD5 should still be found
    ASSERT_EQ(unique_table_contains(&table, md5_1), 1);

    unique_table_free(&table);
}

TEST(test_unique_table_duplicates) {
    UniqueTable table;
    ASSERT_EQ(unique_table_init(&table, 1024), 0);

    uint8_t md5[16] = {0};
    md5[0] = 0xFF;

    // Insert same MD5 multiple times
    ASSERT_EQ(unique_table_insert(&table, md5), 0);
    ASSERT_EQ(unique_table_insert(&table, md5), 1);
    ASSERT_EQ(unique_table_insert(&table, md5), 1);
    ASSERT_EQ(unique_table_insert(&table, md5), 1);

    // Count should still be 1
    ASSERT_EQ(table.count, 1);

    unique_table_free(&table);
}

TEST(test_compute_file_md5) {
    char tmp_path[256];
    const char* content = "Hello, World!\n";

    ASSERT_EQ(create_temp_file(content, tmp_path, sizeof(tmp_path)), 0);

    uint8_t md5_out[16];
    int ret = compute_file_md5(tmp_path, md5_out);

    // Clean up
    unlink(tmp_path);

    ASSERT_EQ(ret, 0);

    // MD5 of "Hello, World!\n" (on Linux with md5sum, on macOS with md5 -r)
    // Expected: bea8252ff4e80f41719ea13cdf007273 (Linux md5sum)
    uint8_t expected[16] = {
        0xbe, 0xa8, 0x25, 0x2f, 0xf4, 0xe8, 0x0f, 0x41,
        0x71, 0x9e, 0xa1, 0x3c, 0xdf, 0x00, 0x72, 0x73
    };
    ASSERT_TRUE(memcmp(md5_out, expected, 16) == 0);
}

TEST(test_hex_to_md5) {
    uint8_t out[16];

    // Test with known MD5 string
    const char* hex = "65a8e27d8879283831b664bd81d35ce1";
    hex_to_md5(hex, out);

    uint8_t expected[16] = {
        0x65, 0xa8, 0xe2, 0x7d, 0x88, 0x79, 0x28, 0x38,
        0x31, 0xb6, 0x64, 0xbd, 0x81, 0xd3, 0x5c, 0xe1
    };

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(out[i], expected[i]);
    }

    // Test with all zeros
    const char* hex_zero = "00000000000000000000000000000000";
    hex_to_md5(hex_zero, out);
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(out[i], 0);
    }

    // Test with all ones
    const char* hex_one = "ffffffffffffffffffffffffffffffff";
    hex_to_md5(hex_one, out);
    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(out[i], 0xFF);
    }
}

TEST(test_unique_table_integration) {
    UniqueTable table;
    ASSERT_EQ(unique_table_init(&table, 1024), 0);

    char tmp_path[256];
    const char* content = "test content for unique detection\n";

    ASSERT_EQ(create_temp_file(content, tmp_path, sizeof(tmp_path)), 0);

    uint8_t md5[16];
    ASSERT_EQ(compute_file_md5(tmp_path, md5), 0);

    // First insert should succeed
    ASSERT_EQ(unique_table_insert(&table, md5), 0);

    // Contains should find it
    ASSERT_EQ(unique_table_contains(&table, md5), 1);

    // Second insert should detect duplicate
    ASSERT_EQ(unique_table_insert(&table, md5), 1);

    unlink(tmp_path);
    unique_table_free(&table);
}

int main(void) {
    register_test("unique_table_init_free", test_func_test_unique_table_init_free);
    register_test("unique_table_insert_and_contains", test_func_test_unique_table_insert_and_contains);
    register_test("unique_table_duplicates", test_func_test_unique_table_duplicates);
    register_test("compute_file_md5", test_func_test_compute_file_md5);
    register_test("hex_to_md5", test_func_test_hex_to_md5);
    register_test("unique_table_integration", test_func_test_unique_table_integration);

    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}
