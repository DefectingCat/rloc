#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../output.h"
#include "test_framework.h"

/* Language helpers for test fixtures */
static const Language* lang_c(void) { return get_language_by_name("C"); }
static const Language* lang_python(void) { return get_language_by_name("Python"); }

/* Test fixture: create a single FileStats entry */
static FileStats make_stats(const char* path, const Language* lang, int blank, int comment,
                            int code) {
    FileStats fs;
    fs.filepath = path;
    fs.lang = lang;
    fs.ignore_reason = NULL;
    fs.counts.blank = blank;
    fs.counts.comment = comment;
    fs.counts.code = code;
    return fs;
}

/* Helper: redirect stdout to a temp file via dup2, run a function, restore, read content */
static char* capture_to_temp(void (*fn)(void)) {
    char tmpname[] = "/tmp/rloc_test_output_XXXXXX";
    int fd = mkstemp(tmpname);
    if (fd < 0) return NULL;
    close(fd);

    int orig_fd = dup(STDOUT_FILENO);
    if (orig_fd < 0) {
        remove(tmpname);
        return NULL;
    }

    FILE* tmp = fopen(tmpname, "w");
    if (!tmp) {
        close(orig_fd);
        remove(tmpname);
        return NULL;
    }

    int tmp_fd = fileno(tmp);
    dup2(tmp_fd, STDOUT_FILENO);

    fn();

    fflush(stdout);
    dup2(orig_fd, STDOUT_FILENO);
    fclose(tmp);
    close(orig_fd);

    FILE* f = fopen(tmpname, "r");
    if (!f) {
        remove(tmpname);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(len + 1);
    if (buf) {
        fread(buf, 1, len, f);
        buf[len] = '\0';
    }
    fclose(f);
    remove(tmpname);
    return buf;
}

/* ---- output_text tests ---- */

static void _text_empty_fn(void) {
    FileStats files[1];
    output_text(files, 0, 0.5);
}
TEST(test_output_text_empty) {
    char* buf = capture_to_temp(_text_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "text file") != NULL);
    ASSERT_TRUE(strstr(buf, "unique file") != NULL);
    ASSERT_TRUE(strstr(buf, "Language") != NULL);
    ASSERT_TRUE(strstr(buf, "SUM") != NULL);
    free(buf);
}

static void _text_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_text(files, 1, 1.0);
}
TEST(test_output_text_single_file) {
    char* buf = capture_to_temp(_text_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "1 text file") != NULL);
    ASSERT_TRUE(strstr(buf, "1 unique file") != NULL);
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "50") != NULL);
    free(buf);
}

static void _text_multi_fn(void) {
    const Language* c = lang_c();
    const Language* py = lang_python();
    FileStats files[3];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    files[1] = make_stats("util.c", c, 3, 5, 30);
    files[2] = make_stats("script.py", py, 2, 8, 20);
    output_text(files, 3, 0.5);
}
TEST(test_output_text_multiple_files) {
    char* buf = capture_to_temp(_text_multi_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "3 text files") != NULL);
    ASSERT_TRUE(strstr(buf, "3 unique files") != NULL);
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "Python") != NULL);
    ASSERT_TRUE(strstr(buf, "80") != NULL);
    ASSERT_TRUE(strstr(buf, "20") != NULL);
    free(buf);
}

static void _text_ignored_fn(void) {
    const Language* c = lang_c();
    FileStats files[2];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    files[1] = make_stats("binary.o", NULL, 0, 0, 0);
    files[1].ignore_reason = "binary";
    output_text(files, 2, 1.0);
}
TEST(test_output_text_with_ignored_files) {
    char* buf = capture_to_temp(_text_ignored_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "2 text files") != NULL);
    ASSERT_TRUE(strstr(buf, "1 unique file") != NULL);
    ASSERT_TRUE(strstr(buf, "1 files ignored") != NULL);
    free(buf);
}

/* ---- output_text_by_file tests ---- */

static void _text_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_text_by_file(files, 1, 1.0);
}
TEST(test_output_text_by_file_single) {
    char* buf = capture_to_temp(_text_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "1 text file") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    ASSERT_TRUE(strstr(buf, "File") != NULL);
    free(buf);
}

/* ---- output_json tests ---- */

static void _json_empty_fn(void) {
    FileStats files[1];
    output_json(files, 0, 0.5);
}
TEST(test_output_json_empty) {
    char* buf = capture_to_temp(_json_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "{") != NULL);
    ASSERT_TRUE(strstr(buf, "\"header\"") != NULL);
    ASSERT_TRUE(strstr(buf, "\"cloc_url\"") != NULL);
    ASSERT_TRUE(strstr(buf, "\"n_files\"") != NULL);
    free(buf);
}

static void _json_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_json(files, 1, 1.0);
}
TEST(test_output_json_single_file) {
    char* buf = capture_to_temp(_json_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "\"C\"") != NULL);
    ASSERT_TRUE(strstr(buf, "\"nFiles\"") != NULL);
    ASSERT_TRUE(strstr(buf, "\"code\"") != NULL);
    free(buf);
}

