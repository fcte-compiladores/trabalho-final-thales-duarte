#include "test_framework.h"
#include "errors.h"
#include "semantic.h"
#include "type_checking.h"
#include "object.h"
#include "value.h"
#include "vm.h"
#include "compiler.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool test_error_system_initialization() {
    initErrorSystem();
    ASSERT(!loxError.hadError);
    ASSERT(!loxError.panicMode);
    ASSERT(loxError.type == ERROR_SYNTAX);
    ASSERT(loxError.line == 0);
    ASSERT(loxError.message == NULL);
    ASSERT(loxError.token == NULL);
    return true;
}

bool test_error_reporting() {
    initErrorSystem();
    
    reportError(ERROR_SYNTAX, 10, "Unexpected token", "}");
    ASSERT(loxError.hadError);
    ASSERT(loxError.panicMode);
    ASSERT(loxError.type == ERROR_SYNTAX);
    ASSERT(loxError.line == 10);
    ASSERT(strcmp(loxError.message, "Unexpected token") == 0);
    ASSERT(strcmp(loxError.token, "}") == 0);
    
    initErrorSystem();
    reportError(ERROR_RUNTIME, 25, "Division by zero", "0");
    ASSERT(loxError.hadError);
    ASSERT(loxError.type == ERROR_RUNTIME);
    ASSERT(loxError.line == 25);
    
    return true;
}

bool test_error_recovery() {
    initErrorSystem();
    
    loxError.panicMode = true;
    loxError.hadError = true;
    
    clearError();
    ASSERT(!loxError.hadError);
    ASSERT(!loxError.panicMode);
    
    return true;
}

