#include "test_framework.h"
#include "errors.h"
#include <stdio.h>
#include <stdbool.h>

bool test_simple_error() {
    initErrorSystem();
    ASSERT(!loxError.hadError);
    return true;
}

bool test_simple_semantic() {
    initErrorSystem();
    ASSERT(validateVariableDeclaration("x", 1));
    ASSERT(!loxError.hadError);
    return true;
}

int simple_test_main(void) {
    printf("=== TESTE SIMPLES ===\n\n");
    
    TestCase tests[] = {
        {"Erro Simples", test_simple_error},
        {"Semântica Simples", test_simple_semantic},
    };
    
    run_test_suite(tests, sizeof(tests)/sizeof(TestCase));
    return 0;
} 