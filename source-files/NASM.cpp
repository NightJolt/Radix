#include "NASM.h"

string NASM::ToVar(const string& str) {
    return "var_" + str;
}

string NASM::ToTempVar(int i) {
    return "@tmp" + to_string(i);
}

string NASM::ToAnon(int i) {
    return "anon_" + to_string(i);
}

string NASM::ToFun(const string& str) {
    if (str == Radix::CORE_FUN_NAME) return CORE_FUN_NAME;

    return "fun_" + str;
}

string NASM::SizeToReg(unsigned int size, char type, char sub_type) {
    if (type == 'A') return size == 4 ? A_32 : (size == 2 ? A_16 : (sub_type == 'H' ? A_8H : A_8L));
    if (type == 'B') return size == 4 ? B_32 : (size == 2 ? B_16 : (sub_type == 'H' ? B_8H : B_8L));
    if (type == 'C') return size == 4 ? C_32 : (size == 2 ? C_16 : (sub_type == 'H' ? C_8H : C_8L));

    return size == 4 ? D_32 : (size == 2 ? D_16 : (sub_type == 'H' ? D_8H : D_8L));
}

string NASM::IdToType(int i) {
    return type_specifiers[i];
}

string NASM::ToScopeEnd(const string& str) {
    return ".end_" + str;
}

string NASM::ToRetVal(const string& str) {
    return ".@ret" + str;
}
