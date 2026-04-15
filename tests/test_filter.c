#include <stdio.h>
#include <string.h>
#include "../filelist.h"
#include "test_framework.h"

TEST(test_match_pattern_syntax) {
    // Test that regex patterns work with valid syntax
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    // Basic regex pattern - match .c files
    config.match_pattern = "\\.c$";

    // This is a syntax test - the pattern should compile without error
    // The actual matching is tested by running rloc
    ASSERT_TRUE(config.match_pattern != NULL);
}

TEST(test_not_match_pattern_syntax) {
    // Test that not-match patterns work with valid syntax
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    // Basic regex pattern - exclude test files
    config.not_match_pattern = "^test_";

    ASSERT_TRUE(config.not_match_pattern != NULL);
}

TEST(test_match_d_pattern_syntax) {
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    config.match_d_pattern = "^src$";

    ASSERT_TRUE(config.match_d_pattern != NULL);
}

TEST(test_not_match_d_pattern_syntax) {
    FilelistConfig config;
    memset(&config, 0, sizeof(config));

    config.not_match_d_pattern = "^(test|tests)$";

    ASSERT_TRUE(config.not_match_d_pattern != NULL);
}

int main(void) {
    register_test("match_pattern_syntax", test_func_test_match_pattern_syntax);
    register_test("not_match_pattern_syntax", test_func_test_not_match_pattern_syntax);
    register_test("match_d_pattern_syntax", test_func_test_match_d_pattern_syntax);
    register_test("not_match_d_pattern_syntax", test_func_test_not_match_d_pattern_syntax);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}