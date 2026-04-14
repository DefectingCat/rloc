#include <stdio.h>
#include <string.h>

#include "../strlit.h"
#include "test_framework.h"

TEST(test_no_strings) {
    const char* input = "int x = 5;\nint y = 10;";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    ASSERT_STR(output, input);
}

TEST(test_double_quote_string) {
    const char* input = "int x = 5;\nconst char *s = \"hello world\";";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    // Check that string delimiters are preserved
    ASSERT_TRUE(strstr(output, "\"hello world\"") != NULL);
    // Check that content between quotes is replaced with spaces
    const char* quote_start = strchr(output, '"');
    const char* quote_end = strchr(quote_start + 1, '"');
    int content_len = quote_end - quote_start - 1;
    int space_count = 0;
    for (int i = 0; i < content_len; i++) {
        if (quote_start[i + 1] == ' ') space_count++;
    }
    ASSERT_EQ(space_count, content_len);
}

TEST(test_single_quote_char) {
    const char* input = "char c = 'a';";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    // Character literals should be preserved as-is
    ASSERT_TRUE(strstr(output, "'a'") != NULL);
}

TEST(test_escaped_quote) {
    const char* input = "const char *s = \"hello \\\"world\\\"\"";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    // Escaped quotes should not end the string
    const char* first_quote = strchr(output, '"');
    const char* last_quote = strrchr(output, '"');
    ASSERT_TRUE(last_quote > first_quote);
}

TEST(test_comment_in_string) {
    const char* input = "const char *s = \"/* not comment */\";";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    // Comment markers inside string should be hidden (replaced with spaces)
    const char* quote_start = strchr(output, '"');
    const char* quote_end = strchr(quote_start + 1, '"');
    ASSERT_TRUE(quote_end != NULL);
    // Content should be spaces
    int content_len = quote_end - quote_start - 1;
    int space_count = 0;
    for (int i = 0; i < content_len; i++) {
        if (quote_start[i + 1] == ' ') space_count++;
    }
    ASSERT_EQ(space_count, content_len);
}

TEST(test_multiline_string) {
    const char* input = "const char *s = \"line1\nline2\";";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    // Newlines in strings should be preserved
    ASSERT_TRUE(strchr(output, '\n') != NULL);
}

TEST(test_empty_string) {
    const char* input = "const char *s = \"\";";
    char output[100];
    size_t len = strip_string_literals(input, strlen(input), output, "\"'", "\\");
    ASSERT_EQ(len, strlen(input));
    // Empty string should be preserved
    ASSERT_TRUE(strstr(output, "\"\"") != NULL);
}

int main(void) {
    // Register all tests manually
    register_test("test_no_strings", test_func_test_no_strings);
    register_test("test_double_quote_string", test_func_test_double_quote_string);
    register_test("test_single_quote_char", test_func_test_single_quote_char);
    register_test("test_escaped_quote", test_func_test_escaped_quote);
    register_test("test_comment_in_string", test_func_test_comment_in_string);
    register_test("test_multiline_string", test_func_test_multiline_string);
    register_test("test_empty_string", test_func_test_empty_string);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}