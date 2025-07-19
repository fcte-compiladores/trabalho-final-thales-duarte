#include "semantic.h"
#include "errors.h"
#include <string.h>

SemanticContext semanticContext = {0};

void initSemanticContext(void) {
    semanticContext.inClass = false;
    semanticContext.inFunction = false;
    semanticContext.inLoop = false;
    semanticContext.inMethod = false;
    semanticContext.currentClass = NULL;
    semanticContext.currentFunction = NULL;
}

bool validateVariableDeclaration(const char* name, int line) {
    if (name == NULL) {
        reportSemanticError(line, "Variable name cannot be null", "null");
        return false;
    }
    

    if (strcmp(name, "true") == 0 || strcmp(name, "false") == 0 || 
        strcmp(name, "nil") == 0 || strcmp(name, "this") == 0 || 
        strcmp(name, "super") == 0 || strcmp(name, "var") == 0 ||
        strcmp(name, "fun") == 0 || strcmp(name, "class") == 0 ||
        strcmp(name, "if") == 0 || strcmp(name, "else") == 0 ||
        strcmp(name, "while") == 0 || strcmp(name, "for") == 0 ||
        strcmp(name, "return") == 0 || strcmp(name, "print") == 0) {
        reportSemanticError(line, "Cannot use reserved word as variable name", name);
        return false;
    }
    
    return true;
}

bool validateFunctionDeclaration(const char* name, int line) {
    if (name == NULL) {
        reportSemanticError(line, "Function name cannot be null", "null");
        return false;
    }
    

    if (strcmp(name, "true") == 0 || strcmp(name, "false") == 0 || 
        strcmp(name, "nil") == 0 || strcmp(name, "this") == 0 || 
        strcmp(name, "super") == 0 || strcmp(name, "var") == 0 ||
        strcmp(name, "fun") == 0 || strcmp(name, "class") == 0 ||
        strcmp(name, "if") == 0 || strcmp(name, "else") == 0 ||
        strcmp(name, "while") == 0 || strcmp(name, "for") == 0 ||
        strcmp(name, "return") == 0 || strcmp(name, "print") == 0) {
        reportSemanticError(line, "Cannot use reserved word as function name", name);
        return false;
    }
    
    return true;
}

bool validateClassDeclaration(const char* name, int line) {
    if (name == NULL) return false;
    

    if (strcmp(name, "true") == 0 || strcmp(name, "false") == 0 || 
        strcmp(name, "nil") == 0 || strcmp(name, "this") == 0 || 
        strcmp(name, "super") == 0 || strcmp(name, "var") == 0 ||
        strcmp(name, "fun") == 0 || strcmp(name, "class") == 0 ||
        strcmp(name, "if") == 0 || strcmp(name, "else") == 0 ||
        strcmp(name, "while") == 0 || strcmp(name, "for") == 0 ||
        strcmp(name, "return") == 0 || strcmp(name, "print") == 0) {
        reportSemanticError(line, "Cannot use reserved word as class name", name);
        return false;
    }
    
    return true;
}

bool validateThisUsage(int line) {
    if (!semanticContext.inClass && !semanticContext.inMethod) {
        reportSemanticError(line, "Cannot use 'this' outside of a class", "this");
        return false;
    }
    return true;
}

bool validateSuperUsage(int line) {
    if (!semanticContext.inClass && !semanticContext.inMethod) {
        reportSemanticError(line, "Cannot use 'super' outside of a class", "super");
        return false;
    }

    return true;
}

bool validateReturnStatement(int line) {
    if (!semanticContext.inFunction && !semanticContext.inMethod) {
        reportSemanticError(line, "Cannot return from top-level code", "return");
        return false;
    }
    return true;
}

bool validateBreakStatement(int line) {
    if (!semanticContext.inLoop) {
        reportSemanticError(line, "Cannot use 'break' outside of a loop", "break");
        return false;
    }
    return true;
}

bool validateContinueStatement(int line) {
    if (!semanticContext.inLoop) {
        reportSemanticError(line, "Cannot use 'continue' outside of a loop", "continue");
        return false;
    }
    return true;
}

bool validateMethodCall(ObjString* methodName, int line) {
    if (methodName == NULL) {
        reportSemanticError(line, "Method name cannot be null", "null");
        return false;
    }

    return true;
}

bool validateFieldAccess(ObjString* fieldName, int line) {
    if (fieldName == NULL) {
        reportSemanticError(line, "Field name cannot be null", "null");
        return false;
    }

    return true;
}

bool validateInheritance(const char* className, const char* superClassName, int line) {
    if (className == NULL || superClassName == NULL) return false;
    
    if (strcmp(className, superClassName) == 0) {
        reportSemanticError(line, "A class cannot inherit from itself", className);
        return false;
    }
    
    return true;
}

bool validateMethodOverride(const char* methodName, int line) {
    if (methodName == NULL) return false;

    return true;
}


void enterClass(ObjClass* klass) {
    semanticContext.inClass = true;
    semanticContext.currentClass = klass;
}

void exitClass(void) {
    semanticContext.inClass = false;
    semanticContext.currentClass = NULL;
}

void enterFunction(ObjFunction* function) {
    semanticContext.inFunction = true;
    semanticContext.currentFunction = function;
}

void exitFunction(void) {
    semanticContext.inFunction = false;
    semanticContext.currentFunction = NULL;
}

void enterLoop(void) {
    semanticContext.inLoop = true;
}

void exitLoop(void) {
    semanticContext.inLoop = false;
}

void enterMethod(void) {
    semanticContext.inMethod = true;
}

void exitMethod(void) {
    semanticContext.inMethod = false;
} 