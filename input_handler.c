#include "input_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int input_handler_process_list_file(CliArgs* args) {
    if (args->list_file == NULL) return 0;

    FILE* list_fp = NULL;
    if (strcmp(args->list_file, "-") == 0) {
        list_fp = stdin;
    } else {
        list_fp = fopen(args->list_file, "r");
        if (!list_fp) {
            fprintf(stderr, "Error: Cannot open list file '%s'\n", args->list_file);
            return -1;
        }
    }

    char line[2048];
    while (fgets(line, sizeof(line), list_fp) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }
        if (len == 0) continue;

        int capacity = args->n_input_files > 0 ? args->n_input_files * 2 : 16;
        char** new_files = realloc(args->input_files, capacity * sizeof(char*));
        if (!new_files) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            if (list_fp != stdin) fclose(list_fp);
            return -1;
        }
        args->input_files = new_files;
        args->input_files[args->n_input_files] = strdup(line);
        if (!args->input_files[args->n_input_files]) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            if (list_fp != stdin) fclose(list_fp);
            return -1;
        }
        args->n_input_files++;
    }

    if (list_fp != stdin) fclose(list_fp);

    if (args->n_input_files == 0) {
        fprintf(stderr, "Error: No input files found in list file '%s'\n", args->list_file);
        return -1;
    }
    return 0;
}
