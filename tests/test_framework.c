#include "test_framework.h"

/* Test result tracking */
int tests_run = 0;
int tests_passed = 0;

/* Test function array and counter */
Test test_registry[MAX_TESTS];
int test_count = 0;

/* Helper function to register tests */
void register_test(const char* name, void (*func)(void)) {
    if (test_count < MAX_TESTS) {
        test_registry[test_count].name = name;
        test_registry[test_count].func = func;
        test_count++;
    }
}

/* Test runner */
void run_all_tests(void) {
    printf("Running %d test(s)...\n", test_count);

    for (int i = 0; i < test_count; i++) {
        tests_run++;
        printf("\n[%d/%d] %s:", i + 1, test_count, test_registry[i].name);

        test_registry[i].func();

        /* If we get here, the test passed (didn't return early) */
        printf(" PASS\n");
        tests_passed++;
    }

    printf("\n========================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed!\n");
    }
}
