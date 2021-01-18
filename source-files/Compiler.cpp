#include "Compiler.h"

ofstream Compiler::out_file = ofstream();

StringTokenizer Compiler::instruct_tokenizer = StringTokenizer(instruct_delims, sizeof instruct_delims / sizeof *instruct_delims, true, StringTokenizer::FIRST_TO_FIT);
StringTokenizer Compiler::exp_tokenizer = StringTokenizer(Radix::exp_delims, sizeof Radix::exp_delims / sizeof *Radix::exp_delims, true, StringTokenizer::LAST_TO_FIT);

vector <unsigned int> Compiler::stack_frame = vector <unsigned int> ();
vector <Compiler::STACK_VAR_DEF> Compiler::stack_mem = vector <STACK_VAR_DEF> ();
vector <int> Compiler::scope_frame = vector <int> ();
vector <unsigned int> Compiler::instruction_frame = vector <unsigned int> ();

unordered_map <string, vector <int>> Compiler::stack_var_ind = unordered_map <string, vector <int>> ();
unordered_map <string, Compiler::FUN_DEF> Compiler::funs = unordered_map <string, FUN_DEF> ();

int Compiler::temp_var_def = -1;

void Compiler::Init(const char* file_name) {
    out_file.open(file_name);
}

void Compiler::Assemble(const char* file_name) {
    ifstream file(file_name);

    TokenizeCodeIntoInstructions(file);

    PushToAsm(NASM::LOGIC_SEGMENT); NewLineAsm();
    PushToAsm(NASM::CORE_FUNCTION_CALL); NewLineAsm();

    DefBuiltinFuns();

    while (instruct_tokenizer.TokensLeft()) {
        ProcessInstrcution(instruct_tokenizer.NextToken());
    }

    /*PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, "4", OP_TYPE::CONSTANT);
    PushInstruction(NASM::MOV, NASM::B_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushInstruction(NASM::LEA, NASM::C_32, OP_TYPE::REGISTER, NASM::ToVar("a"), OP_TYPE::MEMORY);
    PushInstruction(NASM::MOV, NASM::D_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushToAsm(NASM::KERNEL_INTERRUPT); NewLineAsm();*/

    PushToAsm(NASM::GLOBAL_SEGMENT); NewLineAsm();

    // ...

    PushToAsm(NASM::CONSTANT_SEGMENT); NewLineAsm();

    // ...
}

#pragma region BUILTIN_FUNCTIONS

void Compiler::DefBuiltinFuns() {
    DefBuiltinExit();
}

void Compiler::DefBuiltinExit() {
    string fun_name;

    fun_name = NASM::ToFun("exit");
    funs[fun_name] = {};
    PushToAsm(fun_name+ ":"); NewLineAsm();
    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushToAsm(NASM::KERNEL_INTERRUPT); NewLineAsm();
}

#pragma endregion

#pragma region INSTRUCTIONIZE

void Compiler::TokenizeCodeIntoInstructions(ifstream& file) {
    string line;
    string code;

    while (getline(file, line)) {
        if (!line.empty())
            line.append(" "),
            code += line;
    }

    instruct_tokenizer.Process(code);
}

void Compiler::ProcessInstrcution(const string& str) {
    exp_tokenizer.Clear();
    exp_tokenizer.Process(str);

    string first_token = exp_tokenizer.NextTokenUnpopped();

    if (first_token == ";") return;
    if (first_token == "{") {
        PushScope();

        return;
    } else if (first_token == "}") {
        PopScope();

        return;
    }

    Radix::FUN_DEF_TYPE fdt = Radix::FunDefType(first_token);
    int type_specifier_id = Radix::TypeSpecifierToId(first_token);

    if (fdt) {
        exp_tokenizer.Pop();

        string fun_name = NASM::ToFun(exp_tokenizer.NextToken());

        if (exp_tokenizer.TokensLeft()) exp_tokenizer.Pop(); // extra : for simplicity

        FUN_DEF fd;

        while (exp_tokenizer.TokensCount()) {
            string type_name = exp_tokenizer.NextToken();
            string var_name = exp_tokenizer.NextToken();

            //fun_name += "_" + type_name;

            fd.arg_sizes.push_back(Data::TypeSpecifierSize(Radix::TypeSpecifierToId(type_name)));
            fd.arg_names.push_back(var_name);

            if (exp_tokenizer.TokensLeft()) exp_tokenizer.Pop(); // extra , for simplicity
        }

        if (!funs.contains(fun_name)) funs[fun_name] = fd;

        if (fdt == Radix::FUN_DEF_TYPE::FUN) {
            PushToAsm(fun_name + ":"); NewLineAsm();
        }
    } else if (type_specifier_id != -1) {
        exp_tokenizer.Pop();

        unsigned int size = Data::TypeSpecifierSize(type_specifier_id);
        string var_name = NASM::ToVar(exp_tokenizer.NextTokenUnpopped());

        PushVarToStack(var_name, size);

        if (exp_tokenizer.TokensCount() > 1) {
            EvalExp();
        } else {
            exp_tokenizer.Pop();
        }
    } if (Radix::IsRet(first_token)) {
        FreeStack(stack_frame.back(), false);

        PushToAsm(NASM::FUN_RET_CALL); NewLineAsm();
    } else {
        EvalExp();
    }
}

