#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../counter_interface.h"
#include "test_framework.h"

TEST(test_counter_default_new_returns_non_null) {
    CounterInterface* counter = counter_default_new();
    ASSERT_TRUE(counter != NULL);
}

TEST(test_counter_interface_function_pointers_set) {
    CounterInterface* counter = counter_default_new();
    ASSERT_TRUE(counter != NULL);
    ASSERT_TRUE(counter->init != NULL);
    ASSERT_TRUE(counter->count_file != NULL);
    ASSERT_TRUE(counter->count_lines != NULL);
    ASSERT_TRUE(counter->free != NULL);
}

TEST(test_count_result_init_zeros) {
    CountResult result;
    result.blank = 1;
    result.comment = 2;
    result.code = 3;
    result.total = 4;

    count_result_init(&result);

    ASSERT_EQ(result.blank, 0);
    ASSERT_EQ(result.comment, 0);
    ASSERT_EQ(result.code, 0);
    ASSERT_EQ(result.total, 0);
}

TEST(test_count_result_add_correct) {
    CountResult dest;
    count_result_init(&dest);

    CountResult src = {5, 10, 20, 35};
    count_result_add(&dest, &src);

    ASSERT_EQ(dest.blank, 5);
    ASSERT_EQ(dest.comment, 10);
    ASSERT_EQ(dest.code, 20);
    ASSERT_EQ(dest.total, 35);

    /* Add again */
    count_result_add(&dest, &src);

    ASSERT_EQ(dest.blank, 10);
    ASSERT_EQ(dest.comment, 20);
    ASSERT_EQ(dest.code, 40);
    ASSERT_EQ(dest.total, 70);
}

TEST(test_counter_init_success) {
    CounterInterface* counter = counter_default_new();
    ASSERT_TRUE(counter != NULL);

    int result = counter->init(counter);
    ASSERT_EQ(result, 0);
}

int main(void) {
    register_test("test_counter_default_new_returns_non_null", test_func_test_counter_default_new_returns_non_null);
    register_test("test_counter_interface_function_pointers_set", test_func_test_counter_interface_function_pointers_set);
    register_test("test_count_result_init_zeros", test_func_test_count_result_init_zeros);
    register_test("test_count_result_add_correct", test_func_test_count_result_add_correct);
    register_test("test_counter_init_success", test_func_test_counter_init_success);

    run_all_tests();
    return (tests_passed == tests_run) ? 0 : 1;
}