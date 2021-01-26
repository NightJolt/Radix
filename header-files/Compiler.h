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

    static ofstream out_file;

    static constexpr char const* const instruct_delims[] = { ";", "{", "}" };
    static StringTokenizer instruct_tokenizer;
    static StringTokenizer exp_tokenizer;

    static void TokenizeCodeIntoInstructions(ifstream&);
    static void ProcessInstrcution(const string&);

    static void DefBuiltinFuns();
    static void DefBuiltinExit();
    static void DefBuiltinIn();
    static void DefBuiltinOut();

    enum OP_TYPE {
        UNDEFINED,
        REGISTER,
        MEMORY,
        CONSTANT
    };

    static void PushInstruction(const string&, const string& = "", OP_TYPE = OP_TYPE::UNDEFINED, const string& = "", OP_TYPE = OP_TYPE::UNDEFINED, const string& = "");
    static void PushOperand(const string&, OP_TYPE);

    static void AsmPush(const string&);
    static void AsmSeparator();
    static void AsmSpace();
    static void AsmNewLine();

    static OP_TYPE IdentifyOp(const string&);
    static bool IsTempVar(const string&);

    static bool IsVar(const string&);
    static bool IsFun(const string&);

    static const char TEMP_VAR_IDENTIFIER = '@';

    static string EvalExp();

    static string Add(const string&, const string&);
    static string Sub(const string&, const string&);
    static string Ass(const string&, const string&);
    static string AssRef(const string&, const string&, const string&);
    static string Def(const string&, const string&);
    static string Ref(const string&);
    static string Call(const string&, vector <string>&);
    static string LogAnd(const string&, const string&);

    static void In(const string&);
    static void Out(const string&);
    static void Ret(const string&);
    static void Break(int);
    static void Continue(int);
    static void Skip(int);
    static void Exit();

#pragma region VARIABLES

    struct VAR_STACK {
        int type_id;
        unsigned int size;
        unsigned int offset;
        vector <int>* target_vec;
    };

    static unsigned int GetVarSize(const string&);

#pragma endregion

#pragma region SCOPES

    struct FUN_DEF {
        FUN_DEF() { reset(); }

        int ret_type;

        vector <int> arg_types;
        vector <string> arg_names;

        void reset() {
            ret_type = -1;

            arg_types.clear();
            arg_names.clear();
        }
    };

    static struct SCOPE {
        SCOPE() { reset(); }

        string label;
        bool loop;
        bool is_fun;
        string condition;
        unsigned int stack_alloc;

        void reset() {
            label.clear();
            loop = false;
            is_fun = false;
            condition.clear();
            stack_alloc = 0;
        }
    } next_scope_params;

    static vector <SCOPE> scope_frame;
    static unordered_map <string, FUN_DEF> fun_defs;
    static int anon_fun_def;

#pragma endregion

#pragma region MEMORY

    static void PushStack();
    static void PopStack();

    static void PushScope();
    static void PopScope();

    static void AllocStack(unsigned int);
    static void FreeStack(unsigned int, bool = true);

    static string NewTempVar(int);

    static void PushVarToStack(const string&, int, int = 1);
    static unsigned int GetVarOffset(const string&);

    static vector <unsigned int> stack_frame; // number of bytes allocated in current stack
    static vector <VAR_STACK> stack_mem; // variabs in declaration order
    static unordered_map <string, vector <int>> local_var_defs; // variable id in stack_mem by name
    static int temp_var_def;

#pragma endregion
};