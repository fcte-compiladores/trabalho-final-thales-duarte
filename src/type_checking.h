#ifndef type_checking_h
#define type_checking_h

#include "value.h"
#include <stdbool.h>

typedef enum {
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_NIL,
    TYPE_FUNCTION,
    TYPE_CLASS,
    TYPE_INSTANCE,
    TYPE_NATIVE,
    TYPE_CLOSURE,
    TYPE_BOUND_METHOD,
    TYPE_UPVALUE
} ValueType;

ValueType getValueType(Value value);
const char* getTypeName(ValueType type);
bool checkType(Value value, ValueType expected);
bool validateArithmeticOperation(Value left, Value right, int line);
bool validateComparisonOperation(Value left, Value right, int line);
bool validateLogicalOperation(Value left, Value right, int line);
bool validateAssignment(Value target, Value value, int line);
bool validateFunctionCall(Value callee, int argCount, int line);
bool validateTypeMethodCall(Value object, Value method, int line);
bool validateTypeFieldAccess(Value object, int line);
bool validateIndexAccess(Value array, Value index, int line);
bool isNumeric(Value value);
bool isString(Value value);
bool isBoolean(Value value);
bool isCallable(Value value);
bool isObject(Value value);
bool isInstance(Value value);
bool isClass(Value value);

#endif 