#ifndef EXEC_HELPER_H
#define EXEC_HELPER_H

#include <stddef.h>

// Execution result codes
typedef enum {
    EXEC_OK,            // Command executed successfully
    EXEC_TOOL_MISSING,  // Required tool not found
    EXEC_FAILED,        // Command execution failed
    EXEC_OUTPUT_TRUNC,  // Output was truncated (buffer too small)
    EXEC_TIMEOUT        // Command exceeded timeout (future use)
} ExecResult;

// Check if a tool/command is available in PATH
// Returns 1 if available, 0 if not
int check_tool_available(const char* tool_name);

// Find the full path to a tool
// Returns path string (caller must free), or NULL if not found
char* find_tool_path(const char* tool_name);

// Safely execute a command template with path substitution
// cmd_template should contain %s where the path should be inserted
// output buffer receives stdout, out_len specifies buffer size
// timeout_sec is timeout in seconds (0 = no timeout)
// Returns ExecResult code
ExecResult safe_exec(const char* cmd_template, const char* path, char* output, size_t out_len,
                     int timeout_sec);

// Execute a command and capture output
// Simple wrapper for common cases
// Returns ExecResult code
ExecResult exec_capture(const char* cmd, char* output, size_t out_len);

#endif