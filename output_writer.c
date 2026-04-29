#include "output_writer.h"

#include <stdlib.h>

#include "output.h"

/* Text format writer implementation */
static void text_write_header(OutputWriter* self) {
    (void)self;
    /* Text format doesn't need a header */
}

static void text_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
    /* Footer is written by output_text with full results */
}

static void text_free(OutputWriter* self) {
    free(self);
}

/* JSON format writer implementation */
static void json_write_header(OutputWriter* self) {
    (void)self;
}

static void json_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void json_free(OutputWriter* self) {
    free(self);
}

/* CSV format writer implementation */
static void csv_write_header(OutputWriter* self) {
    (void)self;
}

static void csv_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void csv_free(OutputWriter* self) {
    free(self);
}

/* MD format writer implementation */
static void md_write_header(OutputWriter* self) {
    (void)self;
}

static void md_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void md_free(OutputWriter* self) {
    free(self);
}

/* YAML format writer implementation */
static void yaml_write_header(OutputWriter* self) {
    (void)self;
}

static void yaml_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void yaml_free(OutputWriter* self) {
    free(self);
}

/* XML format writer implementation */
static void xml_write_header(OutputWriter* self) {
    (void)self;
}

static void xml_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void xml_free(OutputWriter* self) {
    free(self);
}

/* HTML format writer implementation */
static void html_write_header(OutputWriter* self) {
    (void)self;
}

static void html_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void html_free(OutputWriter* self) {
    free(self);
}

/* SQL format writer implementation */
static void sql_write_header(OutputWriter* self) {
    (void)self;
}

static void sql_write_footer(OutputWriter* self, double elapsed_sec) {
    (void)self;
    (void)elapsed_sec;
}

static void sql_free(OutputWriter* self) {
    free(self);
}

OutputWriter* output_writer_new(int format, int by_file, int by_file_by_lang) {
    OutputWriter* writer = malloc(sizeof(OutputWriter));
    if (!writer) return NULL;

    writer->format = format;
    writer->by_file = by_file;
    writer->by_file_by_lang = by_file_by_lang;
    writer->out = stdout;
    writer->data = NULL;
    writer->write_result = NULL;  /* Results are written via batch functions */

    switch (format) {
        case FORMAT_JSON:
            writer->write_header = json_write_header;
            writer->write_footer = json_write_footer;
            writer->free = json_free;
            break;
        case FORMAT_CSV:
            writer->write_header = csv_write_header;
            writer->write_footer = csv_write_footer;
            writer->free = csv_free;
            break;
        case FORMAT_MD:
            writer->write_header = md_write_header;
            writer->write_footer = md_write_footer;
            writer->free = md_free;
            break;
        case FORMAT_YAML:
            writer->write_header = yaml_write_header;
            writer->write_footer = yaml_write_footer;
            writer->free = yaml_free;
            break;
        case FORMAT_XML:
            writer->write_header = xml_write_header;
            writer->write_footer = xml_write_footer;
            writer->free = xml_free;
            break;
        case FORMAT_HTML:
            writer->write_header = html_write_header;
            writer->write_footer = html_write_footer;
            writer->free = html_free;
            break;
        case FORMAT_SQL:
            writer->write_header = sql_write_header;
            writer->write_footer = sql_write_footer;
            writer->free = sql_free;
            break;
        case FORMAT_TEXT:
        default:
            writer->write_header = text_write_header;
            writer->write_footer = text_write_footer;
            writer->free = text_free;
            break;
    }

    return writer;
}
