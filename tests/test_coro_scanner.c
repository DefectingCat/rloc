#include "test_framework.h"
#include "coro_scanner.h"
#include "filelist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// Helper: create temporary test directory structure
static char* create_temp_dir(const char* name) {
    char* path = malloc(256);
    snprintf(path, 256, "/tmp/rloc_test_%s_%d", name, getpid());
    mkdir(path, 0755);
    return path;
}

static void create_file(const char* dir, const char* name, const char* content) {
    char path[256];
    snprintf(path, 256, "%s/%s", dir, name);
    FILE* f = fopen(path, "w");
    if (f) {
        fputs(content ? content : "test\n", f);
        fclose(f);
    }
}

static void create_subdir(const char* dir, const char* name) {
    char path[256];
    snprintf(path, 256, "%s/%s", dir, name);
    mkdir(path, 0755);
}

static void cleanup_dir(const char* path) {
    DIR* d = opendir(path);
    if (!d) return;
    struct dirent* entry;
    while ((entry = readdir(d))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char full[256];
        snprintf(full, 256, "%s/%s", path, entry->d_name);
        if (entry->d_type == DT_DIR) {
            cleanup_dir(full);
        } else {
            unlink(full);
        }
    }
    closedir(d);
    rmdir(path);
}

// Test: empty directory
TEST(empty_directory) {
    char* dir = create_temp_dir("empty");
    FileList list;
    filelist_init(&list);
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    int ret = coro_scan_directory(dir, &config, &list);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(list.count, 0);

    filelist_free(&list);
    cleanup_dir(dir);
    free(dir);
}

// Test: single file
TEST(single_file) {
    char* dir = create_temp_dir("single");
    create_file(dir, "test.c", "int main() { return 0; }\n");

    FileList list;
    filelist_init(&list);
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    int ret = coro_scan_directory(dir, &config, &list);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(list.count, 1);

    filelist_free(&list);
    cleanup_dir(dir);
    free(dir);
}

// Test: multiple files in nested directories
TEST(nested_directories) {
    char* dir = create_temp_dir("nested");
    create_file(dir, "a.c", "// file a\n");
    create_subdir(dir, "sub1");
    create_file(dir, "sub1/b.c", "// file b\n");
    create_subdir(dir, "sub1/sub2");
    create_file(dir, "sub1/sub2/c.c", "// file c\n");

    FileList list;
    filelist_init(&list);
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    int ret = coro_scan_directory(dir, &config, &list);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(list.count, 3);

    filelist_free(&list);
    cleanup_dir(dir);
    free(dir);
}

// Test: result consistency with normal mode
TEST(consistency_with_normal_mode) {
    char* dir = create_temp_dir("consistency");
    create_file(dir, "file1.c", "code\n");
    create_file(dir, "file2.py", "code\n");
    create_file(dir, "file3.js", "code\n");
    create_subdir(dir, "deep");
    create_file(dir, "deep/file4.c", "code\n");
    create_subdir(dir, "deep/deeper");
    create_file(dir, "deep/deeper/file5.c", "code\n");

    FileList coro_list;
    filelist_init(&coro_list);
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    int coro_ret = coro_scan_directory(dir, &config, &coro_list);

    FileList normal_list;
    filelist_init(&normal_list);
    int normal_ret = filelist_scan(dir, &config, &normal_list);

    ASSERT_EQ(coro_ret, 0);
    ASSERT_EQ(normal_ret, 0);
    ASSERT_EQ(coro_list.count, normal_list.count);

    filelist_free(&coro_list);
    filelist_free(&normal_list);
    cleanup_dir(dir);
    free(dir);
}

// Test: many files (stress test for concurrent coro limit)
TEST(many_files) {
    char* dir = create_temp_dir("many");
    // Create 200 files to test concurrent coroutine handling
    for (int i = 0; i < 200; i++) {
        char name[32];
        snprintf(name, 32, "file_%d.c", i);
        create_file(dir, name, "x\n");
    }

    FileList list;
    filelist_init(&list);
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    int ret = coro_scan_directory(dir, &config, &list);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(list.count, 200);

    filelist_free(&list);
    cleanup_dir(dir);
    free(dir);
}

// Main test runner
int main(void) {
    register_test("empty_directory", test_func_empty_directory);
    register_test("single_file", test_func_single_file);
    register_test("nested_directories", test_func_nested_directories);
    register_test("consistency_with_normal_mode", test_func_consistency_with_normal_mode);
    register_test("many_files", test_func_many_files);

    run_all_tests();

    printf("\nCoroutine scanner tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}