#include <stdio.h>
#include <string.h>
#include "object.h"
#include "memory.h"
#include "value.h"
#ifdef _WIN32
#include <windows.h>
#endif


void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(FILE* file, Value value) {
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        fprintf(file, AS_BOOL(value) ? "true" : "false");
    } else if (IS_NIL(value)) {
        fprintf(file, "nil");
    } else if (IS_NUMBER(value)) {
        fprintf(file, "%g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
#ifdef _WIN32
        if (IS_STRING(value)) {
            // Converte UTF-8 para UTF-16 e imprime corretamente
            const char* utf8 = AS_CSTRING(value);
            int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
            if (wlen > 0) {
                wchar_t* wstr = (wchar_t*)malloc(wlen * sizeof(wchar_t));
                MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, wlen);
                HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                DWORD written;
                WriteConsoleW(h, wstr, wlen-1, &written, NULL);
                free(wstr);
            } else {
                fprintf(file, "%s", utf8);
            }
        } else {
            printObject(file, value);
        }
#else
        printObject(file, value);
#endif
    }
#else
    switch (value.type) {
        case VAL_BOOL: fprintf(file, AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL: fprintf(file, "nil"); break;
        case VAL_NUMBER: fprintf(file, "%g", AS_NUMBER(value)); break;
        case VAL_OBJ:
#ifdef _WIN32
            if (IS_STRING(value)) {
                const char* utf8 = AS_CSTRING(value);
                int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
                if (wlen > 0) {
                    wchar_t* wstr = (wchar_t*)malloc(wlen * sizeof(wchar_t));
                    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, wlen);
                    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
                    DWORD written;
                    WriteConsoleW(h, wstr, wlen-1, &written, NULL);
                    free(wstr);
                } else {
                    fprintf(file, "%s", utf8);
                }
            } else {
                printObject(file, value);
            }
#else
            printObject(file, value);
#endif
            break;
    }
#endif
}

bool valuesEqual(Value a, Value b) {
#ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }

    return a == b;
#else
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
        default:         return false; 
    }
#endif
}