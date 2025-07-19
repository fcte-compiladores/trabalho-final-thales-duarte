#ifndef errors_h
#define errors_h

#include <stdbool.h>

typedef enum {
    ERROR_SYNTAX,
    ERROR_RUNTIME,
    ERROR_SEMANTIC,
    ERROR_TYPE,
    ERROR_COMPILE
} ErrorType;

typedef struct {
    ErrorType type;
    int line;
    const char* message;
    const char* token;
    bool hadError;
    bool panicMode;
} LoxError;

extern LoxError loxError;

void initErrorSystem(void);
void reportError(ErrorType type, int line, const char* message, const char* token);
void reportErrorAtCurrent(const char* message);
void reportErrorAtPrevious(const char* message);
void reportRuntimeError(const char* format, ...);
void reportSemanticError(int line, const char* message, const char* token);
void reportTypeError(int line, const char* expected, const char* got);
void setPanicMode(bool panic);
bool hasError(void);
void clearError(void);

#endif 