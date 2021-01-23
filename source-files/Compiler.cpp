#include "Compiler.h"

ofstream Compiler::out_file = ofstream();

StringTokenizer Compiler::instruct_tokenizer = StringTokenizer(instruct_delims, sizeof instruct_delims / sizeof *instruct_delims, true, StringTokenizer::FIRST_TO_FIT);
StringTokenizer Compiler::exp_tokenizer = StringTokenizer(Radix::exp_delims, sizeof Radix::exp_delims / sizeof *Radix::exp_delims, true, StringTokenizer::LAST_TO_FIT);

vector <unsigned int> Compiler::stack_frame = vector <unsigned int> ();
vector <Compiler::VAR_STACK> Compiler::stack_mem = vector <VAR_STACK> ();
vector <Compiler::SCOPE> Compiler::scope_frame = vector <SCOPE> ();
unordered_map <string, vector <int>> Compiler::local_var_defs = unordered_map <string, vector <int>> ();
unordered_map <string, Compiler::FUN_DEF> Compiler::fun_defs = unordered_map <string, FUN_DEF> ();
vector <Compiler::STRUCT_DEF> Compiler::struct_defs = vector <STRUCT_DEF> ();

int Compiler::temp_var_def = -1;
int Compiler::anon_fun_def = -1;

Compiler::SCOPE Compiler::next_scope_params = SCOPE();

void Compiler::Init(const char* file_name) {
    out_file.open(file_name);
}

void Compiler::Assemble(const char* file_name) {
    ifstream file(file_name);

    TokenizeCodeIntoInstructions(file);

    PushToAsm(NASM::LOGIC_SEGMENT); NewLineAsm();
    PushToAsm(NASM::CORE_FUNCTION_CALL); NewLineAsm();

    //DefBuiltinFuns();

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

/*void Compiler::DefBuiltinFuns() {
    DefBuiltinExit();
}

void Compiler::DefBuiltinExit() {
    string fun_name;

    fun_name = NASM::ToFun("exit");
    funs[fun_name] = {};
    PushToAsm(fun_name+ ":"); NewLineAsm();
    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushToAsm(NASM::KERNEL_INTERRUPT); NewLineAsm();
}*/

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

    if (first_token == "struct") {
        exp_tokenizer.Pop();

        STRUCT_DEF sd;
        sd.label = exp_tokenizer.NextToken();

        exp_tokenizer.Pop(); // ":"

        while (exp_tokenizer.TokensLeft()) {
            VAR_TYPE vt = ToType(exp_tokenizer.NextToken());

            sd.offset.emplace_back(sd.size);
            sd.size += GetTypeSize(vt);
            sd.var_type.emplace_back(vt);
            sd.var_name.emplace_back(exp_tokenizer.NextToken());

            if (exp_tokenizer.TokensLeft()) exp_tokenizer.Pop(); // ","
        }

        if (GetStructId(sd.label) == -1) {
            struct_defs.push_back(sd);
        }

        return;
    }

#pragma region fun_def

    int scope_attr_id;
    next_scope_params.reset(); // { "fun", "type", "if", "loop" };
    FUN_DEF fun;

    while (true) {
        // todo maybe simplify
        if ((scope_attr_id = Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped())) == -1) break;

        exp_tokenizer.Pop();

        if (scope_attr_id == 0) {
            next_scope_params.is_fun = true;

            string fun_label = exp_tokenizer.NextToken();

            if (!fun_defs.contains(fun_label)) fun_defs[fun_label] = fun;
            next_scope_params.label = NASM::ToFun(fun_label);

            if (exp_tokenizer.TokensLeft() && Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) == -1)
                exp_tokenizer.Pop(); // ":"

            while (exp_tokenizer.TokensLeft()) {
                if (Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) != -1) break;

                fun.arg_types.push_back(ToType(exp_tokenizer.NextToken()));
                fun.arg_names.push_back(NASM::ToFun(exp_tokenizer.NextToken()));

                if (exp_tokenizer.TokensLeft() && Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) == -1)
                    exp_tokenizer.Pop(); // ","
            }
        } else if (scope_attr_id == 1) {
            exp_tokenizer.Pop(); // ":"

            fun.ret_type = ToType(exp_tokenizer.NextToken());
        } else if (scope_attr_id == 2) {
            exp_tokenizer.Pop(); // ":"

            while (exp_tokenizer.TokensLeft()) {
                if (Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) != -1) break;

                next_scope_params.condition += exp_tokenizer.NextToken();
            }
        } else if (scope_attr_id == 3) {
            next_scope_params.loop = true;
        }

        if (!exp_tokenizer.TokensLeft()) return;
    }

