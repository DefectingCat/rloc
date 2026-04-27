#include <stdio.h>
#include <string.h>

#include "../lang_defs.h"
#include "../language.h"
#include "test_framework.h"

/* ------------------------------------------------------------------ */
/*  Helper macros for inspecting g_languages array                    */
/* ------------------------------------------------------------------ */

/* Find a language by exact name in g_languages */
static const Language* find_lang(const char* name) {
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        if (strcmp(g_languages[i].name, name) == 0) {
            return &g_languages[i];
        }
    }
    return NULL;
}

/* Check if a language has a single-line (inline) comment filter */
static int lang_has_single_line_comment(const Language* lang) {
    if (!lang || !lang->generic_filters) return 0;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE) return 1;
    }
    return 0;
}

/* Check if a language has a block comment filter */
static int lang_has_block_comment(const Language* lang) {
    if (!lang || !lang->generic_filters) return 0;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_BETWEEN) return 1;
    }
    return 0;
}

/* Get the single-line comment marker (first INLINE filter) */
static const char* lang_get_single_line_comment(const Language* lang) {
    if (!lang || !lang->generic_filters) return NULL;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE) {
            return lang->generic_filters[i].pattern_open;
        }
    }
    return NULL;
}

/* Get the block comment start marker (first BETWEEN filter) */
static const char* lang_get_block_comment_start(const Language* lang) {
    if (!lang || !lang->generic_filters) return NULL;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_BETWEEN) {
            return lang->generic_filters[i].pattern_open;
        }
    }
    return NULL;
}

/* Get the block comment end marker (first BETWEEN filter) */
static const char* lang_get_block_comment_end(const Language* lang) {
    if (!lang || !lang->generic_filters) return NULL;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_BETWEEN) {
            return lang->generic_filters[i].pattern_close;
        }
    }
    return NULL;
}

/* Check if a language needs string literal stripping */
static int lang_needs_string_stripping(const Language* lang) {
    if (!lang) return 0;
    if (!lang->str_delimiters) return 0;
    if (!lang->generic_filters || lang->generic_filter_count == 0) return 0;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE ||
            lang->generic_filters[i].type == FILTER_REMOVE_BETWEEN) {
            return 1;
        }
    }
    return 0;
}

/* Check if a language has both single-line and block comments */
static int lang_has_both_comment_types(const Language* lang) {
    return lang_has_single_line_comment(lang) && lang_has_block_comment(lang);
}

/* Check if a language has multiple inline comment markers */
static int lang_has_multiple_inline_comments(const Language* lang) {
    if (!lang || !lang->generic_filters) return 0;
    int count = 0;
    for (size_t i = 0; i < lang->generic_filter_count; i++) {
        if (lang->generic_filters[i].type == FILTER_REMOVE_INLINE) count++;
    }
    return count;
}

/* ------------------------------------------------------------------ */
/*  Test: total number of built-in languages                          */
/* ------------------------------------------------------------------ */
TEST(test_num_languages) {
    ASSERT_EQ(NUM_LANGUAGES, 25);
}

/* ------------------------------------------------------------------ */
/*  Test: all languages have valid names                              */
/* ------------------------------------------------------------------ */
TEST(test_all_languages_have_names) {
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        ASSERT_TRUE(g_languages[i].name != NULL);
        ASSERT_TRUE(strlen(g_languages[i].name) > 0);
    }
}

/* ------------------------------------------------------------------ */
/*  Test: C language -- // single-line, block comments, both delimiters  */
/* ------------------------------------------------------------------ */
TEST(test_c_comment_rules) {
    const Language* lang = find_lang("C");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
    ASSERT_STR(lang->str_delimiters, "\"'");
    ASSERT_STR(lang->str_escape, "\\");
}

