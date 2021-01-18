#include "NASM.h"

string NASM::IdToTypeSpecifier(int i) {
    return type_specifiers[i];
}

string NASM::SizeToTypeSpecifier(unsigned int i) {
    if (i > 2) i -= 2; else i--;

    return type_specifiers[i];
}

string NASM::ToVar(const string& str) {
    return "var_" + str;
}

string NASM::ToSVar(int i) {
    return "@tmp" + to_string(i);
}

string NASM::ToFun(const string& str) {
    return "fun_" + str;
}

string NASM::SizeToReg(unsigned int size, char type, char sub_type) {
    if (type == 'A') return size == 4 ? A_32 : (size == 2 ? A_16 : (sub_type == 'H' ? A_8H : A_8L));
    if (type == 'B') return size == 4 ? B_32 : (size == 2 ? B_16 : (sub_type == 'H' ? B_8H : B_8L));
    if (type == 'C') return size == 4 ? C_32 : (size == 2 ? C_16 : (sub_type == 'H' ? C_8H : C_8L));

    return size == 4 ? D_32 : (size == 2 ? D_16 : (sub_type == 'H' ? D_8H : D_8L));
}