#pragma endregion

    VAR_TYPE type_specifier = ToType(first_token);

    if (type_specifier.cluster != TYPE_CLUSTER::VOID) {
        exp_tokenizer.Pop();

        VAR_DEF vd;
        vd.SetType(type_specifier);

        PushVarToStack(exp_tokenizer.NextTokenUnpopped(), vd);

        if (exp_tokenizer.TokensCount() > 1) {
            EvalExp();
        } else {
            exp_tokenizer.Pop();
        }
    } else if (first_token == "ret") {
        exp_tokenizer.Pop();
        exp_tokenizer.Pop(); // ":"

        Ret();
    } else if (first_token == "break") {
        exp_tokenizer.Pop();

        if (exp_tokenizer.TokensLeft()) {
            exp_tokenizer.Pop(); // ":"

            Break(stoi(exp_tokenizer.NextToken()));
        } else {
            Break(1);
        }
    } else if (first_token == "continue") {
        exp_tokenizer.Pop();

        if (exp_tokenizer.TokensLeft()) {
            exp_tokenizer.Pop(); // ":"

            Continue(stoi(exp_tokenizer.NextToken()));
        } else {
            Continue(1);
        }
    } else if (first_token == "skip") {
        exp_tokenizer.Pop();

        if (exp_tokenizer.TokensLeft()) {
            exp_tokenizer.Pop(); // ":"

            Skip(stoi(exp_tokenizer.NextToken()));
        } else {
            Skip(1);
        }
    } else {
        EvalExp();
    }
}

#pragma endregion

#pragma region VARIABLE_TYPES

Compiler::VAR_TYPE Compiler::ToType(const string& str) {
    VAR_TYPE vt;

    vt.type_id = Radix::GetPrimitiveId(str);

    if (vt.type_id != -1) {
        vt.cluster = TYPE_CLUSTER::PRIMITIVE;
    } else {
        vt.cluster = TYPE_CLUSTER::COMPLEX;
        vt.type_id = GetStructId(str);

        if (vt.type_id == -1) {
            vt.cluster = TYPE_CLUSTER::VOID;
        }
    }

    return vt;
}

unsigned int Compiler::GetTypeSize(const VAR_TYPE& vt) {
    if (vt.cluster == TYPE_CLUSTER::PRIMITIVE) {
        return Data::GetPrimitiveSize(vt.type_id);
    } else {
        return GetStructById(vt.type_id).size;
    }
}

unsigned int Compiler::GetStructOffset(int ind, const string& str) {
    LOOP(i, 0, struct_defs[ind].var_name.size()) {
        if (struct_defs[ind].var_name[i] == str) return struct_defs[ind].offset[i];
    }
}

Compiler::VAR_TYPE& Compiler::GetStructField(int ind, const string& str) {
    LOOP(i, 0, struct_defs[ind].var_name.size()) {
        if (struct_defs[ind].var_name[i] == str) return struct_defs[ind].var_type[i];
    }
}

int Compiler::GetStructId(const string& str) {
    LOOP(i, 0, struct_defs.size()) {
        if (str == struct_defs[i].label) return i;
    }

    return -1;
}

Compiler::STRUCT_DEF& Compiler::GetStructById(int i) {
    return struct_defs[i];
}

#pragma endregion

#pragma region MEMORY

string Compiler::NewTempVar(unsigned int size) {
    string var_name = NASM::ToTempVar(++temp_var_def);

    VAR_DEF vd;
    VAR_TYPE vt;

    vt.type_id = Data::SizeToId(size);
    vt.cluster = TYPE_CLUSTER::PRIMITIVE;

    vd.SetType(vt);

    PushVarToStack(var_name, vd);

    return var_name;
}

