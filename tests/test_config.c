#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../config.h"
#include "../cli.h"
#include "test_framework.h"

/* Create a temporary config file with given content */
static char* create_temp_config(const char* content) {
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";

    char* path = malloc(256);
    if (!path) {
        return NULL;
    }
    snprintf(path, 256, "%s/rloc_test_config_%.5d.txt", tmpdir, getpid());

    FILE* fp = fopen(path, "w");
    if (!fp) {
        free(path);
        return NULL;
    }
    fputs(content, fp);
    fclose(fp);

    return path;
}

/* Clean up temporary file */
static void remove_temp_config(const char* path) {
    unlink(path);
}

/* Reset CliArgs to default state */
static void reset_args(CliArgs* args) {
    memset(args, 0, sizeof(CliArgs));
    args->output_format = FORMAT_TEXT;
    args->max_temp_size = 1024 * 1024 * 1024;  /* Default 1GB */
    args->max_file_size_mb = 100;
    args->progress_rate = 100;
    args->max_archive_depth = 3;
}

TEST(test_load_empty_file) {
    CliArgs args;
    reset_args(&args);

    char* path = create_temp_config("");
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    remove_temp_config(path);
    free(path);
}

TEST(test_load_comments_and_blank_lines) {
    CliArgs args;
    reset_args(&args);

    const char* content =
        "# This is a comment\n"
        "\n"
        "   # Another comment with leading spaces\n"
        "\n"
        "## Double hash comment\n"
        "\n";

    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    remove_temp_config(path);
    free(path);
}

TEST(test_load_single_value_options) {
    CliArgs args;
    reset_args(&args);

    const char* content =
        "--no-recurse\n"
        "--quiet\n"
        "--by-file\n"
        "--by-file-by-lang\n"
        "--skip-uniqueness\n"
        "--json\n";

    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(args.no_recurse, 1);
    ASSERT_EQ(args.quiet, 1);
    ASSERT_EQ(args.by_file, 1);
    ASSERT_EQ(args.by_file_by_lang, 1);
    ASSERT_EQ(args.skip_uniqueness, 1);
    ASSERT_EQ(args.output_format, FORMAT_JSON);

    remove_temp_config(path);
}

TEST(test_load_value_options) {
    CliArgs args;
    reset_args(&args);

    const char* content =
        "--max-file-size=200\n"
        "--max-temp-size=512\n"
        "--progress-rate=50\n"
        "--skip-leading=5\n"
        "--sdir=/tmp/staging\n"
        "--report-file=/tmp/report.txt\n"
        "--exclude-list-file=/tmp/exclude.txt\n"
        "--sql=/tmp/output.sql\n"
        "--vcs=git\n"
        "--diff=abc123..def456\n"
        "--match-f=*.test\n"
        "--not-match-f=*.tmp\n"
        "--match-d=src\n"
        "--not-match-d=build\n"
        "--unique=/tmp/unique.txt\n"
        "--ignored=/tmp/ignored.txt\n";

    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(args.max_file_size_mb, 200);
    ASSERT_EQ(args.max_temp_size, 512 * 1024 * 1024);
    ASSERT_EQ(args.progress_rate, 50);
    ASSERT_EQ(args.skip_leading, 5);
    ASSERT_STR(args.staging_dir, "/tmp/staging");
    ASSERT_STR(args.report_file, "/tmp/report.txt");
    ASSERT_STR(args.exclude_list_file, "/tmp/exclude.txt");
    ASSERT_EQ(args.output_format, FORMAT_SQL);
    ASSERT_STR(args.sql_file, "/tmp/output.sql");
    ASSERT_EQ(args.vcs, VCS_GIT);
    ASSERT_STR(args.diff_commit1, "abc123");
    ASSERT_STR(args.diff_commit2, "def456");
    ASSERT_STR(args.match_pattern, "*.test");
    ASSERT_STR(args.not_match_pattern, "*.tmp");
    ASSERT_STR(args.match_d_pattern, "src");
    ASSERT_STR(args.not_match_d_pattern, "build");
    ASSERT_STR(args.unique_file, "/tmp/unique.txt");
    ASSERT_STR(args.ignored_file, "/tmp/ignored.txt");

    remove_temp_config(path);
}

TEST(test_load_list_options_comma_separated) {
    CliArgs args;
    reset_args(&args);

    const char* content =
        "--exclude-dir=build,dist,node_modules\n"
        "--include-lang=Python,JavaScript,Go\n"
        "--exclude-lang=C,C++\n"
        "--include-ext=.py,.js,.go\n"
        "--exclude-ext=.log,.tmp,.bak\n"
        "--skip-leading=10,.py,.txt\n";

    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(args.n_exclude_dirs, 3);
    ASSERT_STR(args.exclude_dirs[0], "build");
    ASSERT_STR(args.exclude_dirs[1], "dist");
    ASSERT_STR(args.exclude_dirs[2], "node_modules");

    ASSERT_EQ(args.n_include_langs, 3);
    ASSERT_STR(args.include_langs[0], "Python");
    ASSERT_STR(args.include_langs[1], "JavaScript");
    ASSERT_STR(args.include_langs[2], "Go");

    ASSERT_EQ(args.n_exclude_langs, 2);
    ASSERT_STR(args.exclude_langs[0], "C");
    ASSERT_STR(args.exclude_langs[1], "C++");

    ASSERT_EQ(args.n_include_exts, 3);
    ASSERT_STR(args.include_exts[0], ".py");
    ASSERT_STR(args.include_exts[1], ".js");
    ASSERT_STR(args.include_exts[2], ".go");

    ASSERT_EQ(args.n_exclude_exts, 3);
    ASSERT_STR(args.exclude_exts[0], ".log");
    ASSERT_STR(args.exclude_exts[1], ".tmp");
    ASSERT_STR(args.exclude_exts[2], ".bak");

    ASSERT_EQ(args.n_skip_leading_exts, 2);
    ASSERT_STR(args.skip_leading_exts[0], ".py");
    ASSERT_STR(args.skip_leading_exts[1], ".txt");

    /* Clean up allocated memory */
    free(args.exclude_dirs);
    free(args.include_langs);
    free(args.exclude_langs);
    free(args.include_exts);
    free(args.exclude_exts);
    free(args.skip_leading_exts);

    remove_temp_config(path);
}

