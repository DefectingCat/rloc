#ifndef CONFIG_H
#define CONFIG_H

#include "cli.h"

/* Default config file path: ~/.config/rloc/options.txt */
#define RLOC_CONFIG_DIR ".config/rloc"
#define RLOC_CONFIG_FILE "options.txt"

/* Load config from file and apply to args.
 * Single-value options overwrite, list options append.
 * Returns 0 on success (or no config file), -1 on error.
 */
int config_load(const char* filepath, CliArgs* args);

/* Get default config file path.
 * Caller must free the returned string.
 * Returns NULL if home directory cannot be determined.
 */
char* config_get_default_path(void);

#endif /* CONFIG_H */
