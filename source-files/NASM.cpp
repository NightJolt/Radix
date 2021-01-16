#include "NASM.h"

string NASM::IdToTypeSpecifier(int i) {
    return type_specifiers[i];
}

string NASM::ToVar(const string& str) {
    return "var_" + str;
}

string NASM::ToFun(const string& str) {
    return "fun_" + str;
}