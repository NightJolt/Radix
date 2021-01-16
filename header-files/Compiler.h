#pragma once

#include "globals.h"

#include "StringTokenizer.h"
#include "Radix.h"
#include "NASM.h"
#include "Data.h"
#include "Expression.h"

class Compiler {
public:

    static void Init(const char*);
    static void Assemble(const char*);

private:

    static void TokenizeCodeIntoInstructions(ifstream&);
    static void ProcessInstrcution(const string&);

    static void PushStack();
    static void PopStack();

    static void PushScope();
    static void PopScope();

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

    static void EvalExp();

    static OP_TYPE IdentifyOp(const string&);

    static string Add(const string&, const string&);

    static string Equ(const string&, const string&);

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
    static vector <STACK_VAR_DEF> stack_mem;

    static unordered_map <string, vector <int>> stack_var_ind;
    static int temp_var_def;

    static vector <int> scope_frame;

    static vector <unsigned int> instruction_frame;
};