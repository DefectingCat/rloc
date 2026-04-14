#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../counter.h"
#include "../language.h"
#include "test_framework.h"

extern const Language g_languages[];

TEST(test_c_single_line_block) {
    const char* src = "/* comment */\nint x = 5;\n";
    const Language* c_lang = get_language_by_name("C");
    CountResult result;
    count_lines_with_lang(src, strlen(src), c_lang, &result);
    ASSERT_EQ(result.comment, 1);
    ASSERT_EQ(result.code, 1);
}

TEST(test_c_multi_line_block) {
    const char* src = "/* line1\nline2\nline3 */\nint x;\n";
    const Language* c_lang = get_language_by_name("C");
    CountResult result;
    count_lines_with_lang(src, strlen(src), c_lang, &result);
    ASSERT_EQ(result.comment, 3);
    ASSERT_EQ(result.code, 1);
}

TEST(test_c_inline_block_with_code) {
    const char* src = "int x = 5; /* comment */ y = 10;\n";
    const Language* c_lang = get_language_by_name("C");
    CountResult result;
    count_lines_with_lang(src, strlen(src), c_lang, &result);
    ASSERT_EQ(result.code, 1);
    ASSERT_EQ(result.comment, 0);
}

TEST(test_html_block_comment) {
    const char* src = "<!-- comment -->\n<p>text</p>\n";
    const Language* html_lang = get_language_by_name("HTML");
    CountResult result;
    count_lines_with_lang(src, strlen(src), html_lang, &result);
    ASSERT_EQ(result.comment, 1);
    ASSERT_EQ(result.code, 1);
}

TEST(test_block_in_string) {
    const char* src = "char *s = \"/* not comment */\";\nint x;\n";
    const Language* c_lang = get_language_by_name("C");
    CountResult result;
    count_lines_with_lang(src, strlen(src), c_lang, &result);
    ASSERT_EQ(result.code, 2);
    ASSERT_EQ(result.comment, 0);
}

TEST(test_css_block_comment) {
    const char* src = "/* CSS comment */\nbody { color: red; }\n";
    const Language* css_lang = get_language_by_name("CSS");
    CountResult result;
    count_lines_with_lang(src, strlen(src), css_lang, &result);
    ASSERT_EQ(result.comment, 1);
}

TEST(test_html_detection) {
    const Language* lang = detect_language("test.html");
    ASSERT_STR(lang->name, "HTML");
}

TEST(test_css_detection) {
    const Language* lang = detect_language("test.css");
    ASSERT_STR(lang->name, "CSS");
}

TEST(test_yaml_detection) {
    const Language* lang = detect_language("config.yaml");
    ASSERT_STR(lang->name, "YAML");
}

int main(void) {
    register_test("test_c_single_line_block", test_func_test_c_single_line_block);
    register_test("test_c_multi_line_block", test_func_test_c_multi_line_block);
    register_test("test_c_inline_block_with_code", test_func_test_c_inline_block_with_code);
    register_test("test_html_block_comment", test_func_test_html_block_comment);
    register_test("test_block_in_string", test_func_test_block_in_string);
    register_test("test_css_block_comment", test_func_test_css_block_comment);
    register_test("test_html_detection", test_func_test_html_detection);
    register_test("test_css_detection", test_func_test_css_detection);
    register_test("test_yaml_detection", test_func_test_yaml_detection);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}