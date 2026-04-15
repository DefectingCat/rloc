#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "temp_manager.h"

// Archive type enumeration
typedef enum {
    ARCHIVE_UNKNOWN,
    ARCHIVE_ZIP,
    ARCHIVE_TAR,
    ARCHIVE_TAR_GZ,
    ARCHIVE_TAR_BZ2,
    ARCHIVE_TAR_XZ
} ArchiveType;

// Detect archive type from filename
ArchiveType archive_detect_type(const char* filename);

// Extract archive to temporary directory
// Uses temp_manager to track extracted directory for cleanup
// Returns path to extracted directory, or NULL on error
char* archive_extract(const char* archive_path, TempManager* mgr);

// Check if file is an archive (based on extension)
int archive_is_archive(const char* filename);

// Get tool name needed to extract archive type
const char* archive_get_tool(ArchiveType type);

#endif