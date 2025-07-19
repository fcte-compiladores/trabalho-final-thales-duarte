#ifndef semantic_h
#define semantic_h

#include <stdbool.h>
#include "object.h"

typedef struct {
    bool inClass;
    bool inFunction;
    bool inLoop;
    bool inMethod;
    ObjClass* currentClass;
    ObjFunction* currentFunction;
} SemanticContext;

extern SemanticContext semanticContext;

void initSemanticContext(void);
bool validateVariableDeclaration(const char* name, int line);
bool validateFunctionDeclaration(const char* name, int line);
bool validateClassDeclaration(const char* name, int line);
bool validateThisUsage(int line);
bool validateSuperUsage(int line);
bool validateReturnStatement(int line);
bool validateBreakStatement(int line);
bool validateContinueStatement(int line);
bool validateMethodCall(ObjString* methodName, int line);
bool validateFieldAccess(ObjString* fieldName, int line);
bool validateInheritance(const char* className, const char* superClassName, int line);
bool validateMethodOverride(const char* methodName, int line);


void enterClass(ObjClass* klass);
void exitClass(void);
void enterFunction(ObjFunction* function);
void exitFunction(void);
void enterLoop(void);
void exitLoop(void);
void enterMethod(void);
void exitMethod(void);

#endif 