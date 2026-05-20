#ifndef TUI_DO_TEST_H
#define TUI_DO_TEST_H

#include <stdio.h>
#include <stdlib.h>

static int test_failures = 0;

#define SCENARIO(name) static void name(void)
#define GIVEN(text) printf("  given %s\n", text)
#define WHEN(text) printf("  when  %s\n", text)
#define THEN(text) printf("  then  %s\n", text)

#define EXPECT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "    expected true: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
            test_failures++; \
        } \
    } while (0)

#define EXPECT_INT_EQ(actual, expected) \
    do { \
        int actual_value = (actual); \
        int expected_value = (expected); \
        if (actual_value != expected_value) { \
            fprintf(stderr, "    expected %s == %d, got %d (%s:%d)\n", #actual, expected_value, actual_value, __FILE__, __LINE__); \
            test_failures++; \
        } \
    } while (0)

#define EXPECT_INT_GE(actual, expected) \
    do { \
        int actual_value = (actual); \
        int expected_value = (expected); \
        if (actual_value < expected_value) { \
            fprintf(stderr, "    expected %s >= %d, got %d (%s:%d)\n", #actual, expected_value, actual_value, __FILE__, __LINE__); \
            test_failures++; \
        } \
    } while (0)

#define RUN_SCENARIO(name) \
    do { \
        printf("\nscenario: %s\n", #name); \
        name(); \
    } while (0)

static int finish_tests(void) {
    if (test_failures > 0) {
        fprintf(stderr, "\n%d test failure(s)\n", test_failures);
        return 1;
    }

    printf("\nall tests passed\n");
    return 0;
}

#endif
