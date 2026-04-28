#ifndef ERROR_H
#define ERROR_H

/* Error codes */
typedef enum {
    RLOC_OK = 0,
    RLOC_ERR_MEMORY = 1,
    RLOC_ERR_FILE_OPEN = 2,
    RLOC_ERR_FILE_READ = 3,
    RLOC_ERR_DIR_SCAN = 4,
    RLOC_ERR_CONFIG = 5,
    RLOC_ERR_INVALID_ARG = 6,
    RLOC_ERR_GIT = 7,
    RLOC_ERR_ARCHIVE = 8,
    RLOC_ERR_TIMEOUT = 9,
} RlocError;

/* Error context for detailed reporting */
typedef struct {
    RlocError code;
    const char *file;
    int line;
    const char *message;
} ErrorContext;

/* Macros for error handling */
#define RLOC_ERROR(code, msg) ((ErrorContext){(code), __FILE__, __LINE__, (msg)})
#define RLOC_ERROR_PTR(code, msg) (&(ErrorContext){(code), __FILE__, __LINE__, (msg)})

/* Get error message string */
const char* rloc_strerror(RlocError code);

#endif /* ERROR_H */
