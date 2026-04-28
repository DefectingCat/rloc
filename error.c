#include "error.h"

static const char* error_messages[] = {
    [RLOC_OK] = "Success",
    [RLOC_ERR_MEMORY] = "Memory allocation failed",
    [RLOC_ERR_FILE_OPEN] = "Cannot open file",
    [RLOC_ERR_FILE_READ] = "Cannot read file",
    [RLOC_ERR_DIR_SCAN] = "Cannot scan directory",
    [RLOC_ERR_CONFIG] = "Configuration error",
    [RLOC_ERR_INVALID_ARG] = "Invalid argument",
    [RLOC_ERR_GIT] = "Git operation failed",
    [RLOC_ERR_ARCHIVE] = "Archive extraction failed",
    [RLOC_ERR_TIMEOUT] = "Operation timed out",
};

const char* rloc_strerror(RlocError code) {
    if (code >= 0 && code < (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
        return error_messages[code];
    }
    return "Unknown error";
}
