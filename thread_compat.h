/**
 * Cross-platform thread abstraction layer
 * POSIX pthread on Unix/macOS, Windows threads on Windows
 */

#ifndef THREAD_COMPAT_H
#define THREAD_COMPAT_H

#ifdef _WIN32
#include <windows.h>

typedef HANDLE rloc_thread_t;

/* Windows thread function wrapper to match pthread signature */
typedef struct {
    void* (*func)(void*);
    void* arg;
} WinThreadWrapper;

static DWORD WINAPI win_thread_wrapper_func(void* wrapper_arg) {
    WinThreadWrapper* w = (WinThreadWrapper*)wrapper_arg;
    w->func(w->arg);
    free(w);
    return 0;
}

static inline int rloc_thread_create(rloc_thread_t* thread, void* (*func)(void*), void* arg) {
    WinThreadWrapper* wrapper = malloc(sizeof(WinThreadWrapper));
    if (!wrapper) return -1;
    wrapper->func = func;
    wrapper->arg = arg;
    *thread = CreateThread(NULL, 0, win_thread_wrapper_func, wrapper, 0, NULL);
    if (*thread == NULL) {
        free(wrapper);
        return -1;
    }
    return 0;
}

static inline int rloc_thread_join(rloc_thread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

static inline int rloc_get_cpu_count(void) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (int)sysinfo.dwNumberOfProcessors;
}

/* Windows atomic increment */
static inline void rloc_atomic_increment(int* ptr) { InterlockedIncrement((LONG*)ptr); }

#else
/* POSIX (Linux, macOS, etc.) */
#include <pthread.h>
#include <unistd.h>

typedef pthread_t rloc_thread_t;

static inline int rloc_thread_create(rloc_thread_t* thread, void* (*func)(void*), void* arg) {
    return pthread_create(thread, NULL, func, arg);
}

static inline int rloc_thread_join(rloc_thread_t thread) { return pthread_join(thread, NULL); }

static inline int rloc_get_cpu_count(void) {
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? (int)n : 1;
}

/* GCC/Clang atomic increment */
static inline void rloc_atomic_increment(int* ptr) { __sync_fetch_and_add(ptr, 1); }

#endif

#endif /* THREAD_COMPAT_H */