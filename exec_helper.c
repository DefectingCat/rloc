#include "exec_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

// Check if a tool/command is available in PATH
int check_tool_available(const char* tool_name) {
    if (!tool_name) return 0;

    // Escape tool_name for shell safety
    char* escaped_tool = escape_shell_arg(tool_name);
    if (!escaped_tool) return 0;

    // Use 'which' to check availability
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "which %s 2>/dev/null", escaped_tool);
    free(escaped_tool);

    FILE* fp = popen(cmd, "r");
    if (!fp) return 0;

    char buf[256];
    int found = (fread(buf, 1, sizeof(buf), fp) > 0);
    pclose(fp);

    return found;
}

// Find the full path to a tool
char* find_tool_path(const char* tool_name) {
    if (!tool_name) return NULL;

    // Escape tool_name for shell safety
    char* escaped_tool = escape_shell_arg(tool_name);
    if (!escaped_tool) return NULL;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "which %s 2>/dev/null", escaped_tool);
    free(escaped_tool);

    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;

    char buf[256];
    size_t len = fread(buf, 1, sizeof(buf) - 1, fp);
    pclose(fp);

    if (len == 0) return NULL;

    // Remove trailing newline if present
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[len - 1] = '\0';
        len--;
    }

    return strdup(buf);
}

// Validate that a format string contains only expected %s placeholders
static int validate_format_string(const char* fmt) {
    if (!fmt) return 0;

    int placeholder_count = 0;
    for (const char* p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            if (*p == 's') {
                placeholder_count++;
            } else if (*p != '%' && *p != '\0') {
                // Only %% (escaped %) is allowed, other format specifiers are not
                return 0;
            }
        }
    }
    // Allow 0 or 1 %s placeholder
    return placeholder_count <= 1;
}

// Safely execute a command template with path substitution
ExecResult safe_exec(const char* cmd_template, const char* path, char* output, size_t out_len,
                     int timeout_sec) {
    (void)timeout_sec;  // Currently unused - reserved for future timeout support
    if (!cmd_template || !output || out_len == 0) {
        return EXEC_FAILED;
    }

    // Validate format string to prevent format string attacks
    if (!validate_format_string(cmd_template)) {
        return EXEC_FAILED;
    }

    // Build command with path substitution
    char cmd[1024];
    if (path) {
        // Escape path for shell safety
        char* escaped_path = escape_shell_arg(path);
        if (!escaped_path) return EXEC_FAILED;

        snprintf(cmd, sizeof(cmd), cmd_template, escaped_path);
        free(escaped_path);
    } else {
        // If no path, use template directly (assuming no %s)
        strncpy(cmd, cmd_template, sizeof(cmd) - 1);
        cmd[sizeof(cmd) - 1] = '\0';
    }

    FILE* fp = popen(cmd, "r");
    if (!fp) return EXEC_FAILED;

    size_t total = 0;
    size_t chunk;
    int truncated = 0;

    while ((chunk = fread(output + total, 1, out_len - total - 1, fp)) > 0) {
        total += chunk;
        if (total >= out_len - 1) {
            truncated = 1;
            break;
        }
    }

    int status = pclose(fp);
    output[total] = '\0';

    // Remove trailing newline for cleaner output
    while (total > 0 && (output[total - 1] == '\n' || output[total - 1] == '\r')) {
        output[total - 1] = '\0';
        total--;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        return EXEC_FAILED;
    }

    return truncated ? EXEC_OUTPUT_TRUNC : EXEC_OK;
}

// Execute a command and capture output (simple wrapper)
ExecResult exec_capture(const char* cmd, char* output, size_t out_len) {
    return safe_exec(cmd, NULL, output, out_len, 0);
}