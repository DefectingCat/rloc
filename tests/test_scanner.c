#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../scanner.h"
#include "../temp_manager.h"
#include "../filelist.h"
#include "test_framework.h"

/* Test scanner_scan_input_paths with empty input */
TEST(test_scanner_empty_input) {
    CliArgs args = {0};
    FilelistConfig config = {0};
    FileList filelist;
    TempManager temp_mgr;
    int error_count = 0;

    filelist_init(&filelist);
    temp_manager_create(&temp_mgr, 10);

    args.n_input_files = 0;
    args.input_files = NULL;

    int result = scanner_scan_input_paths(&args, &config, &filelist, &temp_mgr, &error_count);

    ASSERT_EQ(result, 0);
    ASSERT_EQ(error_count, 0);
    ASSERT_EQ(filelist.count, 0);

    temp_manager_destroy(&temp_mgr);
}

/* Test scanner_scan_input_paths with non-existent path */
TEST(test_scanner_nonexistent_path) {
    CliArgs args = {0};
    FilelistConfig config = {0};
    FileList filelist;
    TempManager temp_mgr;
    int error_count = 0;

    filelist_init(&filelist);
    temp_manager_create(&temp_mgr, 10);

    const char* inputs[] = {"/nonexistent/path/that/does/not/exist"};
    args.n_input_files = 1;
    args.input_files = inputs;

    int result = scanner_scan_input_paths(&args, &config, &filelist, &temp_mgr, &error_count);

    /* Should return 0 but increment error_count */
    ASSERT_EQ(result, 0);
    ASSERT_TRUE(error_count > 0);

    temp_manager_destroy(&temp_mgr);
}

/* Test scanner_scan_input_paths with a regular file */
TEST(test_scanner_regular_file) {
    CliArgs args = {0};
    FilelistConfig config = {0};
    FileList filelist;
    TempManager temp_mgr;
    int error_count = 0;

    /* Create a temporary test file */
    const char* test_file = "/tmp/rloc_test_scanner_file.c";
    FILE* f = fopen(test_file, "w");
    ASSERT_TRUE(f != NULL);
    if (f) {
        fprintf(f, "int main() { return 0; }\n");
        fclose(f);
    }

    filelist_init(&filelist);
    temp_manager_create(&temp_mgr, 10);

    const char* inputs[] = {test_file};
    args.n_input_files = 1;
    args.input_files = inputs;

    int result = scanner_scan_input_paths(&args, &config, &filelist, &temp_mgr, &error_count);

    ASSERT_EQ(result, 0);
    ASSERT_EQ(error_count, 0);
    ASSERT_EQ(filelist.count, 1);

    /* Cleanup */
    unlink(test_file);
    temp_manager_destroy(&temp_mgr);
}

/* Test scanner_scan_input_paths with a directory */
TEST(test_scanner_directory) {
    CliArgs args = {0};
    FilelistConfig config = {0};
    FileList filelist;
    TempManager temp_mgr;
    int error_count = 0;

    /* Create a temporary test directory with files */
    const char* test_dir = "/tmp/rloc_test_scanner_dir";
    mkdir(test_dir, 0755);

    char test_file1[256], test_file2[256];
    snprintf(test_file1, sizeof(test_file1), "%s/file1.c", test_dir);
    snprintf(test_file2, sizeof(test_file2), "%s/file2.h", test_dir);

    FILE* f1 = fopen(test_file1, "w");
    if (f1) { fprintf(f1, "int x;\n"); fclose(f1); }

    FILE* f2 = fopen(test_file2, "w");
    if (f2) { fprintf(f2, "int y;\n"); fclose(f2); }

    filelist_init(&filelist);
    temp_manager_create(&temp_mgr, 10);

    const char* inputs[] = {test_dir};
    args.n_input_files = 1;
    args.input_files = inputs;

    int result = scanner_scan_input_paths(&args, &config, &filelist, &temp_mgr, &error_count);

    ASSERT_EQ(result, 0);
    ASSERT_EQ(error_count, 0);
    ASSERT_TRUE(filelist.count >= 2);

    /* Cleanup */
    unlink(test_file1);
    unlink(test_file2);
    rmdir(test_dir);
    temp_manager_destroy(&temp_mgr);
}

int main(void) {
    register_test("test_scanner_empty_input", test_func_test_scanner_empty_input);
    register_test("test_scanner_nonexistent_path", test_func_test_scanner_nonexistent_path);
    register_test("test_scanner_regular_file", test_func_test_scanner_regular_file);
    register_test("test_scanner_directory", test_func_test_scanner_directory);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}