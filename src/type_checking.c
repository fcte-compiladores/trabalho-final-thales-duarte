#include "type_checking.h"
#include "errors.h"
#include "object.h"

ValueType getValueType(Value value) {
    if (IS_NUMBER(value)) return TYPE_NUMBER;
    if (IS_BOOL(value)) return TYPE_BOOL;
    if (IS_NIL(value)) return TYPE_NIL;
    if (IS_OBJ(value)) {
        switch (OBJ_TYPE(value)) {
            case OBJ_STRING: return TYPE_STRING;
            case OBJ_FUNCTION: return TYPE_FUNCTION;
            case OBJ_NATIVE: return TYPE_NATIVE;
            case OBJ_CLOSURE: return TYPE_CLOSURE;
            case OBJ_UPVALUE: return TYPE_UPVALUE;
            case OBJ_CLASS: return TYPE_CLASS;
            case OBJ_INSTANCE: return TYPE_INSTANCE;
            case OBJ_BOUND_METHOD: return TYPE_BOUND_METHOD;
            default: return TYPE_NIL;
        }
    }
    return TYPE_NIL;
}

const char* getTypeName(ValueType type) {
    switch (type) {
        case TYPE_NUMBER: return "number";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "boolean";
        case TYPE_NIL: return "nil";
        case TYPE_FUNCTION: return "function";
        case TYPE_CLASS: return "class";
        case TYPE_INSTANCE: return "instance";
        case TYPE_NATIVE: return "native function";
        case TYPE_CLOSURE: return "closure";
        case TYPE_BOUND_METHOD: return "bound method";
        case TYPE_UPVALUE: return "upvalue";
        default: return "unknown";
    }
}

bool checkType(Value value, ValueType expected) {
    return getValueType(value) == expected;
}

bool validateArithmeticOperation(Value left, Value right, int line) {
    if (!IS_NUMBER(left) || !IS_NUMBER(right)) {
        reportTypeError(line, "number", getTypeName(getValueType(left)));
        return false;
    }
    return true;
}

bool validateComparisonOperation(Value left, Value right, int line) {
    ValueType leftType = getValueType(left);
    ValueType rightType = getValueType(right);
    
    if (leftType != rightType) {
        if (leftType == TYPE_NIL || rightType == TYPE_NIL) {
            return true;
        }
        if (leftType == TYPE_NUMBER && rightType == TYPE_NUMBER) {
            return true;
        }
        if (leftType == TYPE_STRING && rightType == TYPE_STRING) {
            return true;
        }
        if (leftType == TYPE_BOOL && rightType == TYPE_BOOL) {
            return true;
        }
        
        reportTypeError(line, getTypeName(leftType), getTypeName(rightType));
        return false;
    }
    
    return true;
}

bool validateLogicalOperation(Value left, Value right, int line) {
    return true;
}

bool validateAssignment(Value target, Value value, int line) {
    return true;
}

bool validateFunctionCall(Value callee, int argCount, int line) {
    if (!isCallable(callee)) {
        reportTypeError(line, "callable", getTypeName(getValueType(callee)));
        return false;
    }
    return true;
}

bool validateTypeMethodCall(Value object, Value method, int line) {
    if (!isObject(object)) {
        reportTypeError(line, "object", getTypeName(getValueType(object)));
        return false;
    }
    return true;
}

bool validateTypeFieldAccess(Value object, int line) {
    if (!isObject(object)) {
        reportTypeError(line, "object", getTypeName(getValueType(object)));
        return false;
    }
    return true;
}

bool validateIndexAccess(Value array, Value index, int line) {
    if (!isObject(array)) {
        reportTypeError(line, "object", getTypeName(getValueType(array)));
        return false;
    }
    return true;
}

bool isNumeric(Value value) {
    return IS_NUMBER(value);
}

bool isString(Value value) {
    return IS_OBJ(value) && OBJ_TYPE(value) == OBJ_STRING;
}

bool isBoolean(Value value) {
    return IS_BOOL(value);
}

bool isCallable(Value value) {
    if (!IS_OBJ(value)) return false;
    
    switch (OBJ_TYPE(value)) {
        case OBJ_FUNCTION:
        case OBJ_NATIVE:
        case OBJ_CLOSURE:
        case OBJ_BOUND_METHOD:
            return true;
        default:
            return false;
    }
}

bool isObject(Value value) {
    return IS_OBJ(value);
}

bool isInstance(Value value) {
    return IS_OBJ(value) && OBJ_TYPE(value) == OBJ_INSTANCE;
}

bool isClass(Value value) {
    return IS_OBJ(value) && OBJ_TYPE(value) == OBJ_CLASS;
} 