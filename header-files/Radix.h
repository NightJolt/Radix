#pragma once

#include "globals.h"

class Radix {
public:

    static int GetPrimitiveId(const string&);
    static int GetOperatorId(const string&);
    static int GetScopeAttrId(const string&);

    static constexpr char const* const exp_delims[] = {
            "=",
            "==", "=/=", "<", ">", "<=", ">=", "<=>",
            "+", "-", "*", "/", "%",// "++", "--",
            "+=", "-=", "*=", "/=", "%=", "?=",
            "&&", "||",
            "&", "|", "^", "!", ">>", "<<", "~",
            "&=", "|=", "^=", "!=",
            ":", ",",
            "?", "$",
            " ", "(", ")" // extra line
    };

    static constexpr char const* const CORE_FUN_NAME = "start";

private:

    static constexpr char const* const primitives[] = { "i8", "i16", "i32" };
    static constexpr char const* const scope_attributes[] = { "fun", "type", "if", "loop" };

    static constexpr char const* const operators[] = {
            "=",
            "==", "=/=", "<", ">", "<=", ">=", "<=>",
            "+", "-", "*", "/", "%",// "++", "--",
            "+=", "-=", "*=", "/=", "%=", "?=",
            "&&", "||",
            "&", "|", "^", "!", ">>", "<<", "~",
            "&=", "|=", "^=", "!=",
            ":", ",",
            "?", "$"
    };
};