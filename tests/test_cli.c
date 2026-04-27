#include <stdio.h>
#include <string.h>

#include "../cli.h"
#include "test_framework.h"

/* Helper: reset CliArgs to a clean default state */
static void reset_args(CliArgs* args) {
    memset(args, 0, sizeof(CliArgs));
    args->vcs = VCS_AUTO;
    args->output_format = FORMAT_TEXT;
}

/* ============================================================
 *  Help / Version
 * ============================================================ */

TEST(test_help_long_flag) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--help"};
    int ret = cli_parse(2, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.show_help, 1);
    ASSERT_EQ(args.n_input_files, 0);

    cli_free(&args);
}

TEST(test_help_short_flag) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "-h", "/some/path"};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.show_help, 1);
    ASSERT_EQ(args.n_input_files, 1);

    cli_free(&args);
}

TEST(test_version_flag) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--version"};
    int ret = cli_parse(2, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.show_version, 1);

    cli_free(&args);
}

/* ============================================================
 *  --exclude-dir
 * ============================================================ */

TEST(test_exclude_dir_single) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--exclude-dir=node_modules", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_exclude_dirs, 1);
    ASSERT_STR(args.exclude_dirs[0], "node_modules");

    cli_free(&args);
}

TEST(test_exclude_dir_multiple_comma) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--exclude-dir=build,dist,.git", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_exclude_dirs, 3);
    ASSERT_STR(args.exclude_dirs[0], "build");
    ASSERT_STR(args.exclude_dirs[1], "dist");
    ASSERT_STR(args.exclude_dirs[2], ".git");

    cli_free(&args);
}

/* ============================================================
 *  --include-lang
 * ============================================================ */

TEST(test_include_lang_single) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--include-lang=Python", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_include_langs, 1);
    ASSERT_STR(args.include_langs[0], "Python");

    cli_free(&args);
}

TEST(test_include_lang_multiple) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--include-lang=Python,JavaScript,Go", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_include_langs, 3);
    ASSERT_STR(args.include_langs[0], "Python");
    ASSERT_STR(args.include_langs[1], "JavaScript");
    ASSERT_STR(args.include_langs[2], "Go");

    cli_free(&args);
}

/* ============================================================
 *  --exclude-lang
 * ============================================================ */

TEST(test_exclude_lang_single) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--exclude-lang=C", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_exclude_langs, 1);
    ASSERT_STR(args.exclude_langs[0], "C");

    cli_free(&args);
}

TEST(test_exclude_lang_multiple) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--exclude-lang=C,C++,Java", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_exclude_langs, 3);
    ASSERT_STR(args.exclude_langs[0], "C");
    ASSERT_STR(args.exclude_langs[1], "C++");
    ASSERT_STR(args.exclude_langs[2], "Java");

    cli_free(&args);
}

/* ============================================================
 *  --vcs
 * ============================================================ */

TEST(test_vcs_git) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--vcs=git", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.vcs, VCS_GIT);

    cli_free(&args);
}

TEST(test_vcs_svn) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--vcs=svn", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.vcs, VCS_SVN);

    cli_free(&args);
}

TEST(test_vcs_auto) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--vcs=auto", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.vcs, VCS_AUTO);

    cli_free(&args);
}

TEST(test_vcs_none) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--no-vcs", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.vcs, VCS_NONE);

    cli_free(&args);
}

TEST(test_vcs_unknown_fails) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--vcs=hg", "."};
    int ret = cli_parse(3, argv, &args);

    /* Unknown VCS type should return -1 */
    ASSERT_EQ(ret, -1);

    /* When cli_parse returns -1, it has already freed input_files.
     * We must NOT call cli_free() to avoid double-free. */
}

/* ============================================================
 *  --output-format
 * ============================================================ */

TEST(test_output_format_text_default) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "."};
    int ret = cli_parse(2, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_TEXT);

    cli_free(&args);
}

TEST(test_output_format_json) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--json", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_JSON);

    cli_free(&args);
}

TEST(test_output_format_csv) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--csv", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_CSV);

    cli_free(&args);
}

TEST(test_output_format_yaml) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--yaml", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_YAML);

    cli_free(&args);
}

TEST(test_output_format_md) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--md", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_MD);

    cli_free(&args);
}

TEST(test_output_format_xml) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--xml", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_XML);

    cli_free(&args);
}

TEST(test_output_format_html) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--html", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.output_format, FORMAT_HTML);

    cli_free(&args);
}

/* ============================================================
 *  --by-file / --by-file-by-lang
 * ============================================================ */

TEST(test_by_file) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--by-file", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.by_file, 1);
    ASSERT_EQ(args.by_file_by_lang, 0);

    cli_free(&args);
}

TEST(test_by_file_by_lang) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--by-file-by-lang", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.by_file_by_lang, 1);
    ASSERT_EQ(args.by_file, 0);

    cli_free(&args);
}

/* ============================================================
 *  --follow-links
 * ============================================================ */

TEST(test_follow_links) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--follow-links", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.follow_links, 1);

    cli_free(&args);
}

/* ============================================================
 *  --quiet
 * ============================================================ */

TEST(test_quiet) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--quiet", "."};
    int ret = cli_parse(3, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.quiet, 1);

    cli_free(&args);
}

/* ============================================================
 *  No arguments (should fail)
 * ============================================================ */

