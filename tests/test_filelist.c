#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../filelist.h"
#include "../util.h"
#include "test_framework.h"

TEST(test_scan_single_file) {
    // Create a temporary test file
    const char* test_file = "/tmp/rloc_test_file.c";
    FILE* f = fopen(test_file, "w");
    ASSERT_TRUE(f != NULL);
    fprintf(f, "#include <stdio.h>\n\nint main() {\n    return 0;\n}\n");
    fclose(f);

    FileList list;
    filelist_init(&list);

    FilelistConfig config;
    config.no_recurse = 0;
    config.max_file_size = 1024 * 1024;
    config.exclude_dirs = NULL;
    config.n_exclude_dirs = 0;

    int result = filelist_scan(test_file, &config, &list);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(list.count, 1);

    filelist_free(&list);
    remove(test_file);
}

TEST(test_scan_directory_recursive) {
    // Create temporary directory structure
    system("mkdir -p /tmp/rloc_test_dir/subdir");

    FILE* f1 = fopen("/tmp/rloc_test_dir/file1.c", "w");
    fprintf(f1, "int main() { return 0; }\n");
    fclose(f1);

    FILE* f2 = fopen("/tmp/rloc_test_dir/subdir/file2.c", "w");
    fprintf(f2, "void func() {}\n");
    fclose(f2);

    FileList list;
    filelist_init(&list);

    FilelistConfig config;
    config.no_recurse = 0;
    config.max_file_size = 1024 * 1024;
    config.exclude_dirs = NULL;
    config.n_exclude_dirs = 0;

    int result = filelist_scan("/tmp/rloc_test_dir", &config, &list);
    ASSERT_EQ(result, 0);
    ASSERT_TRUE(list.count >= 2);  // At least the 2 files we created

    filelist_free(&list);

    // Cleanup
    system("rm -rf /tmp/rloc_test_dir");
}

TEST(test_no_recurse_option) {
    // Create temporary directory structure
    system("mkdir -p /tmp/rloc_test_norecurse/subdir");

    FILE* f1 = fopen("/tmp/rloc_test_norecurse/file1.c", "w");
    fprintf(f1, "int main() { return 0; }\n");
    fclose(f1);

    FILE* f2 = fopen("/tmp/rloc_test_norecurse/subdir/file2.c", "w");
    fprintf(f2, "void func() {}\n");
    fclose(f2);

    FileList list;
    filelist_init(&list);

    FilelistConfig config;
    config.no_recurse = 1;  // Don't recurse
    config.max_file_size = 1024 * 1024;
    config.exclude_dirs = NULL;
    config.n_exclude_dirs = 0;

    int result = filelist_scan("/tmp/rloc_test_norecurse", &config, &list);
    ASSERT_EQ(result, 0);
    // Should only find file1.c, not file2.c in subdirectory
    ASSERT_TRUE(list.count >= 1);

    // Verify subdir/file2.c is NOT in the list
    int found_subdir_file = 0;
    for (int i = 0; i < list.count; i++) {
        if (strstr(list.paths[i], "subdir") != NULL) {
            found_subdir_file = 1;
        }
    }
    ASSERT_TRUE(found_subdir_file == 0);

    filelist_free(&list);

    // Cleanup
    system("rm -rf /tmp/rloc_test_norecurse");
}

TEST(test_exclude_dir_option) {
    // Create temporary directory structure
    system("mkdir -p /tmp/rloc_test_exclude/excluded_dir");
    system("mkdir -p /tmp/rloc_test_exclude/included_dir");

    FILE* f1 = fopen("/tmp/rloc_test_exclude/excluded_dir/file1.c", "w");
    fprintf(f1, "int main() { return 0; }\n");
    fclose(f1);

    FILE* f2 = fopen("/tmp/rloc_test_exclude/included_dir/file2.c", "w");
    fprintf(f2, "void func() {}\n");
    fclose(f2);

    FileList list;
    filelist_init(&list);

    FilelistConfig config;
    config.no_recurse = 0;
    config.max_file_size = 1024 * 1024;

    // Exclude "excluded_dir"
    const char* excludes[] = {"excluded_dir"};
    config.exclude_dirs = (char**)excludes;
    config.n_exclude_dirs = 1;

    int result = filelist_scan("/tmp/rloc_test_exclude", &config, &list);
    ASSERT_EQ(result, 0);

    // Verify excluded_dir/file1.c is NOT in the list
    int found_excluded = 0;
    for (int i = 0; i < list.count; i++) {
        if (strstr(list.paths[i], "excluded_dir") != NULL) {
            found_excluded = 1;
        }
    }
    ASSERT_TRUE(found_excluded == 0);

    filelist_free(&list);

    // Cleanup
    system("rm -rf /tmp/rloc_test_exclude");
}

int main() {
    register_test("test_scan_single_file", test_func_test_scan_single_file);
    register_test("test_scan_directory_recursive", test_func_test_scan_directory_recursive);
    register_test("test_no_recurse_option", test_func_test_no_recurse_option);
    register_test("test_exclude_dir_option", test_func_test_exclude_dir_option);

    run_all_tests();
    return 0;
}
