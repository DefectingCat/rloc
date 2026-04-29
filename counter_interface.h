#ifndef COUNTER_INTERFACE_H
#define COUNTER_INTERFACE_H

#include <stddef.h>

#include "language.h"

/* Count result structure */
typedef struct {
    int blank;
    int comment;
    int code;
    int total;
} CountResult;

/* Counter interface - abstract interface for line counting implementations */
typedef struct CounterInterface {
    /* Initialize the counter (optional, can be NULL) */
    int (*init)(struct CounterInterface* self);

    /* Count lines in a file given a language context */
    int (*count_file)(struct CounterInterface* self, const char* filepath,
                      const Language* lang, int skip_lines, CountResult* result);

    /* Count lines in a buffer given a language context */
    void (*count_lines)(struct CounterInterface* self, const char* src, size_t len,
                        const Language* lang, int skip_lines, CountResult* result);

    /* Free counter resources (optional, can be NULL) */
    void (*free)(struct CounterInterface* self);

    /* Implementation-specific data */
    void* data;
} CounterInterface;

/* Default counter implementation (standard single-threaded counter) */
CounterInterface* counter_default_new(void);

/* Helper: initialize CountResult to zeros */
static inline void count_result_init(CountResult* result) {
    result->blank = 0;
    result->comment = 0;
    result->code = 0;
    result->total = 0;
}

/* Helper: add two CountResults */
static inline void count_result_add(CountResult* dest, const CountResult* src) {
    dest->blank += src->blank;
    dest->comment += src->comment;
    dest->code += src->code;
    dest->total += src->total;
}

#endif
