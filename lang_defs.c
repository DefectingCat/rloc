#include "lang_defs.h"
#include <stddef.h>

/* Generic filter definitions */
static const GenericFilter python_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter javascript_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter c_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter cpp_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter java_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter go_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter rust_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter typescript_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter ruby_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter php_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_INLINE, "#", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter csharp_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
    {FILTER_REMOVE_BETWEEN, "/*", "*/"},
};

static const GenericFilter swift_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter kotlin_filters[] = {
    {FILTER_REMOVE_INLINE, "//", NULL},
};

static const GenericFilter shell_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

static const GenericFilter perl_filters[] = {
    {FILTER_REMOVE_INLINE, "#", NULL},
};

/* Language definitions array */
const Language g_languages[NUM_LANGUAGES] = {
    {
        .name = "Python",
        .extensions = "py,pyw",
        .filenames = NULL,
        .shebangs = "python,python3",
        .generic_filter_count = 1,
        .generic_filters = python_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "JavaScript",
        .extensions = "js,mjs",
        .filenames = NULL,
        .shebangs = "node",
        .generic_filter_count = 1,
        .generic_filters = javascript_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "C",
        .extensions = "c,h",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 2,
        .generic_filters = c_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "C++",
        .extensions = "cpp,cxx,cc,hpp,hxx,hh",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 2,
        .generic_filters = cpp_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Java",
        .extensions = "java",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 2,
        .generic_filters = java_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Go",
        .extensions = "go",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 1,
        .generic_filters = go_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Rust",
        .extensions = "rs",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 2,
        .generic_filters = rust_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "TypeScript",
        .extensions = "ts,tsx",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 2,
        .generic_filters = typescript_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Ruby",
        .extensions = "rb",
        .filenames = NULL,
        .shebangs = "ruby",
        .generic_filter_count = 1,
        .generic_filters = ruby_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"",
        .str_escape = "\\"
    },
    {
        .name = "PHP",
        .extensions = "php",
        .filenames = NULL,
        .shebangs = "php",
        .generic_filter_count = 3,
        .generic_filters = php_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "C#",
        .extensions = "cs",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 2,
        .generic_filters = csharp_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Swift",
        .extensions = "swift",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 1,
        .generic_filters = swift_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Kotlin",
        .extensions = "kt,kts",
        .filenames = NULL,
        .shebangs = NULL,
        .generic_filter_count = 1,
        .generic_filters = kotlin_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Shell",
        .extensions = "sh,bash,zsh,ksh",
        .filenames = "Makefile,Dockerfile",
        .shebangs = "bash,sh,zsh,ksh",
        .generic_filter_count = 1,
        .generic_filters = shell_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"'",
        .str_escape = "\\"
    },
    {
        .name = "Perl",
        .extensions = "pl,pm",
        .filenames = NULL,
        .shebangs = "perl",
        .generic_filter_count = 1,
        .generic_filters = perl_filters,
        .comment_hook = NULL,
        .str_delimiters = "\"",
        .str_escape = "\\"
    }
};
