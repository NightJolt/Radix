#pragma once

#include "globals.h"

#include "StringTokenizer.h"
#include "Radix.h"
#include "NASM.h"
#include "Data.h"

class Compiler {
public:

    static void Init(const char*);
    static void Assemble(const char*);

private:

    static void TokenizeCodeIntoInstructions(ifstream&);
    static void ProcessInstrcution(const string&);

    static void PushStack();
    static void PopStack();

    static void AllocStack(unsigned int);
    static void FreeStack(unsigned int);

    static void PushVarToStack(const string&, unsigned int);
    static unsigned int GetVarOffsetFromStack(const string&);

    enum OP_TYPE {
        UNDEFINED,
        REGISTER,
        MEMORY,
        CONSTANT
    };

    static void PushInstruction(const string&, const string&, OP_TYPE, const string&, OP_TYPE, const string& = "");
    static void PushOperand(const string&, OP_TYPE);

    static void PushToAsm(const string&);
    static void SeparatorAsm();
    static void SpaceAsm();
    static void NewLineAsm();

    static ofstream out_file;

    static constexpr char const* const instruct_delims[] = { ";", "{", "}" };
    static StringTokenizer instruct_tokenizer;

    static StringTokenizer exp_tokenizer;

    struct GLOB_VAR_DEF {
        string name;
        int size;
    };

    struct STACK_VAR_DEF {
        unsigned int size;
        unsigned int offset;

        vector <int>* target_vec;
    };

    static vector <unsigned int> stack_frame;
    static vector <STACK_VAR_DEF> stack;

    static unordered_map <string, vector <int>> stack_var_ind;

    static vector <unsigned int> instruction_frame;
};