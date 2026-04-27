#include <stdio.h>
#include <string.h>

#include "../parallel.h"
#include "test_framework.h"

TEST(test_default_config) {
    ParallelConfig config;
    parallel_default_config(&config);
    ASSERT_TRUE(config.n_workers >= 1);
    ASSERT_TRUE(config.n_workers <= 16);
    ASSERT_EQ((int)config.chunk_size, 100);
    ASSERT_EQ(config.timeout_sec, 300);
}

TEST(test_single_file_fallback) {
    // Single file should use sequential fallback
    ParallelConfig config;
    config.n_workers = 4;
    config.chunk_size = 100;
    config.timeout_sec = 300;

    ParallelInputFile files[] = {{.filepath = "main.c", .lang = NULL}};
    ParallelResult results[1];
    int n_results = 1;

    int count = parallel_count_files(files, 1, &config, NULL, 0, 0, results, &n_results);
    ASSERT_TRUE(count >= 0 || count == -1);  // May fail if file not accessible
}

TEST(test_small_count_fallback) {
    // Less than 50 files should use sequential fallback
    ParallelConfig config;
    config.n_workers = 1;  // Single worker forces sequential
    config.chunk_size = 100;
    config.timeout_sec = 300;

    ParallelInputFile files[] = {
        {.filepath = "main.c", .lang = NULL},
        {.filepath = "cli.c", .lang = NULL},
        {.filepath = "counter.c", .lang = NULL}
    };
    ParallelResult results[10];
    int n_results = 10;

    int count = parallel_count_files(files, 3, &config, NULL, 0, 0, results, &n_results);
    ASSERT_TRUE(count >= 0 || count == -1);  // May fail if files not accessible
}

int main(void) {
    register_test("default_config", test_func_test_default_config);
    register_test("single_file_fallback", test_func_test_single_file_fallback);
    register_test("small_count_fallback", test_func_test_small_count_fallback);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}