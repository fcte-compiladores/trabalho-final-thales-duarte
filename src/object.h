#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"
#include "stdio.h"
#include "table.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value)    isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)  isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)   isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)   isObjType(value, OBJ_STRING)
#define IS_DICT(value)     isObjType(value, OBJ_DICT)
#define IS_ENUM(value)     isObjType(value, OBJ_ENUM)

#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)    ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)  ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))
#define AS_NATIVE(value)   (((ObjNative*)AS_OBJ(value)))
#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)
#define AS_DICT(value)     ((ObjDict*)AS_OBJ(value))
#define AS_ENUM(value)     ((ObjEnum*)AS_OBJ(value))

typedef enum {
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE,
    OBJ_LIST,
    OBJ_DICT,
    OBJ_ENUM
} ObjType;

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
};

typedef struct {
    Obj obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef bool (*NativeFn)(int argCount, Value* args, Value* result);

typedef struct {
    Obj obj;
    NativeFn function;
    int argCount;
} ObjNative;

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct {
    Obj obj;
    ObjString* name;
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

typedef struct {
    Obj obj;
    int count;
    int capacity;
    Value* values;
} ObjList;

typedef struct {
    Obj obj;
    Table entries;
} ObjDict;

typedef struct {
    Obj obj;
    ObjString* name;
    Table values;
} ObjEnum;

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjClass* newClass(ObjString* name);
ObjClosure* newClosure(ObjFunction* function);
ObjFunction* newFunction();
ObjInstance* newInstance(ObjClass* klass);
ObjNative* newNative(NativeFn function, int argCount);
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
ObjUpvalue* newUpvalue(Value* slot);
void printObject(FILE* file, Value value);
ObjList* newList();
void listAppend(ObjList* list, Value value);
Value listGet(ObjList* list, int index);
void listSet(ObjList* list, int index, Value value);
int listLength(ObjList* list);
ObjDict* newDict();
void dictSet(ObjDict* dict, Value key, Value value);
Value dictGet(ObjDict* dict, Value key);
bool dictDelete(ObjDict* dict, Value key);
int dictLength(ObjDict* dict);
ObjEnum* newEnum(ObjString* name);
void enumAddValue(ObjEnum* enumObj, ObjString* name, Value value);
Value enumGetValue(ObjEnum* enumObj, ObjString* name);
int enumLength(ObjEnum* enumObj);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}


#endif