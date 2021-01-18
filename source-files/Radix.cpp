#include "Radix.h"

int Radix::TypeSpecifierToId(const string& str) {
    int type_specifiers_count = sizeof type_specifiers / sizeof *type_specifiers;

    LOOP(i, 0, type_specifiers_count) {
        if (str == type_specifiers[i]) return i;
    }

    return -1;
}

int Radix::ScopeToId(const string& str) {
    int scope_specifiers_count = sizeof scopes / sizeof *scopes;

    LOOP(i, 0, scope_specifiers_count) {
        if (str == scopes[i]) return i;
    }

    return -1;
}

Radix::FUN_DEF_TYPE Radix::FunDefType(const string& str) {
    return str == "fun" ? FUN_DEF_TYPE::FUN : str == "fundef" ? FUN_DEF_TYPE::FUNDEF : FUN_DEF_TYPE::NONE;
}

int Radix::GetOperatorId(const string& str) {
    int operators_count = sizeof operators / sizeof *operators;

    LOOP(i, 0, operators_count) {
        if (str == operators[i]) return i;
    }

    return -1;
}

bool Radix::IsRet(const string& str) {
    return str == "ret";
}
