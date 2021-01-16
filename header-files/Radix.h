#pragma once

#include "globals.h"

class Radix {
public:

    static int TypeSpecifierToId(const string&);
    static int ScopeToId(const string&);
    static bool IsFunSpecifier(const string&);
    static int GetOperatorId(const string&);

    static constexpr char const* const exp_delims[] = {
            "=",
            "==", "=/=", "<", ">", "<=", ">=", "<=>",
            "+", "-", "*", "/", "%",// "++", "--",
            "+=", "-=", "*=", "/=", "%=",
            "&&", "||",
            "&", "|", "^", "!", ">>", "<<", "~",
            "&=", "|=", "^=", "!=",
            ":",
            " ", "(", ")" // extra line
    };

private:

    static constexpr char const* const type_specifiers[] = { "i8", "i16", "i32" };
    //static constexpr char const* const character_specifiers[] = { };
    static constexpr char const* const scopes[] = { "{", "}" };

    static constexpr char const* const operators[] = {
            "=",
            "==", "=/=", "<", ">", "<=", ">=", "<=>",
            "+", "-", "*", "/", "%",// "++", "--",
            "+=", "-=", "*=", "/=", "%=",
            "&&", "||",
            "&", "|", "^", "!", ">>", "<<", "~",
            "&=", "|=", "^=", "!=",
            ":"
    };
};