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

    static void TokenizeCodeIntoInstructions(ifstream&);
    static void ProcessInstrcution(const string&);

    static void DefBuiltinFuns();
    static void DefBuiltinExit();

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

    static OP_TYPE IdentifyOp(const string&);
    static bool IsTempVar(const string&);
    static bool IsVar(const string&);
    static bool IsFun(const string&);

    static const char TEMP_VAR_IDENTIFIER = '@';

    static string EvalExp();

    static string Add(const string&, const string&);
    static string Sub(const string&, const string&);
    static string Equ(const string&, const string&);
    static string Deref(const string&);
    static string Ref(const string&);
    static string Call(const string&); //todo add args

    static StringTokenizer exp_tokenizer;

#pragma region VARIABLE_TYPES

    enum TYPE_CLUSTER {
        VOID,
        PRIMITIVE,
        COMPLEX
    };

    /*enum VAR_REP {
        VALUE,
        POINTER
    };*/

    struct VAR_TYPE {
        int type_id;
        TYPE_CLUSTER cluster;
    };

    struct VAR_DEF {
        unsigned int size;
        VAR_TYPE type;

        void SetType(const VAR_TYPE& vt) {
            type = vt;
            size = GetTypeSize(vt);
        }
    };

    struct VAR_STACK {
        VAR_DEF var;
        unsigned int offset;
        vector <int>* target_vec;
    };

    static unsigned int GetTypeSize(const VAR_TYPE&);
    static unsigned int GetVarSize(const string&);
    static VAR_TYPE ToType(const string&);

    /*
     e.x.
     struct point : i32 a, i32 b;
     */

    struct STRUCT_DEF {
        unsigned int size;
        string label;

        vector <string> var_name;
        vector <VAR_TYPE> var_type;
    };

    static vector <STRUCT_DEF> struct_defs;

    static int GetStructId(const string&);
    static STRUCT_DEF& GetStructById(int);
    //static STRUCT_DEF& GetStructByLabel(const string&);

#pragma endregion

#pragma region SCOPES

    struct FUN_DEF {
        FUN_DEF() { reset(); }

        VAR_TYPE ret_type;

        vector <VAR_TYPE> arg_types;
        vector <string> arg_names;

        void reset() {
            ret_type.cluster = TYPE_CLUSTER::VOID;

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
        int vars_defined;

        void reset() {
            label.clear();
            loop = false;
            is_fun = false;
            condition.clear();
            vars_defined = 0;
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

    static void Ret();

    static string NewTempVar(unsigned int);

    static void PushVarToStack(const string&, const VAR_DEF&);
    static unsigned int GetVarOffset(const string&);

    static vector <unsigned int> stack_frame; // number of bytes allocated in current stack
    static vector <VAR_STACK> stack_mem; // variabs in declaration order
    static unordered_map <string, vector <int>> local_var_defs; // variable id in stack_mem by name
    static int temp_var_def;

#pragma endregion
};