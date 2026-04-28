#ifndef UNIQUE_H
#define UNIQUE_H

#include <stddef.h>
#include <stdint.h>

#define UNIQUE_MD5_HEX_LEN 32
/* UNIQUE_TABLE_MAX now defined in config.h */

// Hash table entry for MD5-based deduplication
typedef struct {
    uint8_t md5[16];  // Raw MD5 bytes
    int occupied;     // Entry is in use
    int deleted;      // Entry was deleted (tombstone)
} UniqueEntry;

// Open-addressing hash table for tracking unique files by MD5
typedef struct {
    UniqueEntry* entries;
    int capacity;
    int count;
} UniqueTable;

// Initialize unique table with given capacity. Returns 0 on success.
int unique_table_init(UniqueTable* table, int capacity);

// Free hash table entries
void unique_table_free(UniqueTable* table);

// Check if MD5 hash already exists in table. Returns 1 if duplicate.
int unique_table_contains(const UniqueTable* table, const uint8_t md5[16]);

// Insert MD5 hash into table. Returns 1 if already present (duplicate).
int unique_table_insert(UniqueTable* table, const uint8_t md5[16]);

// Compute MD5 hash of a file using md5sum via fork/exec.
// md5_out must be at least 16 bytes.
// Returns 0 on success, -1 on failure.
int compute_file_md5(const char* filepath, uint8_t md5_out[16]);

// Parse hex MD5 string (32 chars) into raw 16 bytes.
void hex_to_md5(const char* hex, uint8_t out[16]);

#endif
