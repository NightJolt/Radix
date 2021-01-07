#pragma once

#include "globals.h"
#include "StringTokenizer.h"
#include "Radix.h"

class NASM {
public:

    static void Init(const char*);
    static void Assemble(const char*);
    //static void Term();

private:

    static constexpr char const* delims[] = { " ", "i32", "=", "+" };

    static void Analyse(const string&);
    //static void ADD_VARS();
    static void PushToAsm(const string&);

    static constexpr char const* LOGICAL_SEGMENT = "segment .text";
    static constexpr char const* CONSTANTS_SEGMENT = "segment .bss";
    static constexpr char const* GLOBALS_SEGMENT = "segment .data";

    static vector <VAR_DEF> globals;
    static vector <VAR_DEF> constants;

    static vector <string> tokens;

    static bool global_scope;

    static StringTokenizer st;
    static ofstream out_file;

    enum Type {
        UNDEFINED,
        VARIABLE_DEFINITION,
        FUNCTION_DEFINITION,
        EXPRESSION
    };

    struct State {
        Type type = UNDEFINED;
    };
};