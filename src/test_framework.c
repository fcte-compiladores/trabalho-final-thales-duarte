#include "test_framework.h"
#include <stdlib.h>

void run_single_test(TestCase* test) {
    printf("Running test: %s\n", test->name);
    bool result = test->test_func();
    if (result) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

void run_test_suite(TestCase* tests, int test_count) {
    printf("Running test suite with %d tests\n", test_count);
    printf("================================\n");
    
    int passed = 0;
    int failed = 0;
    
    for (int i = 0; i < test_count; i++) {
        printf("Test %d: %s... ", i+1, tests[i].name);
        bool result = tests[i].test_func();
        if (result) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
            failed++;
        }
    }
    
    printf("================================\n");
    printf("Test Results: %d passed, %d failed\n", passed, failed);
    
    if (failed > 0) {
        exit(1);
    }
} 