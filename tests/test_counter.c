#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../counter.h"
#include "../language.h"
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

TEST(test_block_comment_multiline) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "int x;\n/* comment line 1\n   comment line 2\n   comment line 3 */\nint y;\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 2);
    ASSERT_EQ(result.comment, 3);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 5);
}

TEST(test_empty_block_comment) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "/**/\nint x;\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 1);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 2);
}

TEST(test_fake_comment_in_string) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "char* s = \"// not a comment\";\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_consecutive_line_comments) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "// first\n// second\n// third\nint x;\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 3);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 4);
}

TEST(test_mixed_whitespace_blank_lines) {
    CountResult result;
    const char* input = "   \n\t  \n\t\n  \t  \nint x;\n";
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.blank, 4);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.total, 5);
}

TEST(test_unicode_in_code) {
    CountResult result;
    const char* input = "const char* msg = \"Hello \xe4\xb8\x96\xe7\x95\x8c\";\n";
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_long_line) {
    CountResult result;
    char input[2000];
    memset(input, 'a', sizeof(input) - 1);
    input[sizeof(input) - 2] = '\n';
    input[sizeof(input) - 1] = '\0';
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_only_comments_file) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "/* header comment */\n// line comment\n/* another\n   block */\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.comment, 4);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 4);
}

TEST(test_code_comment_mixed_tight) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "int x; /* inline */ int y;\n/* start */ int z; /* end */\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 2);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 2);
}

TEST(test_unclosed_block_comment) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "int x;\n/* never closed\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 1);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 2);
}

TEST(test_block_comment_with_code_after) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "/* comment */ int x;\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_string_with_block_comment_markers) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "char* s = \"/* not a comment */\";\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_blank_line_with_carriage_return) {
    CountResult result;
    const char* input = "hello\r\nworld\r\n";
    count_lines(input, strlen(input), &result);
    ASSERT_EQ(result.code, 2);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.total, 2);
}

TEST(test_multiple_block_comments_on_one_line) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "/* a */ /* b */ /* c */\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.comment, 1);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_code_before_and_after_block_comment) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "int x; /* comment */ int y;\n";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 1);
}

TEST(test_line_comment_at_eof_no_newline) {
    CountResult result;
    const Language* c = get_language_by_name("C");
    const char* input = "// comment at end";
    count_lines_with_lang(input, strlen(input), c, 0, &result);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.comment, 1);
    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.total, 1);
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
    register_test("test_block_comment_multiline", test_func_test_block_comment_multiline);
    register_test("test_empty_block_comment", test_func_test_empty_block_comment);
    register_test("test_fake_comment_in_string", test_func_test_fake_comment_in_string);
    register_test("test_consecutive_line_comments", test_func_test_consecutive_line_comments);
    register_test("test_mixed_whitespace_blank_lines", test_func_test_mixed_whitespace_blank_lines);
    register_test("test_unicode_in_code", test_func_test_unicode_in_code);
    register_test("test_long_line", test_func_test_long_line);
    register_test("test_only_comments_file", test_func_test_only_comments_file);
    register_test("test_code_comment_mixed_tight", test_func_test_code_comment_mixed_tight);
    register_test("test_unclosed_block_comment", test_func_test_unclosed_block_comment);
    register_test("test_block_comment_with_code_after", test_func_test_block_comment_with_code_after);
    register_test("test_string_with_block_comment_markers", test_func_test_string_with_block_comment_markers);
    register_test("test_blank_line_with_carriage_return", test_func_test_blank_line_with_carriage_return);
    register_test("test_multiple_block_comments_on_one_line", test_func_test_multiple_block_comments_on_one_line);
    register_test("test_code_before_and_after_block_comment", test_func_test_code_before_and_after_block_comment);
    register_test("test_line_comment_at_eof_no_newline", test_func_test_line_comment_at_eof_no_newline);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
