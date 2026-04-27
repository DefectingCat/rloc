#ifndef LANG_DEFS_H
#define LANG_DEFS_H

#include "language.h"

#define NUM_LANGUAGES 25
#define MAX_CUSTOM_LANGUAGES 64
#define MAX_CUSTOM_FILTERS 8

/* Custom language definition structure (for user-defined languages) */
typedef struct {
    char name[64];
    char extensions[256];      // comma-separated
    char filenames[256];       // comma-separated
    char shebangs[256];        // comma-separated
    char str_delimiters[16];
    char str_escape[8];
    GenericFilter filters[MAX_CUSTOM_FILTERS];
    char filter_patterns_open[MAX_CUSTOM_FILTERS][64];   // storage for pattern_open strings
    char filter_patterns_close[MAX_CUSTOM_FILTERS][64];  // storage for pattern_close strings
    int filter_count;
} CustomLanguage;

/* Array of all built-in language definitions */
extern const Language g_languages[NUM_LANGUAGES];

/* Custom language registry */
extern CustomLanguage g_custom_languages[MAX_CUSTOM_LANGUAGES];
extern int g_custom_language_count;

/* Load custom language definitions from file
 * Returns number of languages loaded, -1 on error
 */
int lang_defs_load_file(const char* filepath);

/* Register a custom language definition
 * Returns 0 on success, -1 if registry is full
 */
int lang_defs_register_custom(const CustomLanguage* lang);

/* Find language by name (checks custom first, then built-in)
 * Returns Language pointer or NULL if not found
 */
const Language* lang_defs_find_by_name(const char* name);

/* Clear all custom language definitions */
void lang_defs_clear_custom(void);

/* Show language information */
int lang_show(const char* name);

/* Show extension to language mapping */
int ext_show(const char* ext);

#endif /* LANG_DEFS_H */
