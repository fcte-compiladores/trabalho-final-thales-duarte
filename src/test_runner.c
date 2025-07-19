#include "test_framework.h"
#include "errors.h"
#include <stdio.h>
#include <stdbool.h>

bool test_error_system_init();
bool test_report_error();
bool test_variable_declaration();
bool test_function_declaration();

int comprehensive_test_main(void);

int main(void) {
    printf("=== TESTES DO COMPILADOR LOX ===\n\n");
    
    printf("--- TESTES BÁSICOS ---\n");
    TestCase basicTests[] = {
        {"Erro: Inicialização", test_error_system_init},
        {"Erro: Reporte de erro", test_report_error},
        {"Variável: Declaração", test_variable_declaration},
        {"Função: Declaração", test_function_declaration},
    };
    
    run_test_suite(basicTests, sizeof(basicTests)/sizeof(TestCase));
    
    printf("\n--- TESTES ABRANGENTES ---\n");
    comprehensive_test_main();
    
    return 0;
} 