#pragma endregion

#pragma region MEMORY

string Compiler::NewTempVar(unsigned int size) {
    string var_name = NASM::ToSVar(++temp_var_def);
    PushVarToStack(var_name, size);

    return var_name;
}

void Compiler::PushStack() {
    stack_frame.push_back(0);

    PushInstruction(NASM::PUSH, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, "", OP_TYPE::UNDEFINED);
    PushInstruction(NASM::MOV, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, NASM::STACK_POINTER_32, OP_TYPE::REGISTER);
}

void Compiler::PopStack() {
    PushToAsm(NASM::FUN_RET_NAME); NewLineAsm();
    PushInstruction(NASM::POP, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, "", OP_TYPE::UNDEFINED);
    PushToAsm(NASM::RET); NewLineAsm();

    stack_frame.pop_back();
}

void Compiler::PushScope() {
    if (scope_frame.empty()) PushStack();

    scope_frame.push_back(0);
}

void Compiler::PopScope() {
    int cnt = scope_frame.back();
    scope_frame.pop_back();

    unsigned int mem_freed = 0;

    while (cnt--) {
        const STACK_VAR_DEF& svd = stack_mem[stack_mem.size() - 1];

        mem_freed += svd.size;
        svd.target_vec->pop_back();
        stack_mem.pop_back();
    }

    FreeStack(mem_freed);

    if (scope_frame.empty()) PopStack();
}

void Compiler::AllocStack(unsigned int size) {
    stack_frame[stack_frame.size() - 1] += size;

    PushInstruction(NASM::SUB, NASM::STACK_POINTER_32, OP_TYPE::REGISTER, to_string(size), OP_TYPE::CONSTANT);
}

void Compiler::FreeStack(unsigned int size, bool free_stack_frame) {
    if (!size) return;

    if (free_stack_frame) stack_frame[stack_frame.size() - 1] -=  size;

    PushInstruction(NASM::ADD, NASM::STACK_POINTER_32, OP_TYPE::REGISTER, to_string(size), OP_TYPE::CONSTANT);
}

void Compiler::PushVarToStack(const string& str, unsigned int size) {
    if (stack_frame.empty()) {
        // todo Push to globals
    } else {
        AllocStack(size);

        scope_frame[scope_frame.size() - 1]++;

        if (!stack_var_ind.contains(str)) {
            stack_var_ind[str] = vector <int> ();
        }

        STACK_VAR_DEF svd = {};

        svd.size = size;
        svd.offset = stack_frame[stack_frame.size() - 1];
        svd.target_vec = &stack_var_ind[str];

        svd.target_vec->push_back(stack_mem.size());

        stack_mem.push_back(svd);
    }
}

unsigned int Compiler::GetVarOffsetFromStack(const string& str) {
    vector <int>* vec_ptr = &stack_var_ind[str];
    int ind = (*vec_ptr)[vec_ptr->size() - 1];
    unsigned int offest = stack_mem[ind].offset;

    return offest;
}

unsigned int Compiler::GetVarSize(const string& str) {
    vector <int>* vec_ptr = &stack_var_ind[str];
    int ind = (*vec_ptr)[vec_ptr->size() - 1];
    unsigned int size = stack_mem[ind].size;

    return size;
}

#pragma endregion

#pragma region ASSEMBLER

bool Compiler::IsTempVar(const string& str) {
    return str[0] == TEMP_VAR_IDENTIFIER;
}

bool Compiler::IsVar(const string& str) {
    return stack_var_ind.contains(NASM::ToVar(str));
}

bool Compiler::IsFun(const string& str) {
    return funs.contains(NASM::ToFun(str));
}

