#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

/* Test result tracking */
extern int tests_run;
extern int tests_passed;

/* Test registration structure */
typedef struct {
    const char* name;
    void (*func)(void);
} Test;

/* Test function array and counter */
#define MAX_TESTS 100
extern Test test_registry[MAX_TESTS];
extern int test_count;

/* Helper function to register tests */
void register_test(const char* name, void (*func)(void));

/* Test definition macro - just defines the function */
#define TEST(name) void test_func_##name(void)

/* Assertion macros */
#define ASSERT_EQ(a, b)                                                                            \
    do {                                                                                           \
        if ((a) != (b)) {                                                                          \
            printf("  FAIL: %s == %s (%d != %d) at %s:%d\n", #a, #b, (int)(a), (int)(b), __FILE__, \
                   __LINE__);                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_STR(a, b)                                                                         \
    do {                                                                                         \
        if (strcmp((a), (b)) != 0) {                                                             \
            printf("  FAIL: %s == %s (\"%s\" != \"%s\") at %s:%d\n", #a, #b, (a), (b), __FILE__, \
                   __LINE__);                                                                    \
            return;                                                                              \
        }                                                                                        \
    } while (0)

#define ASSERT_TRUE(cond)                                                        \
    do {                                                                         \
        if (!(cond)) {                                                           \
            printf("  FAIL: %s is false at %s:%d\n", #cond, __FILE__, __LINE__); \
            return;                                                              \
        }                                                                        \
    } while (0)

/* Test runner */
void run_all_tests(void);

#endif /* TEST_FRAMEWORK_H */