void Compiler::PushStack() {
    stack_frame.push_back(0);

    PushInstruction(NASM::PUSH, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, "", OP_TYPE::UNDEFINED);
    PushInstruction(NASM::MOV, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, NASM::STACK_POINTER_32, OP_TYPE::REGISTER);
}

void Compiler::PopStack() {
    PushInstruction(NASM::POP, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, "", OP_TYPE::UNDEFINED);
    PushToAsm(NASM::RET); NewLineAsm();

    stack_frame.pop_back();
}

void Compiler::PushScope() {
    if (next_scope_params.is_fun) {
        PushToAsm(next_scope_params.label + ":"); NewLineAsm();

        PushStack();
    } else {
        next_scope_params.label = NASM::ToAnon(++anon_fun_def);

        PushToAsm("." + next_scope_params.label + ":"); NewLineAsm();
    }

    if (!next_scope_params.condition.empty()) {
        exp_tokenizer.Clear();
        exp_tokenizer.Process(next_scope_params.condition);

        string res = "1";//EvalExp(); // todo change

        PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, res, IdentifyOp(res));
        PushInstruction(NASM::CMP, NASM::A_32, IdentifyOp(res), "0", OP_TYPE::CONSTANT);
        PushInstruction(NASM::JE, NASM::ToScopeEnd(next_scope_params.label), OP_TYPE::CONSTANT, "", OP_TYPE::UNDEFINED);
    }

    scope_frame.push_back(next_scope_params);
}

void Compiler::PopScope() {
    SCOPE& scope = scope_frame.back();
    unsigned int memory = scope.stack_alloc;

    FreeStack(memory);

    while (memory) {
        const VAR_STACK& vs = stack_mem[stack_mem.size() - 1];

        memory -= vs.var.size;
        vs.target_vec->pop_back();
        stack_mem.pop_back();
    }

    if (scope.loop) {
        PushToAsm(string(NASM::JMP) + " " + (scope.is_fun ? "" : ".") + scope.label);

        NewLineAsm();
    }

    PushToAsm(NASM::ToScopeEnd(scope.label) + ":"); NewLineAsm();

    if (scope.is_fun) PopStack();

    scope_frame.pop_back();
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

void Compiler::Ret() {
    FreeStack(stack_frame.back(), false);

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        if (scope_frame[i].is_fun) {
            PushToAsm( string(NASM::JMP) + " " + NASM::ToScopeEnd(scope_frame[i].label)); NewLineAsm();

            break;
        }
    }
}

void Compiler::Break(int cnt) {
    unsigned int free_mem = 0;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        free_mem += scope_frame[i].stack_alloc;

        if (scope_frame[i].loop) cnt--;

        if (!cnt) {
            PushToAsm(string(NASM::JMP) + " " + NASM::ToScopeEnd(scope_frame.back().label)); NewLineAsm();

            break;
        }
    }

    FreeStack(free_mem, false);
}

void Compiler::Continue(int cnt) {
    unsigned int free_mem = 0;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        free_mem += scope_frame[i].stack_alloc;

        if (scope_frame[i].loop) cnt--;

        if (!cnt) {
            PushToAsm(string(NASM::JMP) + " " + NASM::ToFun(scope_frame.back().label)); NewLineAsm();

            break;
        }
    }

    FreeStack(free_mem, false);
}

void Compiler::Skip(int cnt) {
    unsigned int free_mem = 0;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        free_mem += scope_frame[i].stack_alloc;

        cnt--;

        if (!cnt) {
            PushToAsm(string(NASM::JMP) + " " + NASM::ToScopeEnd(scope_frame[i].label)); NewLineAsm();

            break;
        }
    }

    FreeStack(free_mem, false);
}

void Compiler::PushVarToStack(const string& str, const VAR_DEF& vd) {
    if (stack_frame.empty()) {
        // todo Push to globals
    } else {
        AllocStack(vd.size);

        scope_frame[scope_frame.size() - 1].stack_alloc += vd.size;

        if (!local_var_defs.contains(str)) {
            local_var_defs[str] = vector <int> ();
        }

        VAR_STACK vs = {};

        vs.var = vd;
        vs.offset = stack_frame[stack_frame.size() - 1];
        vs.target_vec = &local_var_defs[str];

        vs.target_vec->push_back(stack_mem.size());

        stack_mem.push_back(vs);
    }
}