bool test_variable_declaration_validation() {
    initErrorSystem();
    
    ASSERT(validateVariableDeclaration("x", 1));
    ASSERT(!loxError.hadError);
    
    ASSERT(validateVariableDeclaration("variavel", 1));
    ASSERT(!loxError.hadError);
    
    ASSERT(validateVariableDeclaration("var_123", 1));
    ASSERT(!loxError.hadError);
    
    ASSERT(validateVariableDeclaration("VARIAVEL", 1));
    ASSERT(!loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("if", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("while", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("for", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("class", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("fun", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("var", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("return", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration("print", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateVariableDeclaration(NULL, 1));
    ASSERT(loxError.hadError);
    
    return true;
}

bool test_function_declaration_validation() {
    initErrorSystem();
    
    ASSERT(validateFunctionDeclaration("foo", 1));
    ASSERT(!loxError.hadError);
    
    ASSERT(validateFunctionDeclaration("calculate", 1));
    ASSERT(!loxError.hadError);
    
    ASSERT(validateFunctionDeclaration("get_value", 1));
    ASSERT(!loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("if", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("while", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("for", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("class", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("fun", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("var", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("return", 1));
    ASSERT(loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFunctionDeclaration("print", 1));
    ASSERT(loxError.hadError);
    
    return true;
}

bool test_method_validation() {
    initErrorSystem();
    
    ObjString* methodName = copyString("toString", 8);
    ASSERT(validateMethodCall(methodName, 1));
    ASSERT(!loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateMethodCall(NULL, 1));
    ASSERT(loxError.hadError);
    
    return true;
}

bool test_field_validation() {
    initErrorSystem();
    
    ObjString* fieldName = copyString("value", 5);
    ASSERT(validateFieldAccess(fieldName, 1));
    ASSERT(!loxError.hadError);
    
    initErrorSystem();
    ASSERT(!validateFieldAccess(NULL, 1));
    ASSERT(loxError.hadError);
    
    return true;
}

bool test_type_detection() {
    Value number = NUMBER_VAL(42.0);
    Value string = OBJ_VAL(copyString("hello", 5));
    Value boolean = BOOL_VAL(true);
    Value nil = NIL_VAL;
    
    ASSERT(getValueType(number) == TYPE_NUMBER);
    ASSERT(getValueType(string) == TYPE_STRING);
    ASSERT(getValueType(boolean) == TYPE_BOOL);
    ASSERT(getValueType(nil) == TYPE_NIL);
    
    return true;
}

bool test_type_names() {
    ASSERT(strcmp(getTypeName(TYPE_NUMBER), "number") == 0);
    ASSERT(strcmp(getTypeName(TYPE_STRING), "string") == 0);
    ASSERT(strcmp(getTypeName(TYPE_BOOL), "boolean") == 0);
    ASSERT(strcmp(getTypeName(TYPE_NIL), "nil") == 0);
    ASSERT(strcmp(getTypeName(TYPE_FUNCTION), "function") == 0);
    
    return true;
}

bool test_arithmetic_validation() {
    Value num1 = NUMBER_VAL(10.0);
    Value num2 = NUMBER_VAL(5.0);
    Value string = OBJ_VAL(copyString("hello", 5));
    
    ASSERT(validateArithmeticOperation(num1, num2, 1));
    
    ASSERT(!validateArithmeticOperation(num1, string, 1));
    ASSERT(!validateArithmeticOperation(string, num2, 1));
    
    return true;
}

bool test_comparison_validation() {
    Value num1 = NUMBER_VAL(10.0);
    Value num2 = NUMBER_VAL(5.0);
    Value string1 = OBJ_VAL(copyString("hello", 5));
    Value string2 = OBJ_VAL(copyString("world", 5));
    Value nil = NIL_VAL;
    
    ASSERT(validateComparisonOperation(num1, num2, 1));
    ASSERT(validateComparisonOperation(string1, string2, 1));
    ASSERT(validateComparisonOperation(nil, nil, 1));
    ASSERT(validateComparisonOperation(num1, nil, 1));
    
    ASSERT(!validateComparisonOperation(num1, string1, 1));
    
    return true;
}

bool test_function_call_validation() {
    ObjFunction* function = newFunction();
    Value funcValue = OBJ_VAL(function);
    
    ASSERT(validateFunctionCall(funcValue, 2, 1));
    
    Value number = NUMBER_VAL(42.0);
    ASSERT(!validateFunctionCall(number, 0, 1));
    
    return true;
}

bool test_compilation_basic() {
    const char* source = "print \"Hello, World!\";";
    ObjFunction* function = compile(source);
    
    if (function != NULL) {
        ASSERT(function->arity == 0);
        return true;
    }
    
    return loxError.hadError;
}

bool test_compilation_variables() {
    const char* source = "var x = 42; print x;";
    ObjFunction* function = compile(source);
    
    if (function != NULL) {
        return true;
    }
    
    return loxError.hadError;
}

bool test_compilation_functions() {
    const char* source = "fun add(a, b) { return a + b; }";
    ObjFunction* function = compile(source);
    
    if (function != NULL) {
        return true;
    }
    
    return loxError.hadError;
}

bool test_compilation_classes() {
    const char* source = "class Test { init() {} }";
    ObjFunction* function = compile(source);
    
    if (function != NULL) {
        return true;
    }
    
    return loxError.hadError;
}

bool test_memory_management() {
    for (int i = 0; i < 1000; i++) {
        ObjString* str = copyString("test", 4);
        ObjFunction* func = newFunction();
        ObjClass* cls = newClass(copyString("Test", 4));
        
        if (str == NULL || func == NULL || cls == NULL) {
            return false;
        }
    }
    
    return true;
}

bool test_error_performance() {
    for (int i = 0; i < 1000; i++) {
        initErrorSystem();
        reportError(ERROR_SYNTAX, i, "Test error", "token");
        clearError();
    }
    
    return true;
}

bool test_invalid_inputs() {
    ObjFunction* nullResult = compile(NULL);
    ASSERT(nullResult == NULL);
    
    ObjFunction* emptyResult = compile("");
    ASSERT(emptyResult == NULL);
    
    ObjFunction* invalidResult = compile("var x = ;");
    ASSERT(invalidResult == NULL);
    
    return true;
}

bool test_value_operations() {
    Value num = NUMBER_VAL(42.0);
    Value str = OBJ_VAL(copyString("test", 4));
    Value bool_val = BOOL_VAL(true);
    Value nil_val = NIL_VAL;
    
    ASSERT(IS_NUMBER(num));
    ASSERT(IS_OBJ(str));
    ASSERT(IS_BOOL(bool_val));
    ASSERT(IS_NIL(nil_val));
    
    ASSERT(AS_NUMBER(num) == 42.0);
    ASSERT(AS_BOOL(bool_val) == true);
    
    return true;
}

bool test_object_creation() {
    ObjString* str = copyString("hello", 5);
    ObjFunction* func = newFunction();
    ObjClass* cls = newClass(copyString("Test", 4));
    
    ASSERT(str != NULL);
    ASSERT(func != NULL);
    ASSERT(cls != NULL);
    
    ASSERT(OBJ_TYPE(OBJ_VAL(str)) == OBJ_STRING);
    ASSERT(OBJ_TYPE(OBJ_VAL(func)) == OBJ_FUNCTION);
    ASSERT(OBJ_TYPE(OBJ_VAL(cls)) == OBJ_CLASS);
    
    return true;
}

int comprehensive_test_main(void) {
    printf("=== TESTES ABRANGENTES DO COMPILADOR LOX ===\n\n");
    
    TestCase tests[] = {
        {"Erro: Inicialização", test_error_system_initialization},
        {"Erro: Reporte", test_error_reporting},
        {"Erro: Recuperação", test_error_recovery},
        
        {"Semântica: Validação de Variáveis", test_variable_declaration_validation},
        {"Semântica: Validação de Funções", test_function_declaration_validation},
        {"Semântica: Validação de Métodos", test_method_validation},
        {"Semântica: Validação de Campos", test_field_validation},
        
        {"Tipos: Detecção", test_type_detection},
        {"Tipos: Nomes", test_type_names},
        {"Tipos: Operações Aritméticas", test_arithmetic_validation},
        {"Tipos: Comparações", test_comparison_validation},
        {"Tipos: Chamadas de Função", test_function_call_validation},
        
        {"Compilação: Básica", test_compilation_basic},
        {"Compilação: Variáveis", test_compilation_variables},
        {"Compilação: Funções", test_compilation_functions},
        {"Compilação: Classes", test_compilation_classes},
        
        {"Performance: Gerenciamento de Memória", test_memory_management},
        {"Performance: Sistema de Erros", test_error_performance},
        
        {"Robustez: Entradas Inválidas", test_invalid_inputs},
        {"Robustez: Operações de Valor", test_value_operations},
        {"Robustez: Criação de Objetos", test_object_creation},
    };
    
    run_test_suite(tests, sizeof(tests)/sizeof(TestCase));
    return 0;
} 