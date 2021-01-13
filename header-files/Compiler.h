#pragma once

#include "globals.h"

class Compiler {
public:

    static void Init(const char*);

private:

    static constexpr char const* LOGICAL_SEGMENT = "segment .text";
    static constexpr char const* CONSTANTS_SEGMENT = "segment .bss";
    static constexpr char const* GLOBALS_SEGMENT = "segment .data";

    static StringTokenizer st;
    static ofstream out_file;
};