unsigned int Compiler::GetVarOffset(const string& str) {
    vector <int>* vec_ptr = &local_var_defs[str];
    int ind = (*vec_ptr)[vec_ptr->size() - 1];
    unsigned int offest = stack_mem[ind].offset;

    return offest;
}

unsigned int Compiler::GetVarSize(const string& str) {
    vector <int>* vec_ptr = &local_var_defs[str];
    int ind = (*vec_ptr)[vec_ptr->size() - 1];
    unsigned int size = stack_mem[ind].var.size;

    return size;
}

#pragma endregion

#pragma region ASSEMBLER

bool Compiler::IsTempVar(const string& str) {
    return str[0] == TEMP_VAR_IDENTIFIER;
}

bool Compiler::IsVar(const string& str) {
    return local_var_defs.contains(str);
}

bool Compiler::IsFun(const string& str) {
    return fun_defs.contains(str);
}

Compiler::OP_TYPE Compiler::IdentifyOp(const string& str) {
    if (IsTempVar(str)) {
        return OP_TYPE::MEMORY;
    } else if (IsVar(str)) {
        return OP_TYPE::MEMORY;
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
        unsigned int offset = GetVarOffset(op);

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

string Compiler::EvalExp() {
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

                string fun_name = res.top(); res.pop();

                res.push(Call(fun_name));
            } else if (op == "?") {
                string op1 = res.top(); res.pop();

                //res.push(Deref(op1));
            } else if (op == "$") {
                string op1 = res.top(); res.pop();

                res.push(Ref(op1)); // got todo smth here
            } else if (op == ".") {
                string op1 = res.top(); res.pop();
                string op2 = res.top(); res.pop();

               // res.push(MemAcc(op1));
            }
        } else {
            res.push(op);
        }
    }

    temp_var_def = -1;

    return res.top();
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
            const string type_specifier = NASM::IdToType(Data::SizeToId(size));

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

    //PushToAsm(";Equ "  + op1 + " " + op2); NewLineAsm();

    if (type2 == OP_TYPE::CONSTANT) {
        unsigned int size = GetVarSize(a);
        const string type_specifier = NASM::IdToType(Data::SizeToId(size));

        PushInstruction(NASM::MOV, a, OP_TYPE::MEMORY, b, OP_TYPE::CONSTANT, type_specifier);
    } else {
        unsigned int size = GetVarSize(a);
        const string type_specifier = NASM::IdToType(Data::SizeToId(size));
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, b, OP_TYPE::MEMORY, type_specifier);
        PushInstruction(NASM::MOV, a, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER, type_specifier);
    }

    return a;
}

/*string Compiler::Deref(const string& a) {
    const unsigned int size = 4;
    string out_var_name = NewTempVar(size);
    string reg = NASM::SizeToReg(size, 'A');

    string op1 = NASM::ToVar(a);

    PushInstruction(NASM::LEA, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
    PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);

    return "0";
}*/

string Compiler::Ref(const string& a) {
    const unsigned int size = 4;
    string out_var_name = NewTempVar(size);
    string reg = NASM::SizeToReg(size, 'A');

    string op1 = NASM::ToVar(a);

    PushInstruction(NASM::LEA, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
    PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);

    return out_var_name;
}

string Compiler::MemAcc(const string& a, const string& b) {
    int struct_id = GetStructId(a);

    unsigned int var_offset = GetVarOffset(a);
    unsigned int field_offset = var_offset + GetStructOffset(struct_id, b);

    string out_var_name = NewTempVar(GetTypeSize(GetStructField(struct_id, b)));
    /*string reg = NASM::SizeToReg(, 'A');

    string op1 = NASM::ToVar(a);

    PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);*/

    return out_var_name;
}

string Compiler::Call(const string& a) {
    PushToAsm(NASM::CALL); SpaceAsm();
    PushToAsm(NASM::ToFun(a)); NewLineAsm();

    return "0"; // todo its only temp
}

#pragma endregion