TEST(test_load_vcs_svn) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--vcs=svn\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.vcs, VCS_SVN);

    remove_temp_config(path);
}

TEST(test_load_vcs_auto) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--vcs=auto\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.vcs, VCS_AUTO);
}

TEST(test_load_csv_output_format) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--csv\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.output_format, FORMAT_CSV);

    remove_temp_config(path);
}

TEST(test_load_md_output_format) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--md\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.output_format, FORMAT_MD);

    remove_temp_config(path);
}

TEST(test_load_yaml_output_format) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--yaml\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.output_format, FORMAT_YAML);

    remove_temp_config(path);
}

TEST(test_load_xml_output_format) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--xml\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.output_format, FORMAT_XML);

    remove_temp_config(path);
}

TEST(test_load_html_output_format) {
    CliArgs args;
    reset_args(&args);

    const char* content = "--html\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    config_load(path, &args);
    ASSERT_EQ(args.output_format, FORMAT_HTML);

    remove_temp_config(path);
}

TEST(test_load_file_not_found) {
    CliArgs args;
    reset_args(&args);

    int ret = config_load("/nonexistent/path/to/config.txt", &args);
    ASSERT_EQ(ret, 0);  /* File not found is OK */
}

TEST(test_load_whitespace_handling) {
    CliArgs args;
    reset_args(&args);

    const char* content =
        "   --quiet   \n"
        "\t--by-file\t\n"
        "  --sdir=/tmp/staging   \n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(args.quiet, 1);
    ASSERT_EQ(args.by_file, 1);
    ASSERT_STR(args.staging_dir, "/tmp/staging");

    free(args.staging_dir);

    remove_temp_config(path);
}

TEST(test_load_duplicate_values_override) {
    CliArgs args;
    reset_args(&args);

    const char* content =
        "--max-file-size=100\n"
        "--max-file-size=200\n"
        "--quiet\n"
        "--quiet\n";
    char* path = create_temp_config(content);
    ASSERT_TRUE(path != NULL);

    int ret = config_load(path, &args);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(args.max_file_size_mb, 200);  /* Last value wins */
    ASSERT_EQ(args.quiet, 1);

    remove_temp_config(path);
}

TEST(test_get_default_path) {
    const char* home = getenv("HOME");
    if (!home) {
        /* Skip test if HOME is not set */
        return;
    }

    char* path = config_get_default_path();
    ASSERT_TRUE(path != NULL);

    /* Check that path contains expected components */
    ASSERT_TRUE(strstr(path, home) != NULL);
    ASSERT_TRUE(strstr(path, ".config") != NULL);
    ASSERT_TRUE(strstr(path, "rloc") != NULL);
    ASSERT_TRUE(strstr(path, "options.txt") != NULL);

    free(path);
}

TEST(test_get_default_path_no_home) {
    /* Temporarily unset HOME */
    const char* home = getenv("HOME");
    if (home) {
        unsetenv("HOME");
        char* path = config_get_default_path();
        ASSERT_TRUE(!path);

        /* Restore HOME */
        setenv("HOME", home, 1);
    }
}

int main(void) {
    register_test("test_load_empty_file", test_func_test_load_empty_file);
    register_test("test_load_comments_and_blank_lines", test_func_test_load_comments_and_blank_lines);
    register_test("test_load_single_value_options", test_func_test_load_single_value_options);
    register_test("test_load_value_options", test_func_test_load_value_options);
    register_test("test_load_list_options_comma_separated", test_func_test_load_list_options_comma_separated);
    register_test("test_load_vcs_svn", test_func_test_load_vcs_svn);
    register_test("test_load_vcs_auto", test_func_test_load_vcs_auto);
    register_test("test_load_csv_output_format", test_func_test_load_csv_output_format);
    register_test("test_load_md_output_format", test_func_test_load_md_output_format);
    register_test("test_load_yaml_output_format", test_func_test_load_yaml_output_format);
    register_test("test_load_xml_output_format", test_func_test_load_xml_output_format);
    register_test("test_load_html_output_format", test_func_test_load_html_output_format);
    register_test("test_load_file_not_found", test_func_test_load_file_not_found);
    register_test("test_load_whitespace_handling", test_func_test_load_whitespace_handling);
    register_test("test_load_duplicate_values_override", test_func_test_load_duplicate_values_override);
    register_test("test_get_default_path", test_func_test_get_default_path);
    register_test("test_get_default_path_no_home", test_func_test_get_default_path_no_home);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
