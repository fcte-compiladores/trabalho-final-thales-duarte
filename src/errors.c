#include "errors.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

LoxError loxError = {0};

void initErrorSystem(void) {
    loxError.hadError = false;
    loxError.panicMode = false;
    loxError.type = ERROR_SYNTAX;
    loxError.line = 0;
    loxError.message = NULL;
    loxError.token = NULL;
}

void reportError(ErrorType type, int line, const char* message, const char* token) {
    if (loxError.panicMode) return;
    
    loxError.panicMode = true;
    loxError.hadError = true;
    loxError.type = type;
    loxError.line = line;
    loxError.message = message;
    loxError.token = token;
    
    const char* errorTypeStr;
    switch (type) {
        case ERROR_SYNTAX: errorTypeStr = "Syntax"; break;
        case ERROR_RUNTIME: errorTypeStr = "Runtime"; break;
        case ERROR_SEMANTIC: errorTypeStr = "Semantic"; break;
        case ERROR_TYPE: errorTypeStr = "Type"; break;
        case ERROR_COMPILE: errorTypeStr = "Compile"; break;
        default: errorTypeStr = "Unknown"; break;
    }
    
    fprintf(stderr, "[line %d] %s Error", line, errorTypeStr);
    
    if (token != NULL) {
        fprintf(stderr, " at '%s'", token);
    }
    
    fprintf(stderr, ": %s\n", message);
}

void reportErrorAtCurrent(const char* message) {

    reportError(ERROR_SYNTAX, 0, message, NULL);
}

void reportErrorAtPrevious(const char* message) {

    reportError(ERROR_SYNTAX, 0, message, NULL);
}

void reportRuntimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Runtime Error: ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    loxError.hadError = true;
    loxError.type = ERROR_RUNTIME;
}

void reportSemanticError(int line, const char* message, const char* token) {
    reportError(ERROR_SEMANTIC, line, message, token);
}

void reportTypeError(int line, const char* expected, const char* got) {
    char message[256];
    snprintf(message, sizeof(message), "Expected %s but got %s", expected, got);
    reportError(ERROR_TYPE, line, message, NULL);
}

void setPanicMode(bool panic) {
    loxError.panicMode = panic;
}

bool hasError(void) {
    return loxError.hadError;
}

void clearError(void) {
    loxError.hadError = false;
    loxError.panicMode = false;
} 