Compiler::OP_TYPE Compiler::IdentifyOp(const string& str) {
    if (IsTempVar(str)) {
        return OP_TYPE::MEMORY;
    } else if (IsVar(str)) {
        return OP_TYPE::MEMORY;
    }  else if (IsFun(str)) {
        return OP_TYPE::INSTRUCTION;
    } else {
        return OP_TYPE::CONSTANT;
    }
}

void Compiler::PushInstruction(const string& instr, const string& op1, OP_TYPE t1, const string& op2, OP_TYPE t2, const string& type_specifier) {
    PushToAsm(instr);

    if (!type_specifier.empty()) {
        SpaceAsm();
        PushToAsm(type_specifier);
    }

    if (t1 == OP_TYPE::UNDEFINED) {
        NewLineAsm();

        return;
    }

    SpaceAsm();
    PushOperand(op1, t1);

    if (t2 == OP_TYPE::UNDEFINED) {
        NewLineAsm();

        return;
    }

    SeparatorAsm();
    SpaceAsm();
    PushOperand(op2, t2);
    NewLineAsm();
}

void Compiler::PushOperand(const string& op, OP_TYPE t) {
    if (t == OP_TYPE::CONSTANT) {
        PushToAsm(op);
    } else if (t == OP_TYPE::REGISTER) {
        PushToAsm(op);
    }else if (t == OP_TYPE::MEMORY) {
        unsigned int offset = GetVarOffsetFromStack(op);

        string str;

        str.append("[");
        str.append(NASM::BASE_POINTER_32);

        if (offset) {
            str.append(" - ");
            str.append(to_string(offset));
        }

        str.append("]");

        PushToAsm(str);
    }
}

void Compiler::PushToAsm(const string& str) {
    out_file << str;
}

void Compiler::SeparatorAsm() {
    out_file << ',';
}

void Compiler::SpaceAsm() {
    out_file << ' ';
}

void Compiler::NewLineAsm() {
    out_file << endl;
}

#pragma endregion

#pragma region EXPRESSION

void Compiler::EvalExp() {
    vector <string> exp = Expression::InfixToPostfix(exp_tokenizer);

    stack <string> res;

    LOOP(i, 0, exp.size()) {
        const string& op = exp[i];

        if (op == ",") continue;

        int ip_ind = Radix::GetOperatorId(op);

        if (ip_ind != -1) {
            if (op == "+") {
                string op1 = res.top(); res.pop();
                string op2 = res.top(); res.pop();

                res.push(Add(op2, op1));
            } else if (op == "-") {
                string op1 = res.top(); res.pop();
                string op2 = res.top(); res.pop();

                res.push(Sub(op2, op1));
            } else if (op == "=") {
                string op1 = res.top(); res.pop();
                string op2 = res.top(); res.pop();

                res.push(Equ(op2, op1));
            } else if (op == ":") {
                vector <string> args;

                while (!IsFun(res.top())) {
                    args.push_back(res.top()); res.pop();
                }

                res.push(Call(res.top())); res.pop();
            }
        } else {
            res.push(op);
        }
    }

    temp_var_def = -1;
}

string Compiler::Add(const string& a, const string& b) {
    OP_TYPE type1 = IdentifyOp(a);
    OP_TYPE type2 = IdentifyOp(b);

    string op1 = a;
    string op2 = b;

    bool tmp1 = IsTempVar(a);
    bool tmp2 = IsTempVar(b);

    string out_var_name;

    if (type1 == OP_TYPE::MEMORY && !tmp1) op1 = NASM::ToVar(a);
    if (type2 == OP_TYPE::MEMORY && !tmp2) op2 = NASM::ToVar(b);

    //PushToAsm(";Add "  + op1 + " " + op2); NewLineAsm();

    if (type1 == OP_TYPE::CONSTANT && type2 == OP_TYPE::CONSTANT) {
        /*out_var_name = NewTempVar(NASM::DEFAULT_EXP_SIZE);
        const string type_specifier = NASM::SizeToTypeSpecifier(NASM::DEFAULT_EXP_SIZE);

        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, op1, OP_TYPE::CONSTANT, type_specifier);
        PushInstruction(NASM::ADD, out_var_name, OP_TYPE::MEMORY, op2, OP_TYPE::CONSTANT, type_specifier);*/

        return to_string(stoi(op1) + stoi(op2));
    } else if (type1 == OP_TYPE::MEMORY && type2 == OP_TYPE::MEMORY) {
        unsigned int size = GetVarSize(op1);
        out_var_name = tmp1 ? op1 : tmp2 ? op2 : NewTempVar(size);
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
        PushInstruction(NASM::ADD, reg, OP_TYPE::REGISTER, op2, OP_TYPE::MEMORY);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    } else {
        if (type1 == OP_TYPE::CONSTANT && type2 == OP_TYPE::MEMORY) {
            swap(op1, op2);
            swap(tmp1, tmp2);
        }

        if (tmp1) {
            unsigned int size = GetVarSize(op1);
            out_var_name = op1;
            const string type_specifier = NASM::SizeToTypeSpecifier(size);

            PushInstruction(NASM::ADD, op1, OP_TYPE::MEMORY, op2, OP_TYPE::CONSTANT, type_specifier);
        } else {
            unsigned int size = GetVarSize(op1);
            out_var_name = NewTempVar(size);
            string reg = NASM::SizeToReg(size, 'A');

            PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
            PushInstruction(NASM::ADD, reg, OP_TYPE::REGISTER, op2, OP_TYPE::CONSTANT);
            PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
        }
    }

    return out_var_name;
}

