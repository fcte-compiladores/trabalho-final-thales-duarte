#ifndef context_h
#define context_h

#include "table.h"
#include "object.h"

typedef struct Context {
    Table variables;
    struct Context* parent;
    bool isClass;
    bool isFunction;
    bool isMethod;
    ObjClass* currentClass;
    ObjFunction* currentFunction;
    int scopeDepth;
} Context;

extern Context* globalContext;

Context* createContext(Context* parent);
void destroyContext(Context* context);
bool defineVariable(Context* ctx, ObjString* name, Value value);
bool assignVariable(Context* ctx, ObjString* name, Value value);
bool getVariable(Context* ctx, ObjString* name, Value* result);
bool hasVariable(Context* ctx, ObjString* name);
void enterScope(Context* ctx);
void exitScope(Context* ctx);
void setContextClass(Context* ctx, ObjClass* klass);
void setContextFunction(Context* ctx, ObjFunction* function);
void setContextMethod(Context* ctx, bool isMethod);
Context* getGlobalContext(void);
void initGlobalContext(void);
void freeGlobalContext(void);

#endif 