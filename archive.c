#include "archive.h"
#include "exec_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Detect archive type from filename
ArchiveType archive_detect_type(const char* filename) {
    if (!filename) return ARCHIVE_UNKNOWN;

    size_t len = strlen(filename);

    // Check extensions
    if (len > 4 && strcmp(filename + len - 4, ".zip") == 0) {
        return ARCHIVE_ZIP;
    }
    if (len > 4 && strcmp(filename + len - 4, ".tar") == 0) {
        return ARCHIVE_TAR;
    }
    if (len > 7 && strcmp(filename + len - 7, ".tar.gz") == 0) {
        return ARCHIVE_TAR_GZ;
    }
    if (len > 8 && strcmp(filename + len - 8, ".tar.bz2") == 0) {
        return ARCHIVE_TAR_BZ2;
    }
    if (len > 7 && strcmp(filename + len - 7, ".tar.xz") == 0) {
        return ARCHIVE_TAR_XZ;
    }
    // Also accept .tgz shorthand
    if (len > 4 && strcmp(filename + len - 4, ".tgz") == 0) {
        return ARCHIVE_TAR_GZ;
    }

    return ARCHIVE_UNKNOWN;
}

// Check if file is an archive
int archive_is_archive(const char* filename) {
    return archive_detect_type(filename) != ARCHIVE_UNKNOWN;
}

// Get tool name needed to extract archive
const char* archive_get_tool(ArchiveType type) {
    switch (type) {
        case ARCHIVE_ZIP: return "unzip";
        case ARCHIVE_TAR: return "tar";
        case ARCHIVE_TAR_GZ: return "tar";
        case ARCHIVE_TAR_BZ2: return "tar";
        case ARCHIVE_TAR_XZ: return "tar";
        default: return NULL;
    }
}

// Extract archive to temporary directory
char* archive_extract(const char* archive_path, TempManager* mgr) {
    if (!archive_path || !mgr) return NULL;

    ArchiveType type = archive_detect_type(archive_path);
    if (type == ARCHIVE_UNKNOWN) return NULL;

    // Check tool availability
    const char* tool = archive_get_tool(type);
    if (!tool || !check_tool_available(tool)) {
        fprintf(stderr, "Warning: %s not available for extraction\n", tool);
        return NULL;
    }

    // Create temp directory for extraction
    char* extract_dir = temp_manager_create_dir(mgr, "rloc_extract");
    if (!extract_dir) return NULL;

    // Build extraction command
    char cmd[2048];
    char output[1024];

    switch (type) {
        case ARCHIVE_ZIP:
            snprintf(cmd, sizeof(cmd), "unzip -q '%s' -d '%s' 2>&1", archive_path, extract_dir);
            break;
        case ARCHIVE_TAR:
            snprintf(cmd, sizeof(cmd), "tar -xf '%s' -C '%s' 2>&1", archive_path, extract_dir);
            break;
        case ARCHIVE_TAR_GZ:
            snprintf(cmd, sizeof(cmd), "tar -xzf '%s' -C '%s' 2>&1", archive_path, extract_dir);
            break;
        case ARCHIVE_TAR_BZ2:
            snprintf(cmd, sizeof(cmd), "tar -xjf '%s' -C '%s' 2>&1", archive_path, extract_dir);
            break;
        case ARCHIVE_TAR_XZ:
            snprintf(cmd, sizeof(cmd), "tar -xJf '%s' -C '%s' 2>&1", archive_path, extract_dir);
            break;
        default:
            return NULL;
    }

    ExecResult result = exec_capture(cmd, output, sizeof(output));
    if (result != EXEC_OK && result != EXEC_OUTPUT_TRUNC) {
        fprintf(stderr, "Warning: Failed to extract %s: %s\n", archive_path, output);
        // Keep directory registered for cleanup
        return NULL;
    }

    return extract_dir;
}