#include <stdio.h>
#include <string.h>

#include "../language.h"
#include "test_framework.h"

TEST(test_detect_c_by_extension) {
    const Language* lang = detect_language("test.c");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C");
}

TEST(test_detect_cpp_by_extension) {
    const Language* lang = detect_language("test.cpp");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C++");
}

TEST(test_detect_python_by_extension) {
    const Language* lang = detect_language("test.py");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Python");
}

TEST(test_detect_javascript_by_extension) {
    const Language* lang = detect_language("test.js");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "JavaScript");
}

TEST(test_detect_by_filename) {
    const Language* lang = detect_language("Makefile");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Shell");
}

TEST(test_detect_unknown) {
    const Language* lang = detect_language("readme.txt");
    ASSERT_TRUE(lang == NULL);
}

TEST(test_detect_multiple_extensions) {
    const Language* lang = detect_language("test.hxx");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C++");
}

TEST(test_detect_html_by_extension) {
    const Language* lang = detect_language("test.html");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "HTML");
}

TEST(test_detect_html_htm_by_extension) {
    const Language* lang = detect_language("test.htm");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "HTML");
}

TEST(test_detect_css_by_extension) {
    const Language* lang = detect_language("test.css");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "CSS");
}

TEST(test_detect_sql_by_extension) {
    const Language* lang = detect_language("test.sql");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "SQL");
}

TEST(test_detect_xml_by_extension) {
    const Language* lang = detect_language("test.xml");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "XML");
}

TEST(test_detect_yaml_by_extension) {
    const Language* lang = detect_language("test.yaml");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "YAML");
}

TEST(test_detect_yml_by_extension) {
    const Language* lang = detect_language("test.yml");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "YAML");
}

TEST(test_detect_toml_by_extension) {
    const Language* lang = detect_language("test.toml");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "TOML");
}

TEST(test_detect_json_by_extension) {
    const Language* lang = detect_language("test.json");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "JSON");
}

TEST(test_detect_markdown_by_extension) {
    const Language* lang = detect_language("test.md");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Markdown");
}

TEST(test_detect_markdown_markdown_by_extension) {
    const Language* lang = detect_language("test.markdown");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Markdown");
}

TEST(test_detect_vue_by_extension) {
    const Language* lang = detect_language("test.vue");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Vue");
}

TEST(test_detect_go_by_extension) {
    const Language* lang = detect_language("test.go");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Go");
}

TEST(test_detect_rust_by_extension) {
    const Language* lang = detect_language("test.rs");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Rust");
}

TEST(test_detect_java_by_extension) {
    const Language* lang = detect_language("test.java");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Java");
}

TEST(test_detect_typescript_by_extension) {
    const Language* lang = detect_language("test.ts");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "TypeScript");
}

TEST(test_detect_typescript_tsx_by_extension) {
    const Language* lang = detect_language("test.tsx");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "TypeScript");
}

TEST(test_detect_ruby_by_extension) {
    const Language* lang = detect_language("test.rb");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Ruby");
}

TEST(test_detect_php_by_extension) {
    const Language* lang = detect_language("test.php");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "PHP");
}

TEST(test_detect_csharp_by_extension) {
    const Language* lang = detect_language("test.cs");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C#");
}

TEST(test_detect_swift_by_extension) {
    const Language* lang = detect_language("test.swift");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Swift");
}

TEST(test_detect_kotlin_by_extension) {
    const Language* lang = detect_language("test.kt");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Kotlin");
}

TEST(test_detect_kotlin_kts_by_extension) {
    const Language* lang = detect_language("test.kts");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Kotlin");
}

TEST(test_detect_perl_by_extension) {
    const Language* lang = detect_language("test.pl");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Perl");
}

TEST(test_detect_perl_pm_by_extension) {
    const Language* lang = detect_language("test.pm");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Perl");
}

int main(void) {
    // Register all tests manually
    register_test("test_detect_c_by_extension", test_func_test_detect_c_by_extension);
    register_test("test_detect_cpp_by_extension", test_func_test_detect_cpp_by_extension);
    register_test("test_detect_python_by_extension", test_func_test_detect_python_by_extension);
    register_test("test_detect_javascript_by_extension",
                  test_func_test_detect_javascript_by_extension);
    register_test("test_detect_by_filename", test_func_test_detect_by_filename);
    register_test("test_detect_unknown", test_func_test_detect_unknown);
    register_test("test_detect_multiple_extensions", test_func_test_detect_multiple_extensions);

    // New tests for 24 languages
    register_test("test_detect_html_by_extension", test_func_test_detect_html_by_extension);
    register_test("test_detect_html_htm_by_extension", test_func_test_detect_html_htm_by_extension);
    register_test("test_detect_css_by_extension", test_func_test_detect_css_by_extension);
    register_test("test_detect_sql_by_extension", test_func_test_detect_sql_by_extension);
    register_test("test_detect_xml_by_extension", test_func_test_detect_xml_by_extension);
    register_test("test_detect_yaml_by_extension", test_func_test_detect_yaml_by_extension);
    register_test("test_detect_yml_by_extension", test_func_test_detect_yml_by_extension);
    register_test("test_detect_toml_by_extension", test_func_test_detect_toml_by_extension);
    register_test("test_detect_json_by_extension", test_func_test_detect_json_by_extension);
    register_test("test_detect_markdown_by_extension", test_func_test_detect_markdown_by_extension);
    register_test("test_detect_markdown_markdown_by_extension",
                  test_func_test_detect_markdown_markdown_by_extension);
    register_test("test_detect_vue_by_extension", test_func_test_detect_vue_by_extension);
    register_test("test_detect_go_by_extension", test_func_test_detect_go_by_extension);
    register_test("test_detect_rust_by_extension", test_func_test_detect_rust_by_extension);
    register_test("test_detect_java_by_extension", test_func_test_detect_java_by_extension);
    register_test("test_detect_typescript_by_extension",
                  test_func_test_detect_typescript_by_extension);
    register_test("test_detect_typescript_tsx_by_extension",
                  test_func_test_detect_typescript_tsx_by_extension);
    register_test("test_detect_ruby_by_extension", test_func_test_detect_ruby_by_extension);
    register_test("test_detect_php_by_extension", test_func_test_detect_php_by_extension);
    register_test("test_detect_csharp_by_extension", test_func_test_detect_csharp_by_extension);
    register_test("test_detect_swift_by_extension", test_func_test_detect_swift_by_extension);
    register_test("test_detect_kotlin_by_extension", test_func_test_detect_kotlin_by_extension);
    register_test("test_detect_kotlin_kts_by_extension",
                  test_func_test_detect_kotlin_kts_by_extension);
    register_test("test_detect_perl_by_extension", test_func_test_detect_perl_by_extension);
    register_test("test_detect_perl_pm_by_extension", test_func_test_detect_perl_pm_by_extension);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
