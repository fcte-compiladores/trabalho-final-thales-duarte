#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include "table.h"
#include "object.h"
#include "context.h"
#include "errors.h"
#include "semantic.h"
#include "type_checking.h"

VM vm;

static void resetStack();
static void runtimeError(const char* format, ...);
static void defineNative(const char* name, NativeFn function, int argCount);

#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[(uint8_t)READ_BYTE()])
#define READ_CONSTANT_16() (frame->closure->function->chunk.constants.values[(uint16_t)READ_SHORT()])
#define READ_SHORT() \
        (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8 | frame->ip[-1])))
#define READ_STRING() AS_STRING(READ_CONSTANT_16())

static void runtimeError(const char* format, ...);

static bool clockNative(int argCount, Value* args, Value* result) {
    *result = NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
    return true;
}

static bool exitNative(int argCount, Value* args, Value* result) {
    if (!IS_NUMBER(args[0])) {
        runtimeError("Number expected for exit() parameter.");
        return false;
    }

    exit(AS_NUMBER(args[0]));

    *result = NIL_VAL;

    return true;
}

static bool readNative(int argCount, Value* args, Value* result) {
    int c = getchar();
    *result = c == -1 ? NIL_VAL : NUMBER_VAL((uint8_t) c);
    return true;
}

static bool printerrNative(int argCount, Value* args, Value* result) {
    printValue(stderr, args[0]);
    fprintf(stderr, "\n");
    *result = NIL_VAL;
    return true;
}

static bool utfNative(int argCount, Value* args, Value* result) {

    for (int i = 0; i < 4; i++) {
        if (i > 0 && IS_NIL(args[i])) continue;
        
        if (!IS_NUMBER(args[i]) || (AS_NUMBER(args[i]) < 0 || AS_NUMBER(args[i]) > 255)) {
            runtimeError("utf parameter should be a number between 0 and 255.");
            return false;
        }        
    }

    int count = 0;
    unsigned char bytes[] = { 
        IS_NIL(args[0]) ? 0 : AS_NUMBER(args[count++]),
        IS_NIL(args[1]) ? 0 : AS_NUMBER(args[count++]),
        IS_NIL(args[2]) ? 0 : AS_NUMBER(args[count++]),
        IS_NIL(args[3]) ? 0 : AS_NUMBER(args[count++]),
    };

    char chars[count];
    memcpy(chars, bytes, count);

    *result = OBJ_VAL(copyString((char*)chars, count));

    return true;
}

static bool listNative(int argCount, Value* args, Value* result) {
    *result = OBJ_VAL(newList());
    return true;
}

static bool appendNative(int argCount, Value* args, Value* result) {
    if (!IS_LIST(args[0])) {
        runtimeError("Primeiro argumento de append deve ser uma lista.");
        return false;
    }
    listAppend(AS_LIST(args[0]), args[1]);
    *result = NIL_VAL;
    return true;
}

static bool getNative(int argCount, Value* args, Value* result) {
    if (!IS_LIST(args[0]) || !IS_NUMBER(args[1])) {
        runtimeError("Argumentos de get devem ser (lista, índice).");
        return false;
    }
    int index = (int)AS_NUMBER(args[1]);
    *result = listGet(AS_LIST(args[0]), index);
    return true;
}

static bool setNative(int argCount, Value* args, Value* result) {
    if (!IS_LIST(args[0]) || !IS_NUMBER(args[1])) {
        runtimeError("Argumentos de set devem ser (lista, índice, valor).");
        return false;
    }
    int index = (int)AS_NUMBER(args[1]);
    listSet(AS_LIST(args[0]), index, args[2]);
    *result = NIL_VAL;
    return true;
}

static bool lengthNative(int argCount, Value* args, Value* result) {
    if (!IS_LIST(args[0])) {
        runtimeError("Argumento de length deve ser uma lista.");
        return false;
    }
    *result = NUMBER_VAL(listLength(AS_LIST(args[0])));
    return true;
}

static bool dictNative(int argCount, Value* args, Value* result) {
    *result = OBJ_VAL(newDict());
    return true;
}

static bool dictSetNative(int argCount, Value* args, Value* result) {
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        runtimeError("Argumentos de dictSet devem ser (dicionário, chave_string, valor).");
        return false;
    }
    dictSet(AS_DICT(args[0]), args[1], args[2]);
    *result = NIL_VAL;
    return true;
}

