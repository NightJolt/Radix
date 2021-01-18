#pragma once

#include "globals.h"

class Radix {
public:

    enum FUN_DEF_TYPE {
        NONE,
        FUN,
        FUNDEF
    };

    static int TypeSpecifierToId(const string&);
    static int ScopeToId(const string&);
    static FUN_DEF_TYPE FunDefType(const string&);
    static int GetOperatorId(const string&);
    static bool IsRet(const string&);

    static constexpr char const* const exp_delims[] = {
            "=",
            "==", "=/=", "<", ">", "<=", ">=", "<=>",
            "+", "-", "*", "/", "%",// "++", "--",
            "+=", "-=", "*=", "/=", "%=",
            "&&", "||",
            "&", "|", "^", "!", ">>", "<<", "~",
            "&=", "|=", "^=", "!=",
            ":", ",",
            " ", "(", ")" // extra line
    };

    static constexpr char const* const CORE_FUN_NAME = "start";

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
            ":", ","
    }; // todo right to left associativity
};