string Compiler::Sub(const string& a, const string& b) {
    OP_TYPE type1 = IdentifyOp(a);
    OP_TYPE type2 = IdentifyOp(b);

    string op1 = a;
    string op2 = b;

    string out_var_name;

    if (type1 == OP_TYPE::MEMORY && !IsTempVar(a)) op1 = NASM::ToVar(a);
    if (type2 == OP_TYPE::MEMORY && !IsTempVar(b)) op2 = NASM::ToVar(b);

    //PushToAsm(";Sub "  + op1 + " " + op2); NewLineAsm();

    if (type1 == OP_TYPE::CONSTANT && type2 == OP_TYPE::CONSTANT) {
        /*out_var_name = NewTempVar(NASM::DEFAULT_EXP_SIZE);
        const string type_specifier = NASM::SizeToTypeSpecifier(NASM::DEFAULT_EXP_SIZE);

        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, op1, OP_TYPE::CONSTANT, type_specifier);
        PushInstruction(NASM::SUB, out_var_name, OP_TYPE::MEMORY, op2, OP_TYPE::CONSTANT, type_specifier);*/

        return to_string(stoi(op1) - stoi(op2));
    } else if (type1 == OP_TYPE::MEMORY && type2 == OP_TYPE::MEMORY) {
        unsigned int size = GetVarSize(op1);
        out_var_name = NewTempVar(size);
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
        PushInstruction(NASM::SUB, reg, OP_TYPE::REGISTER, op2, OP_TYPE::MEMORY);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    } else if (type1 == OP_TYPE::MEMORY && type2 == OP_TYPE::CONSTANT) {
        unsigned int size = GetVarSize(op1);
        out_var_name = NewTempVar(size);
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
        PushInstruction(NASM::SUB, reg, OP_TYPE::REGISTER, op2, OP_TYPE::CONSTANT);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    } else {
        unsigned int size = GetVarSize(op1);
        out_var_name = NewTempVar(size);
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::CONSTANT);
        PushInstruction(NASM::SUB, reg, OP_TYPE::REGISTER, op2, OP_TYPE::MEMORY);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    }

    return out_var_name;
}

string Compiler::Equ(const string& a, const string& b) {
    OP_TYPE type2 = IdentifyOp(b);

    string op1 = NASM::ToVar(a);
    string op2 = b;

    if (type2 == OP_TYPE::MEMORY && b[0] != TEMP_VAR_IDENTIFIER) op2 = NASM::ToVar(b);

    //PushToAsm(";Equ "  + op1 + " " + op2); NewLineAsm();

    if (type2 == OP_TYPE::CONSTANT) {
        unsigned int size = GetVarSize(op1);
        const string type_specifier = NASM::SizeToTypeSpecifier(size);

        PushInstruction(NASM::MOV, op1, OP_TYPE::MEMORY, op2, OP_TYPE::CONSTANT, type_specifier);
    } else {
        unsigned int size = GetVarSize(op1);
        const string type_specifier = NASM::SizeToTypeSpecifier(size);
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op2, OP_TYPE::MEMORY, type_specifier);
        PushInstruction(NASM::MOV, op1, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER, type_specifier);
    }

    return a;
}

string Compiler::Call(const string& a) {
    PushToAsm(NASM::CALL); SpaceAsm();
    PushToAsm(NASM::ToFun(a)); NewLineAsm();

    return a;
}

#pragma endregion