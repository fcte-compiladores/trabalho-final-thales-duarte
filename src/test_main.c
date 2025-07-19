#include "test_framework.h"
#include "errors.h"
#include <stdio.h>
#include <stdbool.h>

bool test_error_system_init() {
    initErrorSystem();
    ASSERT(!loxError.hadError);
    ASSERT(!loxError.panicMode);
    ASSERT(loxError.type == ERROR_SYNTAX);
    return true;
}

bool test_report_error() {
    initErrorSystem();
    reportError(ERROR_RUNTIME, 42, "Mensagem de erro", "token");
    ASSERT(loxError.hadError);
    ASSERT(loxError.panicMode);
    ASSERT(loxError.type == ERROR_RUNTIME);
    ASSERT(loxError.line == 42);
    ASSERT(loxError.message != NULL);
    ASSERT(loxError.token != NULL);
    return true;
}

bool test_variable_declaration() {
    ASSERT(validateVariableDeclaration("x", 1));
    ASSERT(!validateVariableDeclaration("for", 2));
    return true;
}

bool test_function_declaration() {
    ASSERT(validateFunctionDeclaration("foo", 1));
    ASSERT(!validateFunctionDeclaration("class", 2));
    return true;
}

int test_main(void) {
    TestCase tests[] = {
        {"Erro: Inicialização", test_error_system_init},
        {"Erro: Reporte de erro", test_report_error},
        {"Variável: Declaração", test_variable_declaration},
        {"Função: Declaração", test_function_declaration},
    };
    run_test_suite(tests, sizeof(tests)/sizeof(TestCase));
    return 0;
} 