#ifndef test_framework_h
#define test_framework_h

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char* name;
    bool (*test_func)(void);
} TestCase;

#define TEST(name) bool test_##name(void)
#define ASSERT(condition) if (!(condition)) { printf("  FAIL: %s\n", #condition); return false; }
#define ASSERT_EQ(actual, expected) if ((actual) != (expected)) { printf("  FAIL: %s == %s, got %d, expected %d\n", #actual, #expected, (int)(actual), (int)(expected)); return false; }
#define ASSERT_STR_EQ(actual, expected) if (strcmp((actual), (expected)) != 0) { printf("  FAIL: %s == %s, got '%s', expected '%s'\n", #actual, #expected, (actual), (expected)); return false; }
#define ASSERT_NULL(ptr) if ((ptr) != NULL) { printf("  FAIL: %s should be NULL\n", #ptr); return false; }
#define ASSERT_NOT_NULL(ptr) if ((ptr) == NULL) { printf("  FAIL: %s should not be NULL\n", #ptr); return false; }

void run_test_suite(TestCase* tests, int test_count);
void run_single_test(TestCase* test);

#endif 