static bool dictGetNative(int argCount, Value* args, Value* result) {
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        runtimeError("Argumentos de dictGet devem ser (dicionário, chave_string).");
        return false;
    }
    *result = dictGet(AS_DICT(args[0]), args[1]);
    return true;
}

static bool dictDeleteNative(int argCount, Value* args, Value* result) {
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        runtimeError("Argumentos de dictDelete devem ser (dicionário, chave_string).");
        return false;
    }
    *result = BOOL_VAL(dictDelete(AS_DICT(args[0]), args[1]));
    return true;
}

static bool dictLengthNative(int argCount, Value* args, Value* result) {
    if (!IS_DICT(args[0])) {
        runtimeError("Argumento de dictLength deve ser um dicionário.");
        return false;
    }
    *result = NUMBER_VAL(dictLength(AS_DICT(args[0])));
    return true;
}

static bool enumNative(int argCount, Value* args, Value* result) {
    if (!IS_STRING(args[0])) {
        runtimeError("Argumento de enum deve ser uma string (nome do enum).");
        return false;
    }
    *result = OBJ_VAL(newEnum(AS_STRING(args[0])));
    return true;
}

static bool enumAddValueNative(int argCount, Value* args, Value* result) {
    if (!IS_ENUM(args[0]) || !IS_STRING(args[1])) {
        runtimeError("Argumentos de enumAddValue devem ser (enum, nome_string, valor).");
        return false;
    }
    enumAddValue(AS_ENUM(args[0]), AS_STRING(args[1]), args[2]);
    *result = NIL_VAL;
    return true;
}

static bool enumGetValueNative(int argCount, Value* args, Value* result) {
    if (!IS_ENUM(args[0]) || !IS_STRING(args[1])) {
        runtimeError("Argumentos de enumGetValue devem ser (enum, nome_string).");
        return false;
    }
    *result = enumGetValue(AS_ENUM(args[0]), AS_STRING(args[1]));
    return true;
}

static bool enumLengthNative(int argCount, Value* args, Value* result) {
    if (!IS_ENUM(args[0])) {
        runtimeError("Argumento de enumLength deve ser um enum.");
        return false;
    }
    *result = NUMBER_VAL(enumLength(AS_ENUM(args[0])));
    return true;
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) { 
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    resetStack();
}

static void defineNative(const char* name, NativeFn function, int argCount) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function, argCount)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM() {
    resetStack();
    vm.objects = NULL;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);

    vm.initString = NULL;
    vm.initString = copyString("init", 4);

    defineNative("clock", clockNative, 0);
    defineNative("exit", exitNative, 1);
    defineNative("read", readNative, 0);
    defineNative("printerr", printerrNative, 1);
    defineNative("utf", utfNative, 4);
    defineNative("list", listNative, 0);
    defineNative("append", appendNative, 2);
    defineNative("get", getNative, 2);
    defineNative("set", setNative, 3);
    defineNative("length", lengthNative, 1);
    defineNative("dict", dictNative, 0);
    defineNative("dictSet", dictSetNative, 3);
    defineNative("dictGet", dictGetNative, 2);
    defineNative("dictDelete", dictDeleteNative, 2);
    defineNative("dictLength", dictLengthNative, 1);
    defineNative("enum", enumNative, 1);
    defineNative("enumAddValue", enumAddValueNative, 3);
    defineNative("enumGetValue", enumGetValueNative, 2);
    defineNative("enumLength", enumLengthNative, 1);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    vm.initString = NULL;
    freeObjects();
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount);
            }
            case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(callee);
                // Move os argumentos uma posição para frente
                for (int i = argCount; i > 0; i--) {
                    vm.stackTop[-i] = vm.stackTop[-i + 1];
                }
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
                Value initializer;
                if (tableGet(&klass->methods, vm.initString, &initializer)) {
                    if (!IS_CLOSURE(initializer)) {
                        runtimeError("Método 'init' da classe não é uma função.");
                        return false;
                    }
                    return call(AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }
                return true;
            }
            case OBJ_CLOSURE: {
                return call(AS_CLOSURE(callee), argCount);
            }
            case OBJ_NATIVE: {
                ObjNative* native = AS_NATIVE(callee);
                if (native->argCount != argCount) {
                    runtimeError("Expected %d arguments but got %d.", native->argCount, argCount);
                    return false;
                }
                Value result;
                if (!(native->function(argCount, vm.stackTop - argCount, &result))) {
                    return false;
                }
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break;
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    return call(AS_CLOSURE(method), argCount);
}

