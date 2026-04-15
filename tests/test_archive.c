#include <stdio.h>
#include <string.h>

#include "../archive.h"
#include "test_framework.h"

TEST(test_detect_tar_gz) {
    ArchiveType type = archive_detect_type("test.tar.gz");
    ASSERT_EQ(type, ARCHIVE_TAR_GZ);
}

TEST(test_detect_zip) {
    ArchiveType type = archive_detect_type("test.zip");
    ASSERT_EQ(type, ARCHIVE_ZIP);
}

TEST(test_detect_tar) {
    ArchiveType type = archive_detect_type("test.tar");
    ASSERT_EQ(type, ARCHIVE_TAR);
}

TEST(test_detect_tgz) {
    ArchiveType type = archive_detect_type("test.tgz");
    ASSERT_EQ(type, ARCHIVE_TAR_GZ);
}

TEST(test_detect_unknown) {
    ArchiveType type = archive_detect_type("test.txt");
    ASSERT_EQ(type, ARCHIVE_UNKNOWN);
}

TEST(test_is_archive) {
    ASSERT_TRUE(archive_is_archive("file.tar.gz") == 1);
    ASSERT_TRUE(archive_is_archive("file.c") == 0);
}

TEST(test_get_tool) {
    const char* tool = archive_get_tool(ARCHIVE_ZIP);
    ASSERT_STR(tool, "unzip");
    tool = archive_get_tool(ARCHIVE_TAR_GZ);
    ASSERT_STR(tool, "tar");
}

int main(void) {
    register_test("detect_tar_gz", test_func_test_detect_tar_gz);
    register_test("detect_zip", test_func_test_detect_zip);
    register_test("detect_tar", test_func_test_detect_tar);
    register_test("detect_tgz", test_func_test_detect_tgz);
    register_test("detect_unknown", test_func_test_detect_unknown);
    register_test("is_archive", test_func_test_is_archive);
    register_test("get_tool", test_func_test_get_tool);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}