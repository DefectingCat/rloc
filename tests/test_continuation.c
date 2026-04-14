#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../counter.h"
#include "../language.h"
#include "test_framework.h"

TEST(test_shell_continuation) {
    // Shell line ending with backslash
    const char* src = "echo hello \\n    world\\n";
    const Language* shell_lang = get_language_by_name("Shell");
    CountResult result;
    count_lines_with_lang(src, strlen(src), shell_lang, &result);
    // Both lines should count as code (continuation inherits parent type)
    ASSERT_EQ(result.code, 2);
}

TEST(test_c_preprocessor_continuation) {
    // C preprocessor continuation
    const char* src = "#define X \\n    100\\nint y;\\n";
    const Language* c_lang = get_language_by_name("C");
    CountResult result;
    count_lines_with_lang(src, strlen(src), c_lang, &result);
    // #define line + continuation + int y line = 3 code lines
    ASSERT_EQ(result.code, 3);
}

TEST(test_comment_continuation) {
    // Comment with continuation
    const char* src = "# comment \\n    continued\\necho done\\n";
    const Language* shell_lang = get_language_by_name("Shell");
    CountResult result;
    count_lines_with_lang(src, strlen(src), shell_lang, &result);
    // Comment continuation inherits comment type
    ASSERT_EQ(result.comment, 2);
    ASSERT_EQ(result.code, 1);
}

int main(void) {
    register_test("test_shell_continuation", test_func_test_shell_continuation);
    register_test("test_c_preprocessor_continuation", test_func_test_c_preprocessor_continuation);
    register_test("test_comment_continuation", test_func_test_comment_continuation);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}