bool invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount);
    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }
    ObjInstance* instance = AS_INSTANCE(receiver);
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount - 1] = value;
        return callValue(value, argCount);
    }
    return invokeFromClass(instance->klass, name, argCount);
}

static bool bindMethod(ObjClass* klass, ObjString* name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(method));

    pop();
    push(OBJ_VAL(bound));
    return true;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult handleGetProperty(CallFrame* frame) {
    if (!IS_INSTANCE(peek(0))) {
        runtimeError("Only instances have properties.");
        return INTERPRET_RUNTIME_ERROR;
    }
    ObjInstance* instance = AS_INSTANCE(peek(0));
    ObjString* name = READ_STRING();
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        pop();
        push(value);
        return INTERPRET_OK;
    }
    if (!bindMethod(instance->klass, name)) {
        return INTERPRET_RUNTIME_ERROR;
    }
    return INTERPRET_OK;
}

static InterpretResult handleSetProperty(CallFrame* frame) {
    if (!IS_INSTANCE(peek(1))) {
        runtimeError("Only instances have properties.");
        return INTERPRET_RUNTIME_ERROR;
    }
    ObjInstance* instance = AS_INSTANCE(peek(1));
    tableSet(&instance->fields, READ_STRING(), peek(0));
    Value value = pop();
    pop();
    push(value);
    return INTERPRET_OK;
}

