#include "Radix.h"

int Radix::GetPrimitiveId(const string& str) {
    int type_specifiers_count = sizeof primitives / sizeof *primitives;

    LOOP(i, 0, type_specifiers_count) {
        if (str == primitives[i]) return i;
    }

    return -1;
}

int Radix::GetOperatorId(const string& str) {
    int operators_count = sizeof operators / sizeof *operators;

    LOOP(i, 0, operators_count) {
        if (str == operators[i]) return i;
    }

    return -1;
}

int Radix::GetScopeAttrId(const string& str) {
    int operators_count = sizeof scope_attributes / sizeof *scope_attributes;

    LOOP(i, 0, operators_count) {
        if (str == scope_attributes[i]) return i;
    }

    return -1;
}
