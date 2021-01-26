#pragma once

#include "globals.h"
#include "Radix.h"

class NASM {
public:

    static string ToVar(const string&);
    static string ToTempVar(int);
    static string ToFun(const string&);
    static string ToAnon(int);
    static string ToRetVal(const string&);
    static string ToScopeEnd(const string&);

    static string SizeToReg(unsigned int, char, char = 'L');
    static string IdToType(int);

    static constexpr const unsigned int DEFAULT_EXP_SIZE = 4;

    static constexpr char const* const LOGIC_SEGMENT = "segment .text";
    static constexpr char const* const CONSTANT_SEGMENT = "segment .bss";
    static constexpr char const* const GLOBAL_SEGMENT = "segment .data";

    static constexpr char const* const CORE_FUNCTION_CALL = "global _start";

    static constexpr char const* const KERNEL_INTERRUPT = "int 0x80";

    static constexpr char const* const INSTRUCTION_POINTER_32 = "eip";
    static constexpr char const* const STACK_POINTER_32 = "esp";
    static constexpr char const* const BASE_POINTER_32 = "ebp";

    static constexpr char const* const A_32 = "eax";
    static constexpr char const* const A_16 = "ax";
    static constexpr char const* const A_8H = "ah";
    static constexpr char const* const A_8L = "al";

    static constexpr char const* const B_32 = "ebx";
    static constexpr char const* const B_16 = "bx";
    static constexpr char const* const B_8H = "bh";
    static constexpr char const* const B_8L = "bl";

    static constexpr char const* const C_32 = "ecx";
    static constexpr char const* const C_16 = "cx";
    static constexpr char const* const C_8H = "ch";
    static constexpr char const* const C_8L = "cl";

    static constexpr char const* const D_32 = "edx";
    static constexpr char const* const D_16 = "dx";
    static constexpr char const* const D_8H = "dh";
    static constexpr char const* const D_8L = "dl";

    static constexpr char const* const ADD = "add";
    static constexpr char const* const SUB = "sub";
    static constexpr char const* const MUL = "mul";
    static constexpr char const* const IMUL = "imul";
    static constexpr char const* const DIV = "div";
    static constexpr char const* const IDIV = "idiv";

    static constexpr char const* const MOV = "mov";
    static constexpr char const* const LEA = "lea";

    static constexpr char const* const PUSH = "push";
    static constexpr char const* const POP = "pop";

    static constexpr char const* const CALL = "call";
    static constexpr char const* const RET = "ret";
    static constexpr char const* const JMP = "jmp";

    static constexpr char const* const CMP = "cmp";
    static constexpr char const* const JE = "je";

    static constexpr char const* const CORE_FUN_NAME = "_start";

    static constexpr char const* const SYS_EXIT = "1";
    static constexpr char const* const SYS_READ = "3";
    static constexpr char const* const SYS_WRITE = "4";
    static constexpr char const* const STD_IN = "0";
    static constexpr char const* const STD_OUT = "1";

private:

    static constexpr char const* const type_specifiers[] = { "byte", "word", "dword" };
};