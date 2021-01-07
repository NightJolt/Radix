#include "Radix.h"

int Radix::IsTypeSpecifier(const string& str) {
    LOOP(i, 0, sizeof type_specifiers / sizeof *type_specifiers) {
        if (str == type_specifiers[i]) return i;
    }

    return -1;
}

bool Radix::IsFuncSpecifier(const string& str) {
    return str == "func";
}