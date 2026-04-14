#include "../language.h"
#include "test_framework.h"
#include <stdio.h>
#include <string.h>

TEST(test_detect_c_by_extension) {
    const Language *lang = detect_language("test.c");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C");
}

TEST(test_detect_cpp_by_extension) {
    const Language *lang = detect_language("test.cpp");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C++");
}

TEST(test_detect_python_by_extension) {
    const Language *lang = detect_language("test.py");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Python");
}

TEST(test_detect_javascript_by_extension) {
    const Language *lang = detect_language("test.js");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "JavaScript");
}

TEST(test_detect_by_filename) {
    const Language *lang = detect_language("Makefile");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Shell");
}

TEST(test_detect_unknown) {
    const Language *lang = detect_language("readme.txt");
    ASSERT_TRUE(lang == NULL);
}

TEST(test_detect_multiple_extensions) {
    const Language *lang = detect_language("test.hxx");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C++");
}

int main(void) {
    // Register all tests manually
    register_test("test_detect_c_by_extension", test_func_test_detect_c_by_extension);
    register_test("test_detect_cpp_by_extension", test_func_test_detect_cpp_by_extension);
    register_test("test_detect_python_by_extension", test_func_test_detect_python_by_extension);
    register_test("test_detect_javascript_by_extension", test_func_test_detect_javascript_by_extension);
    register_test("test_detect_by_filename", test_func_test_detect_by_filename);
    register_test("test_detect_unknown", test_func_test_detect_unknown);
    register_test("test_detect_multiple_extensions", test_func_test_detect_multiple_extensions);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}