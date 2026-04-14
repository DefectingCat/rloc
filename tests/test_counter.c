#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../counter.h"
#include "test_framework.h"

TEST(test_empty_file) {
    CountResult result;
    count_lines("", 0, &result);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.total, 0);
}

TEST(test_all_blank) {
    CountResult result;
    const char* input = "\n\n\n\n\n\n\n\n\n\n";  // 10 blank lines
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 10);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.total, 10);
}

TEST(test_all_code) {
    CountResult result;
    const char* input = "int main() {\nreturn 0;\n}\n";  // 3 lines of code + 1 blank
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 3);
    ASSERT_EQ(result.total, 3);
}

TEST(test_mixed) {
    CountResult result;
    const char* input = "\nint x;\nint y;\nint z;\n\n";  // blank, 3 code, blank
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 2);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 3);
    ASSERT_EQ(result.total, 5);
}

TEST(test_trailing_newline) {
    CountResult result;
    const char* input = "hello\n";  // 1 line of code
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.total, 1);
}

TEST(test_no_trailing_newline) {
    CountResult result;
    const char* input = "hello";  // 1 line of code, no trailing newline
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.total, 1);
}

TEST(test_blank_with_spaces) {
    CountResult result;
    const char* input = "   \n\t\n";  // 2 blank lines with spaces/tabs
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 2);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.total, 2);
}

TEST(test_file_not_found) {
    CountResult result;
    int ret = count_file("nonexistent_file_xyz123.c", &result);
    ASSERT_EQ(ret, -1);
}

int main(void) {
    // Register all tests manually
    register_test("test_empty_file", test_func_test_empty_file);
    register_test("test_all_blank", test_func_test_all_blank);
    register_test("test_all_code", test_func_test_all_code);
    register_test("test_mixed", test_func_test_mixed);
    register_test("test_trailing_newline", test_func_test_trailing_newline);
    register_test("test_no_trailing_newline", test_func_test_no_trailing_newline);
    register_test("test_blank_with_spaces", test_func_test_blank_with_spaces);
    register_test("test_file_not_found", test_func_test_file_not_found);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