TEST(test_no_args_fails) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc"};
    int ret = cli_parse(1, argv, &args);

    ASSERT_EQ(ret, -1);

    /* When cli_parse returns -1, it has already freed internal allocations. */
}

/* ============================================================
 *  Unknown parameter (treated as input file, not an error)
 * ============================================================ */

TEST(test_unknown_param_treated_as_file) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--unknown-flag"};
    int ret = cli_parse(2, argv, &args);

    /* Unknown flags become input file paths, not errors */
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_input_files, 1);
    ASSERT_STR(args.input_files[0], "--unknown-flag");

    cli_free(&args);
}

/* ============================================================
 *  Parameter combination tests
 * ============================================================ */

TEST(test_combo_exclude_dir_and_include_lang) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--exclude-dir=node_modules,build", "--include-lang=Python,JavaScript",
                    "."};
    int ret = cli_parse(4, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_exclude_dirs, 2);
    ASSERT_STR(args.exclude_dirs[0], "node_modules");
    ASSERT_STR(args.exclude_dirs[1], "build");
    ASSERT_EQ(args.n_include_langs, 2);
    ASSERT_STR(args.include_langs[0], "Python");
    ASSERT_STR(args.include_langs[1], "JavaScript");

    cli_free(&args);
}

TEST(test_combo_vcs_json_by_file_quiet) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--vcs=git", "--json", "--by-file", "--quiet", "."};
    int ret = cli_parse(6, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.vcs, VCS_GIT);
    ASSERT_EQ(args.output_format, FORMAT_JSON);
    ASSERT_EQ(args.by_file, 1);
    ASSERT_EQ(args.quiet, 1);

    cli_free(&args);
}

TEST(test_combo_exclude_lang_csv_by_file_by_lang) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--exclude-lang=C,C++", "--csv", "--by-file-by-lang", "src"};
    int ret = cli_parse(5, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.n_exclude_langs, 2);
    ASSERT_STR(args.exclude_langs[0], "C");
    ASSERT_STR(args.exclude_langs[1], "C++");
    ASSERT_EQ(args.output_format, FORMAT_CSV);
    ASSERT_EQ(args.by_file_by_lang, 1);

    cli_free(&args);
}

TEST(test_combo_follow_links_vcs_auto_yaml) {
    CliArgs args;
    reset_args(&args);

    char* argv[] = {"rloc", "--follow-links", "--vcs=auto", "--yaml", "."};
    int ret = cli_parse(5, argv, &args);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(args.follow_links, 1);
    ASSERT_EQ(args.vcs, VCS_AUTO);
    ASSERT_EQ(args.output_format, FORMAT_YAML);

    cli_free(&args);
}

int main(void) {
    /* Help / Version */
    register_test("help_long_flag", test_func_test_help_long_flag);
    register_test("help_short_flag", test_func_test_help_short_flag);
    register_test("version_flag", test_func_test_version_flag);

    /* --exclude-dir */
    register_test("exclude_dir_single", test_func_test_exclude_dir_single);
    register_test("exclude_dir_multiple_comma", test_func_test_exclude_dir_multiple_comma);

    /* --include-lang */
    register_test("include_lang_single", test_func_test_include_lang_single);
    register_test("include_lang_multiple", test_func_test_include_lang_multiple);

    /* --exclude-lang */
    register_test("exclude_lang_single", test_func_test_exclude_lang_single);
    register_test("exclude_lang_multiple", test_func_test_exclude_lang_multiple);

    /* --vcs */
    register_test("vcs_git", test_func_test_vcs_git);
    register_test("vcs_svn", test_func_test_vcs_svn);
    register_test("vcs_auto", test_func_test_vcs_auto);
    register_test("vcs_none", test_func_test_vcs_none);
    register_test("vcs_unknown_fails", test_func_test_vcs_unknown_fails);

    /* --output-format */
    register_test("output_format_text_default", test_func_test_output_format_text_default);
    register_test("output_format_json", test_func_test_output_format_json);
    register_test("output_format_csv", test_func_test_output_format_csv);
    register_test("output_format_yaml", test_func_test_output_format_yaml);
    register_test("output_format_md", test_func_test_output_format_md);
    register_test("output_format_xml", test_func_test_output_format_xml);
    register_test("output_format_html", test_func_test_output_format_html);

    /* --by-file / --by-file-by-lang */
    register_test("by_file", test_func_test_by_file);
    register_test("by_file_by_lang", test_func_test_by_file_by_lang);

    /* --follow-links */
    register_test("follow_links", test_func_test_follow_links);

    /* --quiet */
    register_test("quiet", test_func_test_quiet);

    /* No arguments */
    register_test("no_args_fails", test_func_test_no_args_fails);

    /* Unknown parameter */
    register_test("unknown_param_treated_as_file", test_func_test_unknown_param_treated_as_file);

    /* Combinations */
    register_test("combo_exclude_dir_and_include_lang",
                  test_func_test_combo_exclude_dir_and_include_lang);
    register_test("combo_vcs_json_by_file_quiet", test_func_test_combo_vcs_json_by_file_quiet);
    register_test("combo_exclude_lang_csv_by_file_by_lang",
                  test_func_test_combo_exclude_lang_csv_by_file_by_lang);
    register_test("combo_follow_links_vcs_auto_yaml",
                  test_func_test_combo_follow_links_vcs_auto_yaml);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