static void _json_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_json_by_file(files, 1, 1.0);
}
TEST(test_output_json_by_file) {
    char* buf = capture_to_temp(_json_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "\"main.c\"") != NULL);
    ASSERT_TRUE(strstr(buf, "\"language\"") != NULL);
    free(buf);
}

/* ---- output_csv tests ---- */

static void _csv_empty_fn(void) {
    FileStats files[1];
    output_csv(files, 0, 0.5);
}
TEST(test_output_csv_empty) {
    char* buf = capture_to_temp(_csv_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "files,language,blank,comment,code") != NULL);
    free(buf);
}

static void _csv_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_csv(files, 1, 1.0);
}
TEST(test_output_csv_single_file) {
    char* buf = capture_to_temp(_csv_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "files,language,blank,comment,code") != NULL);
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "SUM") != NULL);
    free(buf);
}

static void _csv_multi_fn(void) {
    const Language* c = lang_c();
    const Language* py = lang_python();
    FileStats files[3];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    files[1] = make_stats("util.c", c, 3, 5, 30);
    files[2] = make_stats("script.py", py, 2, 8, 20);
    output_csv(files, 3, 0.5);
}
TEST(test_output_csv_multiple_files) {
    char* buf = capture_to_temp(_csv_multi_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "Python") != NULL);
    free(buf);
}

static void _csv_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_csv_by_file(files, 1, 1.0);
}
TEST(test_output_csv_by_file) {
    char* buf = capture_to_temp(_csv_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "language,filename,blank,comment,code") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    free(buf);
}

/* ---- output_md tests ---- */

static void _md_empty_fn(void) {
    FileStats files[1];
    output_md(files, 0, 0.5);
}
TEST(test_output_md_empty) {
    char* buf = capture_to_temp(_md_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "Language") != NULL);
    ASSERT_TRUE(strstr(buf, "|") != NULL);
    ASSERT_TRUE(strstr(buf, "SUM:") != NULL);
    free(buf);
}

static void _md_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_md(files, 1, 1.0);
}
TEST(test_output_md_single_file) {
    char* buf = capture_to_temp(_md_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "50") != NULL);
    free(buf);
}

static void _md_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_md_by_file(files, 1, 1.0);
}
TEST(test_output_md_by_file) {
    char* buf = capture_to_temp(_md_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "File") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    free(buf);
}

/* ---- output_yaml tests ---- */

static void _yaml_empty_fn(void) {
    FileStats files[1];
    output_yaml(files, 0, 0.5);
}
TEST(test_output_yaml_empty) {
    char* buf = capture_to_temp(_yaml_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "---") != NULL);
    ASSERT_TRUE(strstr(buf, "header:") != NULL);
    ASSERT_TRUE(strstr(buf, "languages:") != NULL);
    ASSERT_TRUE(strstr(buf, "SUM:") != NULL);
    free(buf);
}

static void _yaml_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_yaml(files, 1, 1.0);
}
TEST(test_output_yaml_single_file) {
    char* buf = capture_to_temp(_yaml_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "name: C") != NULL);
    ASSERT_TRUE(strstr(buf, "code: 50") != NULL);
    free(buf);
}

static void _yaml_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_yaml_by_file(files, 1, 1.0);
}
TEST(test_output_yaml_by_file) {
    char* buf = capture_to_temp(_yaml_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "---") != NULL);
    ASSERT_TRUE(strstr(buf, "files:") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    free(buf);
}

/* ---- output_xml tests ---- */

static void _xml_empty_fn(void) {
    FileStats files[1];
    output_xml(files, 0, 0.5);
}
TEST(test_output_xml_empty) {
    char* buf = capture_to_temp(_xml_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "<?xml version=\"1.0\"") != NULL);
    ASSERT_TRUE(strstr(buf, "<results>") != NULL);
    ASSERT_TRUE(strstr(buf, "<header>") != NULL);
    ASSERT_TRUE(strstr(buf, "</results>") != NULL);
    free(buf);
}

static void _xml_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_xml(files, 1, 1.0);
}
TEST(test_output_xml_single_file) {
    char* buf = capture_to_temp(_xml_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "<language") != NULL);
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "code=\"50\"") != NULL);
    free(buf);
}

static void _xml_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_xml_by_file(files, 1, 1.0);
}
TEST(test_output_xml_by_file) {
    char* buf = capture_to_temp(_xml_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "<?xml version=\"1.0\"") != NULL);
    ASSERT_TRUE(strstr(buf, "<file") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    free(buf);
}

/* ---- output_html tests ---- */

static void _html_empty_fn(void) {
    FileStats files[1];
    output_html(files, 0, 0.5);
}
TEST(test_output_html_empty) {
    char* buf = capture_to_temp(_html_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "<!DOCTYPE html>") != NULL);
    ASSERT_TRUE(strstr(buf, "<html>") != NULL);
    ASSERT_TRUE(strstr(buf, "<table>") != NULL);
    ASSERT_TRUE(strstr(buf, "</html>") != NULL);
    free(buf);
}

