#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "value.h"
#include "table.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;
    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    return object;
}

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method) {
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass* newClass(ObjString* name) {
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}

ObjClosure* newClosure(ObjFunction* function) {
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjInstance* newInstance(ObjClass* klass) {
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjNative* newNative(NativeFn function, int argCount) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    native->argCount = argCount;
    return native;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NIL_VAL);
    pop();
    return string;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

ObjUpvalue* newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->closed = NIL_VAL;
    upvalue->next = NULL;
    return upvalue;
}

static void printFunction(FILE* file, ObjFunction* function) {
    if (function->name == NULL) {
        fprintf(file, "<script>");
        return;
    }

    printf("<fn %s>", function->name->chars);
}

void printObject(FILE* file, Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_BOUND_METHOD:
            printFunction(file, AS_BOUND_METHOD(value)->method->function);
            break;
        case OBJ_CLASS:
            fprintf(file, "%s", AS_CLASS(value)->name->chars);
            break;
        case OBJ_CLOSURE:
            printFunction(file, AS_CLOSURE(value)->function);
            break;
        case OBJ_FUNCTION:
            printFunction(file, AS_FUNCTION(value));
            break;
        case OBJ_INSTANCE: {
            Value str = valueToString(value);
            if (IS_STRING(str)) {
                fprintf(file, "%s", AS_CSTRING(str));
            } else {
                ObjInstance* instance = AS_INSTANCE(value);
                fprintf(file, "%s instance", instance->klass->name->chars);
            }
            break;
        }
        case OBJ_NATIVE:
            fprintf(file, "<native fn>");
            break;
        case OBJ_STRING:
            fprintf(file, "%s", AS_CSTRING(value));
            break;
        case OBJ_UPVALUE:
            fprintf(file, "upvalue");
            break;
        case OBJ_LIST: {
            ObjList* list = AS_LIST(value);
            fprintf(file, "[");
            for (int i = 0; i < list->count; i++) {
                printValue(file, list->values[i]);
                if (i < list->count - 1) fprintf(file, ", ");
            }
            fprintf(file, "]");
            break;
        }
        case OBJ_DICT: {
            ObjDict* dict = AS_DICT(value);
            fprintf(file, "{");
            bool first = true;
            for (int i = 0; i < dict->entries.capacity; i++) {
                Entry* entry = &dict->entries.entries[i];
                if (entry->key != NULL) {
                    if (!first) fprintf(file, ", ");
                    printValue(file, OBJ_VAL(entry->key));
                    fprintf(file, ": ");
                    printValue(file, entry->value);
                    first = false;
                }
            }
            fprintf(file, "}");
            break;
        }
        case OBJ_ENUM: {
            ObjEnum* enumObj = AS_ENUM(value);
            fprintf(file, "%s enum", enumObj->name->chars);
            break;
        }
    }
}

ObjList* newList() {
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    list->count = 0;
    list->capacity = 0;
    list->values = NULL;
    return list;
}

void listAppend(ObjList* list, Value value) {
    if (list->capacity < list->count + 1) {
        int oldCapacity = list->capacity;
        list->capacity = oldCapacity < 8 ? 8 : oldCapacity * 2;
        list->values = (Value*)reallocate(list->values, sizeof(Value) * oldCapacity, sizeof(Value) * list->capacity);
    }
    list->values[list->count++] = value;
}

Value listGet(ObjList* list, int index) {
    if (index < 0 || index >= list->count) return NIL_VAL;
    return list->values[index];
}

void listSet(ObjList* list, int index, Value value) {
    if (index < 0 || index >= list->count) return;
    list->values[index] = value;
}

int listLength(ObjList* list) {
    return list->count;
}

ObjDict* newDict() {
    ObjDict* dict = ALLOCATE_OBJ(ObjDict, OBJ_DICT);
    initTable(&dict->entries);
    return dict;
}

void dictSet(ObjDict* dict, Value key, Value value) {
    if (!IS_STRING(key)) return;
    tableSet(&dict->entries, AS_STRING(key), value);
}

Value dictGet(ObjDict* dict, Value key) {
    if (!IS_STRING(key)) return NIL_VAL;
    Value value;
    if (tableGet(&dict->entries, AS_STRING(key), &value)) {
        return value;
    }
    return NIL_VAL;
}

bool dictDelete(ObjDict* dict, Value key) {
    if (!IS_STRING(key)) return false;
    return tableDelete(&dict->entries, AS_STRING(key));
}

int dictLength(ObjDict* dict) {
    return dict->entries.count;
}

ObjEnum* newEnum(ObjString* name) {
    ObjEnum* enumObj = ALLOCATE_OBJ(ObjEnum, OBJ_ENUM);
    enumObj->name = name;
    initTable(&enumObj->values);
    return enumObj;
}

void enumAddValue(ObjEnum* enumObj, ObjString* name, Value value) {
    tableSet(&enumObj->values, name, value);
}

Value enumGetValue(ObjEnum* enumObj, ObjString* name) {
    Value value;
    if (tableGet(&enumObj->values, name, &value)) {
        return value;
    }
    return NIL_VAL;
}

int enumLength(ObjEnum* enumObj) {
    return enumObj->values.count;
}

