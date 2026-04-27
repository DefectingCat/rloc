#include <stdio.h>
#include <string.h>

#include "../cli.h"
#include "test_framework.h"

TEST(test_strip_comments_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--strip-comments=nc", "test.c"};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.strip_comments != NULL);
    ASSERT_STR(args.strip_comments, "nc");
    ASSERT_TRUE(args.strip_code == NULL);

    cli_free(&args);
}

TEST(test_strip_code_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--strip-code=nocode", "test.c"};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.strip_code != NULL);
    ASSERT_STR(args.strip_code, "nocode");
    ASSERT_TRUE(args.strip_comments == NULL);

    cli_free(&args);
}

TEST(test_strip_mutually_exclusive) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--strip-comments=nc", "--strip-code=nocode", "test.c"};
    int ret = cli_parse(4, argv, &args);

    // CLI parsing succeeds, but validation should fail in main.c
    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.strip_comments != NULL);
    ASSERT_TRUE(args.strip_code != NULL);

    cli_free(&args);
}

TEST(test_fullpath_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--fullpath", "--match-f='test'", "test.c"};
    int ret = cli_parse(4, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.fullpath == 1);

    cli_free(&args);
}

TEST(test_lang_no_ext_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--lang-no-ext=Python", "Makefile"};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.lang_no_ext != NULL);
    ASSERT_STR(args.lang_no_ext, "Python");

    cli_free(&args);
}

TEST(test_script_lang_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--script-lang=Python,python3", "script"};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_script_lang, 1);
    ASSERT_TRUE(args.script_langs != NULL);
    ASSERT_TRUE(args.script_names != NULL);

    cli_free(&args);
}

TEST(test_follow_links_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--follow-links", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.follow_links == 1);

    cli_free(&args);
}

TEST(test_include_content_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--include-content='Copyright'", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.include_content != NULL);

    cli_free(&args);
}

TEST(test_exclude_content_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--exclude-content='GENERATED'", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.exclude_content != NULL);

    cli_free(&args);
}

TEST(test_timeout_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--timeout=5", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.timeout_sec, 5);

    cli_free(&args);
}

TEST(test_ignore_regex_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--ignore-regex='C|^\\s*[{};]\\s*$'", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_ignore_regex, 1);
    ASSERT_TRUE(args.ignore_regex_langs != NULL);
    ASSERT_TRUE(args.ignore_regex_patterns != NULL);

    cli_free(&args);
}

TEST(test_diagnostic_output_cli) {
    CliArgs args;
    memset(&args, 0, sizeof(args));

    char* argv[] = {"rloc", "--categorized=cat.txt", "--counted=count.txt", "--found=found.txt",
                    "."};
    int ret = cli_parse(5, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_TRUE(args.categorized_file != NULL);
    ASSERT_TRUE(args.counted_file != NULL);
    ASSERT_TRUE(args.found_file != NULL);

    cli_free(&args);
}

int main(void) {
    register_test("strip_comments_cli", test_func_test_strip_comments_cli);
    register_test("strip_code_cli", test_func_test_strip_code_cli);
    register_test("strip_mutually_exclusive", test_func_test_strip_mutually_exclusive);
    register_test("fullpath_cli", test_func_test_fullpath_cli);
    register_test("lang_no_ext_cli", test_func_test_lang_no_ext_cli);
    register_test("script_lang_cli", test_func_test_script_lang_cli);
    register_test("follow_links_cli", test_func_test_follow_links_cli);
    register_test("include_content_cli", test_func_test_include_content_cli);
    register_test("exclude_content_cli", test_func_test_exclude_content_cli);
    register_test("timeout_cli", test_func_test_timeout_cli);
    register_test("ignore_regex_cli", test_func_test_ignore_regex_cli);
    register_test("diagnostic_output_cli", test_func_test_diagnostic_output_cli);
    run_all_tests();
    return tests_passed == test_count ? 0 : 1;
}