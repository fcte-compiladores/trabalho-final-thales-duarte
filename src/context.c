#include "context.h"
#include "memory.h"
#include <stdlib.h>

Context* globalContext = NULL;

Context* createContext(Context* parent) {
    Context* ctx = ALLOCATE(Context, 1);
    initTable(&ctx->variables);
    ctx->parent = parent;
    ctx->isClass = false;
    ctx->isFunction = false;
    ctx->isMethod = false;
    ctx->currentClass = NULL;
    ctx->currentFunction = NULL;
    ctx->scopeDepth = parent ? parent->scopeDepth + 1 : 0;
    return ctx;
}

void destroyContext(Context* context) {
    if (context == NULL) return;
    
    freeTable(&context->variables);
    FREE(Context, context);
}

bool defineVariable(Context* ctx, ObjString* name, Value value) {
    if (ctx == NULL || name == NULL) return false;
    

    Value existing;
    if (tableGet(&ctx->variables, name, &existing)) {
        return false; 
    }
    
    tableSet(&ctx->variables, name, value);
    return true;
}

bool assignVariable(Context* ctx, ObjString* name, Value value) {
    if (ctx == NULL || name == NULL) return false;
    

    Context* current = ctx;
    while (current != NULL) {
        Value existing;
        if (tableGet(&current->variables, name, &existing)) {
            tableSet(&current->variables, name, value);
            return true;
        }
        current = current->parent;
    }
    
    return false; 
}

bool getVariable(Context* ctx, ObjString* name, Value* result) {
    if (ctx == NULL || name == NULL || result == NULL) return false;
    

    Context* current = ctx;
    while (current != NULL) {
        if (tableGet(&current->variables, name, result)) {
            return true;
        }
        current = current->parent;
    }
    
    return false; 
}

bool hasVariable(Context* ctx, ObjString* name) {
    if (ctx == NULL || name == NULL) return false;
    

    Context* current = ctx;
    while (current != NULL) {
        Value existing;
        if (tableGet(&current->variables, name, &existing)) {
            return true;
        }
        current = current->parent;
    }
    
    return false;
}

void enterScope(Context* ctx) {
    if (ctx == NULL) return;
    ctx->scopeDepth++;
}

void exitScope(Context* ctx) {
    if (ctx == NULL) return;
    if (ctx->scopeDepth > 0) {
        ctx->scopeDepth--;
    }
}

void setContextClass(Context* ctx, ObjClass* klass) {
    if (ctx == NULL) return;
    ctx->currentClass = klass;
    ctx->isClass = (klass != NULL);
}

void setContextFunction(Context* ctx, ObjFunction* function) {
    if (ctx == NULL) return;
    ctx->currentFunction = function;
    ctx->isFunction = (function != NULL);
}

void setContextMethod(Context* ctx, bool isMethod) {
    if (ctx == NULL) return;
    ctx->isMethod = isMethod;
}

Context* getGlobalContext(void) {
    return globalContext;
}

void initGlobalContext(void) {
    if (globalContext == NULL) {
        globalContext = createContext(NULL);
    }
}

void freeGlobalContext(void) {
    if (globalContext != NULL) {
        destroyContext(globalContext);
        globalContext = NULL;
    }
} 