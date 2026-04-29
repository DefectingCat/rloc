#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../output_writer.h"
#include "test_framework.h"

TEST(test_output_writer_new_text) {
    OutputWriter* writer = output_writer_new(FORMAT_TEXT, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->write_header != NULL);
    ASSERT_TRUE(writer->write_footer != NULL);
    ASSERT_TRUE(writer->free != NULL);
    writer->free(writer);
}

TEST(test_output_writer_new_json) {
    OutputWriter* writer = output_writer_new(FORMAT_JSON, 1, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_JSON);
    ASSERT_TRUE(writer->by_file == 1);
    writer->free(writer);
}

TEST(test_output_writer_new_csv) {
    OutputWriter* writer = output_writer_new(FORMAT_CSV, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_CSV);
    writer->free(writer);
}

TEST(test_output_writer_new_md) {
    OutputWriter* writer = output_writer_new(FORMAT_MD, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_MD);
    writer->free(writer);
}

TEST(test_output_writer_new_yaml) {
    OutputWriter* writer = output_writer_new(FORMAT_YAML, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_YAML);
    writer->free(writer);
}

TEST(test_output_writer_new_xml) {
    OutputWriter* writer = output_writer_new(FORMAT_XML, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_XML);
    writer->free(writer);
}

TEST(test_output_writer_new_html) {
    OutputWriter* writer = output_writer_new(FORMAT_HTML, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_HTML);
    writer->free(writer);
}

TEST(test_output_writer_new_sql) {
    OutputWriter* writer = output_writer_new(FORMAT_SQL, 0, 0);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->format == FORMAT_SQL);
    writer->free(writer);
}

TEST(test_output_writer_by_file_by_lang) {
    OutputWriter* writer = output_writer_new(FORMAT_TEXT, 1, 1);
    ASSERT_TRUE(writer != NULL);
    ASSERT_TRUE(writer->by_file == 1);
    ASSERT_TRUE(writer->by_file_by_lang == 1);
    writer->free(writer);
}

int main(void) {
    register_test("test_output_writer_new_text", test_func_test_output_writer_new_text);
    register_test("test_output_writer_new_json", test_func_test_output_writer_new_json);
    register_test("test_output_writer_new_csv", test_func_test_output_writer_new_csv);
    register_test("test_output_writer_new_md", test_func_test_output_writer_new_md);
    register_test("test_output_writer_new_yaml", test_func_test_output_writer_new_yaml);
    register_test("test_output_writer_new_xml", test_func_test_output_writer_new_xml);
    register_test("test_output_writer_new_html", test_func_test_output_writer_new_html);
    register_test("test_output_writer_new_sql", test_func_test_output_writer_new_sql);
    register_test("test_output_writer_by_file_by_lang", test_func_test_output_writer_by_file_by_lang);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}