#define _POSIX_C_SOURCE 200809L
#include "unique.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static uint32_t hash_md5_bytes(const uint8_t* md5, uint32_t capacity) {
    // Simple FNV-1a variant hash over 16 MD5 bytes
    uint32_t h = 2166136261u;
    for (int i = 0; i < 16; i++) {
        h ^= md5[i];
        h *= 16777619u;
    }
    return h % capacity;
}

int unique_table_init(UniqueTable* table, int capacity) {
    if (capacity <= 0) return -1;
    table->entries = calloc((size_t)capacity, sizeof(UniqueEntry));
    if (!table->entries) return -1;
    table->capacity = capacity;
    table->count = 0;
    return 0;
}

void unique_table_free(UniqueTable* table) {
    if (table->entries) {
        free(table->entries);
        table->entries = NULL;
    }
    table->capacity = 0;
    table->count = 0;
}

int unique_table_contains(const UniqueTable* table, const uint8_t md5[16]) {
    uint32_t h = hash_md5_bytes(md5, (uint32_t)table->capacity);
    for (int i = 0; i < table->capacity; i++) {
        uint32_t idx = (h + (uint32_t)i) % (uint32_t)table->capacity;
        if (!table->entries[idx].occupied) return 0;
        if (table->entries[idx].deleted) continue;
        if (memcmp(table->entries[idx].md5, md5, 16) == 0) return 1;
    }
    return 0;
}

int unique_table_insert(UniqueTable* table, const uint8_t md5[16]) {
    // Check existing first
    if (unique_table_contains(table, md5)) return 1;

    uint32_t h = hash_md5_bytes(md5, (uint32_t)table->capacity);
    for (int i = 0; i < table->capacity; i++) {
        uint32_t idx = (h + (uint32_t)i) % (uint32_t)table->capacity;
        if (!table->entries[idx].occupied || table->entries[idx].deleted) {
            memcpy(table->entries[idx].md5, md5, 16);
            table->entries[idx].occupied = 1;
            table->entries[idx].deleted = 0;
            table->count++;
            return 0;
        }
        if (memcmp(table->entries[idx].md5, md5, 16) == 0) return 1;
    }
    // Table full
    return -1;
}

int compute_file_md5(const char* filepath, uint8_t md5_out[16]) {
    int pipefd[2];
    if (pipe(pipefd) == -1) return -1;

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        // Close stderr to suppress error messages
        close(STDERR_FILENO);

        // Execute md5sum via PATH search
        execlp("md5sum", "md5sum", filepath, (char*)NULL);
        // If md5sum fails, try md5 (BSD/macOS)
        execlp("md5", "md5", "-r", filepath, (char*)NULL);
        _exit(127);
    }

    // Parent process
    close(pipefd[1]);

    char buf[256];
    ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    if (n <= 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) return -1;

    buf[n] = '\0';
    hex_to_md5(buf, md5_out);
    return 0;
}

void hex_to_md5(const char* hex, uint8_t out[16]) {
    for (int i = 0; i < 16; i++) {
        unsigned int byte;
        sscanf(hex + i * 2, "%02x", &byte);
        out[i] = (uint8_t)byte;
    }
}