static void _html_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_html(files, 1, 1.0);
}
TEST(test_output_html_single_file) {
    char* buf = capture_to_temp(_html_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "rloc Report") != NULL);
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    ASSERT_TRUE(strstr(buf, "50") != NULL);
    ASSERT_TRUE(strstr(buf, "SUM") != NULL);
    free(buf);
}

static void _html_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_html_by_file(files, 1, 1.0);
}
TEST(test_output_html_by_file) {
    char* buf = capture_to_temp(_html_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "<!DOCTYPE html>") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    free(buf);
}

/* ---- output_sql tests ---- */

static void _sql_empty_fn(void) {
    FileStats files[1];
    output_sql(files, 0, 0.5, "test_project");
}
TEST(test_output_sql_empty) {
    char* buf = capture_to_temp(_sql_empty_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "CREATE TABLE") != NULL);
    ASSERT_TRUE(strstr(buf, "metadata") != NULL);
    ASSERT_TRUE(strstr(buf, "results") != NULL);
    ASSERT_TRUE(strstr(buf, "test_project") != NULL);
    free(buf);
}

static void _sql_single_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_sql(files, 1, 1.0, "my_project");
}
TEST(test_output_sql_single_file) {
    char* buf = capture_to_temp(_sql_single_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "my_project") != NULL);
    ASSERT_TRUE(strstr(buf, "INSERT INTO") != NULL);
    ASSERT_TRUE(strstr(buf, "C") != NULL);
    free(buf);
}

static void _sql_by_file_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_sql_by_file(files, 1, 1.0, "my_project");
}
TEST(test_output_sql_by_file) {
    char* buf = capture_to_temp(_sql_by_file_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "CREATE TABLE") != NULL);
    ASSERT_TRUE(strstr(buf, "files") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    free(buf);
}

/* ---- output_by_file_by_lang tests ---- */

static void _by_file_by_lang_fn(void) {
    const Language* c = lang_c();
    FileStats files[1];
    files[0] = make_stats("main.c", c, 5, 10, 50);
    output_by_file_by_lang(files, 1, 1.0);
}
TEST(test_output_by_file_by_lang) {
    char* buf = capture_to_temp(_by_file_by_lang_fn);
    ASSERT_TRUE(buf != NULL);
    if (!buf) return;
    ASSERT_TRUE(strstr(buf, "File") != NULL);
    ASSERT_TRUE(strstr(buf, "Language") != NULL);
    ASSERT_TRUE(strstr(buf, "main.c") != NULL);
    ASSERT_TRUE(strstr(buf, "SUM:") != NULL);
    free(buf);
}

/* ===================== main ===================== */

int main(void) {
    /* output_text */
    register_test("test_output_text_empty", test_func_test_output_text_empty);
    register_test("test_output_text_single_file", test_func_test_output_text_single_file);
    register_test("test_output_text_multiple_files", test_func_test_output_text_multiple_files);
    register_test("test_output_text_with_ignored_files",
                  test_func_test_output_text_with_ignored_files);

    /* output_text_by_file */
    register_test("test_output_text_by_file_single", test_func_test_output_text_by_file_single);

    /* output_json */
    register_test("test_output_json_empty", test_func_test_output_json_empty);
    register_test("test_output_json_single_file", test_func_test_output_json_single_file);
    register_test("test_output_json_by_file", test_func_test_output_json_by_file);

    /* output_csv */
    register_test("test_output_csv_empty", test_func_test_output_csv_empty);
    register_test("test_output_csv_single_file", test_func_test_output_csv_single_file);
    register_test("test_output_csv_multiple_files", test_func_test_output_csv_multiple_files);
    register_test("test_output_csv_by_file", test_func_test_output_csv_by_file);

    /* output_md */
    register_test("test_output_md_empty", test_func_test_output_md_empty);
    register_test("test_output_md_single_file", test_func_test_output_md_single_file);
    register_test("test_output_md_by_file", test_func_test_output_md_by_file);

    /* output_yaml */
    register_test("test_output_yaml_empty", test_func_test_output_yaml_empty);
    register_test("test_output_yaml_single_file", test_func_test_output_yaml_single_file);
    register_test("test_output_yaml_by_file", test_func_test_output_yaml_by_file);

    /* output_xml */
    register_test("test_output_xml_empty", test_func_test_output_xml_empty);
    register_test("test_output_xml_single_file", test_func_test_output_xml_single_file);
    register_test("test_output_xml_by_file", test_func_test_output_xml_by_file);

    /* output_html */
    register_test("test_output_html_empty", test_func_test_output_html_empty);
    register_test("test_output_html_single_file", test_func_test_output_html_single_file);
    register_test("test_output_html_by_file", test_func_test_output_html_by_file);

    /* output_sql */
    register_test("test_output_sql_empty", test_func_test_output_sql_empty);
    register_test("test_output_sql_single_file", test_func_test_output_sql_single_file);
    register_test("test_output_sql_by_file", test_func_test_output_sql_by_file);

    /* additional formats */
    register_test("test_output_by_file_by_lang", test_func_test_output_by_file_by_lang);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