static InterpretResult handleAdd(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
    if (IS_STRING(a) && IS_STRING(b)) {
        push(a);
        push(b);
        concatenate();
        return INTERPRET_OK;
    } 
    else if (IS_STRING(a)) {
        Value bStr = valueToString(b);
        push(a);
        push(bStr);
        concatenate();
        return INTERPRET_OK;
    }
    else if (IS_STRING(b)) {
        Value aStr = valueToString(a);
        push(aStr);
        push(b);
        concatenate();
        return INTERPRET_OK;
    }
    else if (IS_NUMBER(a) && IS_NUMBER(b)) {
        push(NUMBER_VAL(AS_NUMBER(a) + AS_NUMBER(b)));
        return INTERPRET_OK;
    }
    else if (IS_LIST(a) || IS_LIST(b)) {
        runtimeError("Operação '+' não suporta listas.");
        return INTERPRET_RUNTIME_ERROR;
    }
    else if (IS_INSTANCE(a)) {
        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__add__", 7);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a); 
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            runtimeError("Classe '%s' não implementa operador '+' (__add__).", instance->klass->name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
    else {
        runtimeError("Operands must be two numbers or two strings.");
        return INTERPRET_RUNTIME_ERROR;
    }
}

static InterpretResult handleSubtract(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        push(NUMBER_VAL(AS_NUMBER(a) - AS_NUMBER(b)));
        return INTERPRET_OK;
    }
    else if (IS_LIST(a) || IS_LIST(b)) {
        runtimeError("Operação '-' não suporta listas.");
        return INTERPRET_RUNTIME_ERROR;
    }
    else if (IS_INSTANCE(a)) {
        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__sub__", 7);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a); 
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            runtimeError("Classe '%s' não implementa operador '-' (__sub__).", instance->klass->name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
    else {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
}

static InterpretResult handleMultiply(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
  
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        push(NUMBER_VAL(AS_NUMBER(a) * AS_NUMBER(b)));
        return INTERPRET_OK;
    }

    else if (IS_LIST(a) || IS_LIST(b)) {
        runtimeError("Operação '*' não suporta listas.");
        return INTERPRET_RUNTIME_ERROR;
    }

    else if (IS_INSTANCE(a)) {

        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__mul__", 7);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a);
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            runtimeError("Classe '%s' não implementa operador '*' (__mul__).", instance->klass->name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
    else {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
}

static InterpretResult handleDivide(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
    
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        if (AS_NUMBER(b) == 0.0) {
            runtimeError("Division by zero.");
            return INTERPRET_RUNTIME_ERROR;
        }
        push(NUMBER_VAL(AS_NUMBER(a) / AS_NUMBER(b)));
        return INTERPRET_OK;
    }
    
    else if (IS_LIST(a) || IS_LIST(b)) {
        runtimeError("Operação '/' não suporta listas.");
        return INTERPRET_RUNTIME_ERROR;
    }
    
    else if (IS_INSTANCE(a)) {
     
        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__div__", 7);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a); 
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            runtimeError("Classe '%s' não implementa operador '/' (__div__).", instance->klass->name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
    else {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
}

static InterpretResult handleCall(CallFrame* frame) {
    int argCount = READ_BYTE();
    Value callee = peek(argCount);
    if (!callValue(callee, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
    }
    frame = &vm.frames[vm.frameCount - 1];
    return INTERPRET_OK;
}

static InterpretResult handleInvoke(CallFrame* frame) {
    ObjString* method = READ_STRING();
    int argCount = READ_BYTE();
    if (!invoke(method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
    }
    frame = &vm.frames[vm.frameCount - 1];
    return INTERPRET_OK;
}

static InterpretResult handleSuperInvoke(CallFrame* frame) {
    ObjString* method = READ_STRING();
    int argCount = READ_BYTE();
    ObjClass* superclass = AS_CLASS(pop());
    if (!invokeFromClass(superclass, method, argCount)) {
        return INTERPRET_RUNTIME_ERROR;
    }
    frame = &vm.frames[vm.frameCount - 1];
    return INTERPRET_OK;
}

static InterpretResult handleReturn(CallFrame* frame) {
    Value result = pop();
    closeUpvalues(frame->slots);
    vm.frameCount--;
    if (vm.frameCount == 0) {
        pop();
        return INTERPRET_OK;
    }
    vm.stackTop = frame->slots;
    push(result);
    frame = &vm.frames[vm.frameCount - 1];
    return INTERPRET_OK;
}

static InterpretResult handleClosure(CallFrame* frame) {
    ObjFunction* function = AS_FUNCTION(READ_CONSTANT_16());
    ObjClosure* closure = newClosure(function);
    push(OBJ_VAL(closure));
    for (int i = 0; i < closure->upvalueCount; i++) {
        uint8_t isLocal = READ_BYTE();
        uint8_t index = READ_BYTE();
        if (isLocal) {
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
        } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
        }
    }
    return INTERPRET_OK;
}

static InterpretResult handleGetGlobal(CallFrame* frame) {
    ObjString* name = READ_STRING();
    Value value;
    if (!tableGet(&vm.globals, name, &value)) {
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
    }
    push(value);
    return INTERPRET_OK;
}

static InterpretResult handleSetGlobal(CallFrame* frame) {
    ObjString* name = READ_STRING();
    if (tableSet(&vm.globals, name, peek(0))) {
        tableDelete(&vm.globals, name);
        runtimeError("Undefined variable '%s'.", name->chars);
        return INTERPRET_RUNTIME_ERROR;
    }
    return INTERPRET_OK;
}

static InterpretResult handleDefineGlobal(CallFrame* frame) {
    ObjString* name = READ_STRING();
    tableSet(&vm.globals, name, peek(0));
    pop();
    return INTERPRET_OK;
}

static InterpretResult handleGetLocal(CallFrame* frame) {
    uint8_t slot = READ_BYTE();
    push(frame->slots[slot]);
    return INTERPRET_OK;
}

static InterpretResult handleSetLocal(CallFrame* frame) {
    uint8_t slot = READ_BYTE();
    frame->slots[slot] = peek(0);
    return INTERPRET_OK;
}

static InterpretResult handleGetUpvalue(CallFrame* frame) {
    uint8_t slot = READ_BYTE();
    push(*frame->closure->upvalues[slot]->location);
    return INTERPRET_OK;
}

static InterpretResult handleSetUpvalue(CallFrame* frame) {
    uint8_t slot = READ_BYTE();
    *frame->closure->upvalues[slot]->location = peek(0);
    return INTERPRET_OK;
}

static InterpretResult handleGetSuper(CallFrame* frame) {
    ObjString* name = READ_STRING();
    ObjClass* superclass = AS_CLASS(pop());
    if (!bindMethod(superclass, name)) {
        return INTERPRET_RUNTIME_ERROR;
    }
    return INTERPRET_OK;
}

static InterpretResult handleJump(CallFrame* frame) {
    uint16_t offset = READ_SHORT();
    frame->ip += offset;
    return INTERPRET_OK;
}

static InterpretResult handleJumpIfFalse(CallFrame* frame) {
    uint16_t offset = READ_SHORT();
    if (isFalsey(peek(0))) frame->ip += offset;
    return INTERPRET_OK;
}

static InterpretResult handleLoop(CallFrame* frame) {
    uint16_t offset = READ_SHORT();
    frame->ip -= offset;
    return INTERPRET_OK;
}

static InterpretResult handleEqual(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
   
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        push(BOOL_VAL(AS_NUMBER(a) == AS_NUMBER(b)));
        return INTERPRET_OK;
    }
    else if (IS_BOOL(a) && IS_BOOL(b)) {
        push(BOOL_VAL(AS_BOOL(a) == AS_BOOL(b)));
        return INTERPRET_OK;
    }
    else if (IS_NIL(a) && IS_NIL(b)) {
        push(BOOL_VAL(true));
        return INTERPRET_OK;
    }
    else if (IS_STRING(a) && IS_STRING(b)) {
        push(BOOL_VAL(AS_STRING(a) == AS_STRING(b)));
        return INTERPRET_OK;
    }
    else if (IS_INSTANCE(a)) {
        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__eq__", 6);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a); 
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            push(BOOL_VAL(valuesEqual(a, b)));
            return INTERPRET_OK;
        }
    }
    else {
        push(BOOL_VAL(valuesEqual(a, b)));
        return INTERPRET_OK;
    }
}

static InterpretResult handleGreater(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        push(BOOL_VAL(AS_NUMBER(a) > AS_NUMBER(b)));
        return INTERPRET_OK;
    }
    else if (IS_INSTANCE(a)) {
        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__gt__", 6);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a); 
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            runtimeError("Classe '%s' não implementa operador '>' (__gt__).", instance->klass->name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
    else {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
}

static InterpretResult handleLess(CallFrame* frame) {
    Value b = pop();
    Value a = pop();
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        push(BOOL_VAL(AS_NUMBER(a) < AS_NUMBER(b)));
        return INTERPRET_OK;
    }
    else if (IS_INSTANCE(a)) {
        ObjInstance* instance = AS_INSTANCE(a);
        ObjString* methodName = copyString("__lt__", 6);
        Value method;
        if (tableGet(&instance->klass->methods, methodName, &method)) {
            push(a); 
            push(b); 
            callValue(method, 1);
            return INTERPRET_OK;
        } else {
            runtimeError("Classe '%s' não implementa operador '<' (__lt__).", instance->klass->name->chars);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
    else {
        runtimeError("Operands must be numbers.");
        return INTERPRET_RUNTIME_ERROR;
    }
}

static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            if (IS_LIST(peek(0)) || IS_LIST(peek(1))) { \
                runtimeError("Operações aritméticas não suportam listas."); \
            } else { \
                runtimeError("Operands must be numbers."); \
            } \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
    } while (false)

    for (;;) {
        if (vm.frameCount == 0) {
            return INTERPRET_OK;
        }
        
        frame = &vm.frames[vm.frameCount - 1];
        
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT:    push(READ_CONSTANT());    break;
            case OP_CONSTANT_16: push(READ_CONSTANT_16()); break;
            case OP_INTEGER:    push(NUMBER_VAL(READ_BYTE()));  break;
            case OP_INTEGER_16: push(NUMBER_VAL(READ_SHORT())); break;
            case OP_NIL:        push(NIL_VAL);                  break;
            case OP_TRUE:       push(BOOL_VAL(true));           break;
            case OP_FALSE:      push(BOOL_VAL(false));          break;
            case OP_MINUS_ONE:  push(NUMBER_VAL(-1));           break;
            case OP_ZERO:       push(NUMBER_VAL(0));            break;
            case OP_ONE:        push(NUMBER_VAL(1));            break;
            case OP_POP:        pop(); break;
            case OP_GET_LOCAL: {
                InterpretResult result = handleGetLocal(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_SET_LOCAL: {
                InterpretResult result = handleSetLocal(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_GET_GLOBAL: {
                InterpretResult result = handleGetGlobal(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_SET_GLOBAL: {
                InterpretResult result = handleSetGlobal(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_DEFINE_GLOBAL: {
                InterpretResult result = handleDefineGlobal(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_GET_UPVALUE: {
                InterpretResult result = handleGetUpvalue(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_SET_UPVALUE: {
                InterpretResult result = handleSetUpvalue(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_GET_PROPERTY: {
                InterpretResult result = handleGetProperty(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_SET_PROPERTY: {
                InterpretResult result = handleSetProperty(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_GET_SUPER: {
                InterpretResult result = handleGetSuper(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_EQUAL: {
                InterpretResult result = handleEqual(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_GREATER: {
                InterpretResult result = handleGreater(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_LESS: {
                InterpretResult result = handleLess(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_ADD: {
                InterpretResult result = handleAdd(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_SUBTRACT: {
                InterpretResult result = handleSubtract(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_MULTIPLY: {
                InterpretResult result = handleMultiply(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_DIVIDE: {
                InterpretResult result = handleDivide(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_NOT:      push(BOOL_VAL(isFalsey(pop()))); break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                *(vm.stackTop - 1) = NUMBER_VAL(-AS_NUMBER(*(vm.stackTop - 1)));
                break;
            case OP_PRINT: {
                printValue(stdout, pop());
                printf("\n");
                break;
            }
            case OP_JUMP: {
                InterpretResult result = handleJump(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                InterpretResult result = handleJumpIfFalse(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_LOOP: {
                InterpretResult result = handleLoop(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_CALL: {
                InterpretResult result = handleCall(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_INVOKE: {
                InterpretResult result = handleInvoke(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_SUPER_INVOKE: {
                InterpretResult result = handleSuperInvoke(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_CLOSURE: {
                InterpretResult result = handleClosure(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_CLOSE_UPVALUE: {
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            }
            case OP_RETURN: {
                InterpretResult result = handleReturn(frame);
                if (result != INTERPRET_OK) return result;
                break;
            }
            case OP_CLASS: {
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            }
            case OP_INHERIT: {
                Value superclass = peek(1);
                if (!IS_CLASS(superclass)) {
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop();
                break;
            }
            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;
    if (debugAstMode) return INTERPRET_OK;
    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);
    return run();
}

Value getToStringValue(Value instance) {
    if (!IS_INSTANCE(instance)) return NIL_VAL;
    ObjInstance* obj = AS_INSTANCE(instance);
    ObjString* methodName = copyString("toString", 8);
    Value method;
    if (!tableGet(&obj->klass->methods, methodName, &method)) {
        return NIL_VAL;
    }
    
    Value* oldStackTop = vm.stackTop;
    int oldFrameCount = vm.frameCount;
    
    push(instance); 
    bool success = callValue(method, 0);
    
    if (success && vm.frameCount > oldFrameCount) {
        Value result = pop();
        vm.stackTop = oldStackTop;
        vm.frameCount = oldFrameCount;
        return result;
    } else {
        vm.stackTop = oldStackTop;
        vm.frameCount = oldFrameCount;
        return OBJ_VAL(copyString("[has toString]", 13));
    }
}

Value valueToString(Value value) {
    if (IS_STRING(value)) {
        return value;
    } else if (IS_NUMBER(value)) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%g", AS_NUMBER(value));
        return OBJ_VAL(copyString(buffer, strlen(buffer)));
    } else if (IS_BOOL(value)) {
        return OBJ_VAL(copyString(AS_BOOL(value) ? "true" : "false", 
                                 AS_BOOL(value) ? 4 : 5));
    } else if (IS_NIL(value)) {
        return OBJ_VAL(copyString("nil", 3));
    } else if (IS_INSTANCE(value)) {
        return getToStringValue(value);
    } else if (IS_LIST(value)) {
        return OBJ_VAL(copyString("[list]", 6));
    } else if (IS_DICT(value)) {
        return OBJ_VAL(copyString("[dict]", 6));
    } else if (IS_ENUM(value)) {
        ObjEnum* enumObj = AS_ENUM(value);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%s enum", enumObj->name->chars);
        return OBJ_VAL(copyString(buffer, strlen(buffer)));
    } else {
        return OBJ_VAL(copyString("[object]", 8));
    }
}

Value vmToString(Value instance) {
    if (!IS_INSTANCE(instance)) return valueToString(instance);
    ObjInstance* obj = AS_INSTANCE(instance);
    ObjString* methodName = NULL;
    for (int i = 0; i < obj->klass->methods.capacity; i++) {
        Entry* entry = &obj->klass->methods.entries[i];
        if (entry->key != NULL && strcmp(entry->key->chars, "toString") == 0) {
            methodName = entry->key;
            break;
        }
    }
    if (methodName == NULL) return OBJ_VAL(copyString("[object]", 8));
    push(instance);
    if (!invoke(methodName, 0)) {
        pop();
        return OBJ_VAL(copyString("[object]", 8));
    }
    InterpretResult result = run();
    if (result != INTERPRET_OK) {
        return OBJ_VAL(copyString("[object]", 8));
    }
    Value strResult = pop();
    return valueToString(strResult);
}