/* ------------------------------------------------------------------ */
/*  Test: C++ -- same comment style as C                              */
/* ------------------------------------------------------------------ */
TEST(test_cpp_comment_rules) {
    const Language* lang = find_lang("C++");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_both_comment_types(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: Java -- same comment style as C                             */
/* ------------------------------------------------------------------ */
TEST(test_java_comment_rules) {
    const Language* lang = find_lang("Java");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_both_comment_types(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: JavaScript -- // single-line, block comments  */
/* ------------------------------------------------------------------ */
TEST(test_javascript_comment_rules) {
    const Language* lang = find_lang("JavaScript");
    ASSERT_TRUE(lang != NULL);
    /* javascript_filters only defines // inline, no block comment filter */
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
}

/* ------------------------------------------------------------------ */
/*  Test: TypeScript -- same as JS/C-style                            */
/* ------------------------------------------------------------------ */
TEST(test_typescript_comment_rules) {
    const Language* lang = find_lang("TypeScript");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_both_comment_types(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: Rust -- // and block comments */
/* ------------------------------------------------------------------ */
TEST(test_rust_comment_rules) {
    const Language* lang = find_lang("Rust");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_both_comment_types(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: Go -- only // single-line, no block comments                */
/* ------------------------------------------------------------------ */
TEST(test_go_comment_rules) {
    const Language* lang = find_lang("Go");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
}

/* ------------------------------------------------------------------ */
/*  Test: Swift -- only // single-line                                */
/* ------------------------------------------------------------------ */
TEST(test_swift_comment_rules) {
    const Language* lang = find_lang("Swift");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
}

/* ------------------------------------------------------------------ */
/*  Test: Kotlin -- only // single-line                               */
/* ------------------------------------------------------------------ */
TEST(test_kotlin_comment_rules) {
    const Language* lang = find_lang("Kotlin");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
}

/* ------------------------------------------------------------------ */
/*  Test: C# -- // and block comments */
/* ------------------------------------------------------------------ */
TEST(test_csharp_comment_rules) {
    const Language* lang = find_lang("C#");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_both_comment_types(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: Python -- # single-line, no block comments                  */
/* ------------------------------------------------------------------ */
TEST(test_python_comment_rules) {
    const Language* lang = find_lang("Python");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
    ASSERT_STR(lang->str_delimiters, "\"'");
}

/* ------------------------------------------------------------------ */
/*  Test: Ruby -- # single-line, no block comments                    */
/* ------------------------------------------------------------------ */
TEST(test_ruby_comment_rules) {
    const Language* lang = find_lang("Ruby");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
    /* Ruby uses only double-quote delimiters (no single quotes) */
    ASSERT_STR(lang->str_delimiters, "\"");
}

/* ------------------------------------------------------------------ */
/*  Test: Shell -- # single-line, no block comments                   */
/* ------------------------------------------------------------------ */
TEST(test_shell_comment_rules) {
    const Language* lang = find_lang("Shell");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
}

/* ------------------------------------------------------------------ */
/*  Test: Perl -- # single-line, no block comments                    */
/* ------------------------------------------------------------------ */
TEST(test_perl_comment_rules) {
    const Language* lang = find_lang("Perl");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
    ASSERT_STR(lang->str_delimiters, "\"");
}

/* ------------------------------------------------------------------ */
/*  Test: PHP -- // and # single-line, block comments */
/* ------------------------------------------------------------------ */
TEST(test_php_comment_rules) {
    const Language* lang = find_lang("PHP");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(lang_has_block_comment(lang));
    /* PHP has two inline comment markers: // and # */
    ASSERT_EQ(lang_has_multiple_inline_comments(lang), 2);
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: SQL -- -- single-line, no block comments                    */
/* ------------------------------------------------------------------ */
TEST(test_sql_comment_rules) {
    const Language* lang = find_lang("SQL");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "--");
}

/* ------------------------------------------------------------------ */
/*  Test: TOML -- # single-line, no block comments                    */
/* ------------------------------------------------------------------ */
TEST(test_toml_comment_rules) {
    const Language* lang = find_lang("TOML");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
    ASSERT_STR(lang->str_delimiters, "\"");
}

/* ------------------------------------------------------------------ */
/*  Test: YAML -- # single-line, no block comments                    */
/* ------------------------------------------------------------------ */
TEST(test_yaml_comment_rules) {
    const Language* lang = find_lang("YAML");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
    ASSERT_STR(lang->str_delimiters, "\"");
}

/* ------------------------------------------------------------------ */
/*  Test: CSS -- only block comments, no single-line comments            */
/* ------------------------------------------------------------------ */
TEST(test_css_comment_rules) {
    const Language* lang = find_lang("CSS");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(!lang_has_single_line_comment(lang));
    ASSERT_TRUE(lang_has_block_comment(lang));
    ASSERT_STR(lang_get_block_comment_start(lang), "/*");
    ASSERT_STR(lang_get_block_comment_end(lang), "*/");
}

/* ------------------------------------------------------------------ */
/*  Test: HTML -- <!-- --> block comments                             */
/* ------------------------------------------------------------------ */
TEST(test_html_comment_rules) {
    const Language* lang = find_lang("HTML");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(!lang_has_single_line_comment(lang));
    ASSERT_TRUE(lang_has_block_comment(lang));
    ASSERT_STR(lang_get_block_comment_start(lang), "<!--");
    ASSERT_STR(lang_get_block_comment_end(lang), "-->");
}

/* ------------------------------------------------------------------ */
/*  Test: XML -- <!-- --> block comments (same as HTML)               */
/* ------------------------------------------------------------------ */
TEST(test_xml_comment_rules) {
    const Language* lang = find_lang("XML");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(!lang_has_single_line_comment(lang));
    ASSERT_TRUE(lang_has_block_comment(lang));
    ASSERT_STR(lang_get_block_comment_start(lang), "<!--");
    ASSERT_STR(lang_get_block_comment_end(lang), "-->");
}

/* ------------------------------------------------------------------ */
/*  Test: JSON -- no comments defined                                 */
/* ------------------------------------------------------------------ */
TEST(test_json_no_comments) {
    const Language* lang = find_lang("JSON");
    ASSERT_TRUE(lang != NULL);
    ASSERT_EQ(lang->generic_filter_count, 0);
    ASSERT_TRUE(!lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_TRUE(lang_get_single_line_comment(lang) == NULL);
    ASSERT_TRUE(lang_get_block_comment_start(lang) == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: Markdown -- no comments defined                             */
/* ------------------------------------------------------------------ */
TEST(test_markdown_no_comments) {
    const Language* lang = find_lang("Markdown");
    ASSERT_TRUE(lang != NULL);
    ASSERT_EQ(lang->generic_filter_count, 0);
    ASSERT_TRUE(!lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_TRUE(lang->str_delimiters == NULL);
    ASSERT_TRUE(lang->str_escape == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: Vue -- uses JS filters (// and block comments) */
/* ------------------------------------------------------------------ */
TEST(test_vue_comment_rules) {
    const Language* lang = find_lang("Vue");
    ASSERT_TRUE(lang != NULL);
    /* Vue uses javascript_filters which only defines // inline */
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "//");
}

/* ------------------------------------------------------------------ */
/*  Test: Config language -- # single-line via shell filters,          */
/*         no string delimiters (dotfiles)                             */
/* ------------------------------------------------------------------ */
TEST(test_config_comment_rules) {
    const Language* lang = find_lang("Config");
    ASSERT_TRUE(lang != NULL);
    ASSERT_TRUE(lang_has_single_line_comment(lang));
    ASSERT_TRUE(!lang_has_block_comment(lang));
    ASSERT_STR(lang_get_single_line_comment(lang), "#");
    /* Config has no string delimiters */
    ASSERT_TRUE(lang->str_delimiters == NULL);
    ASSERT_TRUE(lang->str_escape == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: string delimiter variations across languages                 */
/* ------------------------------------------------------------------ */
TEST(test_string_delimiters_double_and_single) {
    /* Languages with both " and ' delimiters */
    const char* both_quote[] = {"Python", "JavaScript", "C", "C++", "Java", "Go", "Rust",
                                "TypeScript", "PHP", "C#", "Swift", "Kotlin", "Shell", "CSS",
                                "HTML", "SQL", "Vue", "XML"};
    for (size_t i = 0; i < sizeof(both_quote) / sizeof(both_quote[0]); i++) {
        const Language* lang = find_lang(both_quote[i]);
        ASSERT_TRUE(lang != NULL);
        ASSERT_TRUE(lang->str_delimiters != NULL);
        ASSERT_TRUE(strstr(lang->str_delimiters, "\"") != NULL);
        ASSERT_TRUE(strstr(lang->str_delimiters, "'") != NULL);
    }
}

/* ------------------------------------------------------------------ */
/*  Test: string delimiter single-quote only languages                */
/* ------------------------------------------------------------------ */
TEST(test_string_delimiters_double_only) {
    /* Languages with only " delimiter */
    const char* double_only[] = {"Ruby", "Perl", "JSON", "TOML", "YAML"};
    for (size_t i = 0; i < sizeof(double_only) / sizeof(double_only[0]); i++) {
        const Language* lang = find_lang(double_only[i]);
        ASSERT_TRUE(lang != NULL);
        ASSERT_TRUE(lang->str_delimiters != NULL);
        ASSERT_TRUE(strstr(lang->str_delimiters, "\"") != NULL);
        ASSERT_TRUE(strstr(lang->str_delimiters, "'") == NULL);
    }
}

/* ------------------------------------------------------------------ */
/*  Test: languages that need string stripping                         */
/* ------------------------------------------------------------------ */
TEST(test_languages_needing_string_stripping) {
    /* Languages with both filters AND str_delimiters need stripping */
    const char* needs_strip[] = {"Python", "JavaScript", "C", "C++", "Java", "Go", "Rust",
                                 "TypeScript", "Ruby", "PHP", "C#", "Swift", "Kotlin", "Shell",
                                 "Perl", "CSS", "HTML", "SQL", "TOML", "Vue", "XML", "YAML"};
    for (size_t i = 0; i < sizeof(needs_strip) / sizeof(needs_strip[0]); i++) {
        const Language* lang = find_lang(needs_strip[i]);
        ASSERT_TRUE(lang != NULL);
        ASSERT_TRUE(lang_needs_string_stripping(lang));
    }
}

/* ------------------------------------------------------------------ */
/*  Test: languages that do NOT need string stripping                  */
/* ------------------------------------------------------------------ */
TEST(test_languages_not_needing_string_stripping) {
    /* JSON has delimiters but no filters; Markdown has neither */
    const char* no_strip[] = {"JSON", "Markdown", "Config"};
    for (size_t i = 0; i < sizeof(no_strip) / sizeof(no_strip[0]); i++) {
        const Language* lang = find_lang(no_strip[i]);
        ASSERT_TRUE(lang != NULL);
        ASSERT_TRUE(!lang_needs_string_stripping(lang));
    }
}

/* ------------------------------------------------------------------ */
/*  Test: get_language_by_name with exact case                        */
/* ------------------------------------------------------------------ */
TEST(test_get_language_by_name_exact) {
    const Language* lang = get_language_by_name("Python");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Python");
}

/* ------------------------------------------------------------------ */
/*  Test: get_language_by_name case-insensitive                       */
/* ------------------------------------------------------------------ */
TEST(test_get_language_by_name_case_insensitive) {
    const Language* lang = get_language_by_name("python");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Python");

    lang = get_language_by_name("PYTHON");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Python");

    lang = get_language_by_name("c++");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C++");
}

/* ------------------------------------------------------------------ */
/*  Test: get_language_by_name with unknown name returns NULL         */
/* ------------------------------------------------------------------ */
TEST(test_get_language_by_name_unknown) {
    const Language* lang = get_language_by_name("NonexistentLang");
    ASSERT_TRUE(lang == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: get_language_by_name with NULL returns NULL                 */
/* ------------------------------------------------------------------ */
TEST(test_get_language_by_name_null) {
    const Language* lang = get_language_by_name(NULL);
    ASSERT_TRUE(lang == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: detect_language returns correct language for C header        */
/* ------------------------------------------------------------------ */
TEST(test_detect_c_header) {
    const Language* lang = detect_language("test.h");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C");
}

/* ------------------------------------------------------------------ */
/*  Test: detect_language for Config dotfiles                          */
/* ------------------------------------------------------------------ */
TEST(test_detect_config_by_filename) {
    const Language* lang = detect_language(".gitignore");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Config");

    lang = detect_language(".clang-format");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Config");
}

/* ------------------------------------------------------------------ */
/*  Test: all escape chars are backslash where present                 */
/* ------------------------------------------------------------------ */
TEST(test_all_escape_chars_are_backslash) {
    for (int i = 0; i < NUM_LANGUAGES; i++) {
        if (g_languages[i].str_escape != NULL) {
            ASSERT_STR(g_languages[i].str_escape, "\\");
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Test: PHP has three filter types (// inline, # inline, block comments) */
/* ------------------------------------------------------------------ */
TEST(test_php_has_three_filters) {
    const Language* lang = find_lang("PHP");
    ASSERT_TRUE(lang != NULL);
    ASSERT_EQ(lang->generic_filter_count, 3);
    ASSERT_EQ(lang_has_multiple_inline_comments(lang), 2);
    ASSERT_TRUE(lang_has_block_comment(lang));
}

/* ------------------------------------------------------------------ */
/*  Test: lang_defs_find_by_name for built-in languages               */
/* ------------------------------------------------------------------ */
TEST(test_lang_defs_find_by_name_builtin) {
    const Language* lang = lang_defs_find_by_name("C");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "C");

    lang = lang_defs_find_by_name("Rust");
    ASSERT_TRUE(lang != NULL);
    ASSERT_STR(lang->name, "Rust");
}

/* ------------------------------------------------------------------ */
/*  Test: lang_defs_find_by_name returns NULL for unknown             */
/* ------------------------------------------------------------------ */
TEST(test_lang_defs_find_by_name_unknown) {
    const Language* lang = lang_defs_find_by_name("Brainfuck");
    ASSERT_TRUE(lang == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: lang_defs_find_by_name returns NULL for NULL input          */
/* ------------------------------------------------------------------ */
TEST(test_lang_defs_find_by_name_null_input) {
    const Language* lang = lang_defs_find_by_name(NULL);
    ASSERT_TRUE(lang == NULL);
}

/* ------------------------------------------------------------------ */
/*  Test: custom language registry starts empty                       */
/* ------------------------------------------------------------------ */
TEST(test_custom_registry_starts_empty) {
    lang_defs_clear_custom();
    ASSERT_EQ(g_custom_language_count, 0);
}

/* ------------------------------------------------------------------ */
/*  Test: extensions for multi-extension languages                     */
/* ------------------------------------------------------------------ */
TEST(test_multi_extension_languages) {
    /* C has c,h */
    const Language* c_lang = find_lang("C");
    ASSERT_STR(c_lang->extensions, "c,h");

    /* TypeScript has ts,tsx */
    const Language* ts_lang = find_lang("TypeScript");
    ASSERT_STR(ts_lang->extensions, "ts,tsx");

    /* YAML has yaml,yml */
    const Language* yaml_lang = find_lang("YAML");
    ASSERT_STR(yaml_lang->extensions, "yaml,yml");

    /* Shell has sh,bash,zsh,ksh */
    const Language* sh_lang = find_lang("Shell");
    ASSERT_STR(sh_lang->extensions, "sh,bash,zsh,ksh");
}

/* ------------------------------------------------------------------ */
/*  Test: shebang detection for scripting languages                    */
/* ------------------------------------------------------------------ */
TEST(test_languages_with_shebangs) {
    /* Python has python,python3 shebangs */
    const Language* py = find_lang("Python");
    ASSERT_TRUE(py->shebangs != NULL);
    ASSERT_STR(py->shebangs, "python,python3");

    /* Ruby has ruby shebang */
    const Language* rb = find_lang("Ruby");
    ASSERT_TRUE(rb->shebangs != NULL);
    ASSERT_STR(rb->shebangs, "ruby");

    /* Shell has bash,sh,zsh,ksh shebangs */
    const Language* sh = find_lang("Shell");
    ASSERT_TRUE(sh->shebangs != NULL);
    ASSERT_STR(sh->shebangs, "bash,sh,zsh,ksh");
}

/* ------------------------------------------------------------------ */
/*  Test: languages without shebangs                                  */
/* ------------------------------------------------------------------ */
TEST(test_languages_without_shebangs) {
    /* C has no shebangs */
    const Language* c_lang = find_lang("C");
    ASSERT_TRUE(c_lang->shebangs == NULL);

    /* JSON has no shebangs */
    const Language* json_lang = find_lang("JSON");
    ASSERT_TRUE(json_lang->shebangs == NULL);

    /* Rust has no shebangs */
    const Language* rust_lang = find_lang("Rust");
    ASSERT_TRUE(rust_lang->shebangs == NULL);
}

/* ------------------------------------------------------------------ */
/*  Main                                                              */
/* ------------------------------------------------------------------ */
int main(void) {
    register_test("test_num_languages", test_func_test_num_languages);
    register_test("test_all_languages_have_names", test_func_test_all_languages_have_names);

    /* C-family languages */
    register_test("test_c_comment_rules", test_func_test_c_comment_rules);
    register_test("test_cpp_comment_rules", test_func_test_cpp_comment_rules);
    register_test("test_java_comment_rules", test_func_test_java_comment_rules);
    register_test("test_javascript_comment_rules", test_func_test_javascript_comment_rules);
    register_test("test_typescript_comment_rules", test_func_test_typescript_comment_rules);
    register_test("test_rust_comment_rules", test_func_test_rust_comment_rules);
    register_test("test_go_comment_rules", test_func_test_go_comment_rules);
    register_test("test_swift_comment_rules", test_func_test_swift_comment_rules);
    register_test("test_kotlin_comment_rules", test_func_test_kotlin_comment_rules);
    register_test("test_csharp_comment_rules", test_func_test_csharp_comment_rules);

    /* Scripting languages */
    register_test("test_python_comment_rules", test_func_test_python_comment_rules);
    register_test("test_ruby_comment_rules", test_func_test_ruby_comment_rules);
    register_test("test_shell_comment_rules", test_func_test_shell_comment_rules);
    register_test("test_perl_comment_rules", test_func_test_perl_comment_rules);
    register_test("test_php_comment_rules", test_func_test_php_comment_rules);
    register_test("test_sql_comment_rules", test_func_test_sql_comment_rules);
    register_test("test_toml_comment_rules", test_func_test_toml_comment_rules);
    register_test("test_yaml_comment_rules", test_func_test_yaml_comment_rules);

    /* Markup / data languages */
    register_test("test_css_comment_rules", test_func_test_css_comment_rules);
    register_test("test_html_comment_rules", test_func_test_html_comment_rules);
    register_test("test_xml_comment_rules", test_func_test_xml_comment_rules);
    register_test("test_json_no_comments", test_func_test_json_no_comments);
    register_test("test_markdown_no_comments", test_func_test_markdown_no_comments);
    register_test("test_vue_comment_rules", test_func_test_vue_comment_rules);

    /* Config language */
    register_test("test_config_comment_rules", test_func_test_config_comment_rules);

    /* String delimiters */
    register_test("test_string_delimiters_double_and_single",
                  test_func_test_string_delimiters_double_and_single);
    register_test("test_string_delimiters_double_only",
                  test_func_test_string_delimiters_double_only);

    /* String stripping */
    register_test("test_languages_needing_string_stripping",
                  test_func_test_languages_needing_string_stripping);
    register_test("test_languages_not_needing_string_stripping",
                  test_func_test_languages_not_needing_string_stripping);

    /* Language lookup */
    register_test("test_get_language_by_name_exact", test_func_test_get_language_by_name_exact);
    register_test("test_get_language_by_name_case_insensitive",
                  test_func_test_get_language_by_name_case_insensitive);
    register_test("test_get_language_by_name_unknown", test_func_test_get_language_by_name_unknown);
    register_test("test_get_language_by_name_null", test_func_test_get_language_by_name_null);

    /* Filepath detection */
    register_test("test_detect_c_header", test_func_test_detect_c_header);
    register_test("test_detect_config_by_filename", test_func_test_detect_config_by_filename);

    /* Escape chars */
    register_test("test_all_escape_chars_are_backslash",
                  test_func_test_all_escape_chars_are_backslash);

    /* PHP specific */
    register_test("test_php_has_three_filters", test_func_test_php_has_three_filters);

    /* lang_defs API */
    register_test("test_lang_defs_find_by_name_builtin",
                  test_func_test_lang_defs_find_by_name_builtin);
    register_test("test_lang_defs_find_by_name_unknown",
                  test_func_test_lang_defs_find_by_name_unknown);
    register_test("test_lang_defs_find_by_name_null_input",
                  test_func_test_lang_defs_find_by_name_null_input);

    /* Custom language registry */
    register_test("test_custom_registry_starts_empty", test_func_test_custom_registry_starts_empty);

    /* Multi-extension and shebang tests */
    register_test("test_multi_extension_languages", test_func_test_multi_extension_languages);
    register_test("test_languages_with_shebangs", test_func_test_languages_with_shebangs);
    register_test("test_languages_without_shebangs", test_func_test_languages_without_shebangs);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}
