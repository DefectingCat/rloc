#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "cli.h"

/* Handle --list-file: read input paths from file or STDIN */
int input_handler_process_list_file(CliArgs* args);

#endif
