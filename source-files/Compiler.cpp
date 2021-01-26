#include "Compiler.h"

ofstream Compiler::out_file = ofstream();

StringTokenizer Compiler::instruct_tokenizer = StringTokenizer(instruct_delims, sizeof instruct_delims / sizeof *instruct_delims, true, StringTokenizer::FIRST_TO_FIT);
StringTokenizer Compiler::exp_tokenizer = StringTokenizer(Radix::exp_delims, sizeof Radix::exp_delims / sizeof *Radix::exp_delims, true, StringTokenizer::LAST_TO_FIT);

vector <unsigned int> Compiler::stack_frame = vector <unsigned int> ();
vector <Compiler::VAR_STACK> Compiler::stack_mem = vector <VAR_STACK> ();
vector <Compiler::SCOPE> Compiler::scope_frame = vector <SCOPE> ();
unordered_map <string, vector <int>> Compiler::local_var_defs = unordered_map <string, vector <int>> ();
unordered_map <string, Compiler::FUN_DEF> Compiler::fun_defs = unordered_map <string, FUN_DEF> ();

int Compiler::temp_var_def = -1;
int Compiler::anon_fun_def = -1;

Compiler::SCOPE Compiler::next_scope_params = SCOPE();

void Compiler::Init(const char* file_name) {
    out_file.open(file_name);
}

void Compiler::Assemble(const char* file_name) {
    ifstream file(file_name);

    TokenizeCodeIntoInstructions(file);

    AsmPush(NASM::LOGIC_SEGMENT); AsmNewLine();
    AsmPush(NASM::CORE_FUNCTION_CALL); AsmNewLine();

    DefBuiltinFuns();

    while (instruct_tokenizer.TokensLeft()) {
        ProcessInstrcution(instruct_tokenizer.NextToken());
    }

    //AsmPush(NASM::GLOBAL_SEGMENT); AsmNewLine();

    // ...

    //AsmPush(NASM::CONSTANT_SEGMENT); AsmNewLine();

    // ...
}

#pragma region BUILTIN

void Compiler::DefBuiltinFuns() {
    DefBuiltinExit();
    DefBuiltinIn();
    DefBuiltinOut();
}

void Compiler::DefBuiltinExit() {
    AsmPush("exit:"); AsmNewLine();

    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, NASM::SYS_EXIT, OP_TYPE::CONSTANT);
    PushInstruction(NASM::KERNEL_INTERRUPT);
}

void Compiler::DefBuiltinIn() {
    AsmPush("in:"); AsmNewLine();

    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, NASM::SYS_READ, OP_TYPE::CONSTANT);
    PushInstruction(NASM::MOV, NASM::B_32, OP_TYPE::REGISTER, NASM::STD_IN, OP_TYPE::CONSTANT);
    PushInstruction(NASM::KERNEL_INTERRUPT);

    PushInstruction(NASM::RET);
}

void Compiler::DefBuiltinOut() {
    AsmPush("out:"); AsmNewLine();

    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, NASM::SYS_WRITE, OP_TYPE::CONSTANT);
    PushInstruction(NASM::MOV, NASM::B_32, OP_TYPE::REGISTER, NASM::STD_OUT, OP_TYPE::CONSTANT);
    PushInstruction(NASM::KERNEL_INTERRUPT);

    PushInstruction(NASM::RET);
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

    temp_var_def = -1;

    string first_token = exp_tokenizer.NextTokenUnpopped();

    if (first_token == ";") return;
    if (first_token == "{") {
        PushScope();

        return;
    } else if (first_token == "}") {
        PopScope();

        return;
    }

#pragma region fun_def

    int scope_attr_id;
    next_scope_params.reset(); // { "def", "type", "if", "loop" };
    FUN_DEF fun;;

    while (true) {
        if ((scope_attr_id = Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped())) == -1) break;

        exp_tokenizer.Pop();

        if (scope_attr_id == 0) {
            next_scope_params.is_fun = true;

            next_scope_params.label = exp_tokenizer.NextToken();

            if (exp_tokenizer.TokensLeft() && Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) == -1)
                exp_tokenizer.Pop(); // ":"

            while (exp_tokenizer.TokensLeft()) {
                if (Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) != -1) break;

                fun.arg_types.push_back(Radix::GetPrimitiveId(exp_tokenizer.NextToken()));
                fun.arg_names.push_back(exp_tokenizer.NextToken());

                if (exp_tokenizer.TokensLeft() && Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) == -1)
                    exp_tokenizer.Pop(); // ","
            }
        } else if (scope_attr_id == 1) {
            exp_tokenizer.Pop(); // ":"

            fun.ret_type = Radix::GetPrimitiveId(exp_tokenizer.NextToken());
        } else if (scope_attr_id == 2) {
            exp_tokenizer.Pop(); // ":"

            while (exp_tokenizer.TokensLeft()) {
                if (Radix::GetScopeAttrId(exp_tokenizer.NextTokenUnpopped()) != -1) break;

                next_scope_params.condition += exp_tokenizer.NextToken();
            }
        } else if (scope_attr_id == 3) {
            next_scope_params.loop = true;
        }

        if (!exp_tokenizer.TokensLeft()) {
            if (!fun_defs.contains(next_scope_params.label)) fun_defs[next_scope_params.label] = fun;

            return;
        }
    }

#pragma endregion

    int type_id = Radix::GetPrimitiveId(exp_tokenizer.NextTokenUnpopped());

    if (type_id != -1) {
        exp_tokenizer.Pop();

        PushVarToStack(exp_tokenizer.NextTokenUnpopped(), type_id);

        if (exp_tokenizer.TokensCount() > 1) {
            EvalExp();
        } else {
            exp_tokenizer.Pop();
        }
    } else if (first_token == "ret") {
        exp_tokenizer.Pop();

        if (exp_tokenizer.TokensLeft()) {
            exp_tokenizer.Pop(); // ":"

            Ret(EvalExp());
        } else {
            Ret("0");
        }
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
    } else if (first_token == "in") {
        exp_tokenizer.Pop();
        exp_tokenizer.Pop(); // ":"

        In(EvalExp());
    } else if (first_token == "out") {
        exp_tokenizer.Pop();
        exp_tokenizer.Pop(); // ":"

        Out(EvalExp());
    } else if (first_token == "exit") {
        exp_tokenizer.Pop();

        Exit();
    } else {
        EvalExp();
    }
}

#pragma endregion

#pragma region MEMORY

string Compiler::NewTempVar(int type_id) {
    string var_name = NASM::ToTempVar(++temp_var_def);

    PushVarToStack(var_name, type_id);

    return var_name;
}

void Compiler::PushStack() {
    stack_frame.push_back(0);

    PushInstruction(NASM::PUSH, NASM::BASE_POINTER_32, OP_TYPE::REGISTER);
    PushInstruction(NASM::MOV, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, NASM::STACK_POINTER_32, OP_TYPE::REGISTER);
}

void Compiler::PopStack() {
    PushInstruction(NASM::POP, NASM::BASE_POINTER_32, OP_TYPE::REGISTER);
    AsmPush(NASM::RET); AsmNewLine();

    stack_frame.pop_back();
}

void Compiler::PushScope() {
    if (next_scope_params.is_fun) {
        AsmPush(NASM::ToFun(next_scope_params.label) + ":"); AsmNewLine();

        PushStack();
    } else {
        next_scope_params.label = NASM::ToAnon(++anon_fun_def);

        AsmPush("." + next_scope_params.label + ":"); AsmNewLine();
    }

    if (!next_scope_params.condition.empty()) {
        exp_tokenizer.Clear();
        exp_tokenizer.Process(next_scope_params.condition);

        string res = EvalExp();

        PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, res, IdentifyOp(res));
        PushInstruction(NASM::CMP, NASM::A_32, OP_TYPE::REGISTER, "0", OP_TYPE::CONSTANT);

        string label = next_scope_params.is_fun ? NASM::ToScopeEnd(NASM::ToFun(next_scope_params.label)) : NASM::ToScopeEnd(next_scope_params.label);

        PushInstruction(NASM::JE, label, OP_TYPE::CONSTANT);
    }

    scope_frame.push_back(next_scope_params);

    if (next_scope_params.is_fun) {
        FUN_DEF& fd = fun_defs[next_scope_params.label];
        unsigned int offset = 8;

        LOOP(i, 0, fd.arg_types.size()) {
            PushVarToStack(fd.arg_names[i], fd.arg_types[i]);

            string reg = NASM::SizeToReg(Data::GetPrimitiveSize(fd.arg_types[i]), 'A');

            PushInstruction(
                    NASM::MOV,
                    reg, OP_TYPE::REGISTER,
                    "[" + string(NASM::BASE_POINTER_32) + " + " + to_string(offset) + "]", OP_TYPE::CONSTANT
            );
            PushInstruction(NASM::MOV, fd.arg_names[i], OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);

            offset += Data::GetPrimitiveSize(fd.arg_types[i]);
        }
    }

    next_scope_params.reset();
}

void Compiler::PopScope() {
    SCOPE& scope = scope_frame.back();
    unsigned int memory = scope.stack_alloc;

    FreeStack(memory);

    while (memory) {
        const VAR_STACK& vs = stack_mem[stack_mem.size() - 1];

        memory -= Data::GetPrimitiveSize(vs.type_id);
        vs.target_vec->pop_back();
        stack_mem.pop_back();
    }

    string label = scope.is_fun ? NASM::ToFun(scope.label) : "." + scope.label;
    if (scope.loop) {
        AsmPush(string(NASM::JMP) + " " + label); AsmNewLine();
    }

    label = scope.is_fun ? NASM::ToScopeEnd(NASM::ToFun(scope.label)) : NASM::ToScopeEnd(scope.label);
    AsmPush(label + ":"); AsmNewLine();

    if (scope.label == Radix::CORE_FUN_NAME) Exit();

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

void Compiler::Ret(const string& ret_val) {
    FreeStack(stack_frame.back(), false);

    int target = -1;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        if (scope_frame[i].is_fun) {
            target = i;

            break;
        }
    }

    FUN_DEF& fd = fun_defs[scope_frame[target].label];
    unsigned int ret_addr_offset = 8;

    LOOP(i, 0, fd.arg_types.size()) {
        ret_addr_offset += Data::GetPrimitiveSize(fd.arg_types[i]);
    }

    int ret_type_id = fd.ret_type;
    unsigned int ret_type_size = Data::GetPrimitiveSize(ret_type_id);
    string ret_type = NASM::IdToType(fd.ret_type);

    bool is_ok = IsVar(ret_val);

    if (fd.ret_type != -1) {
        if (IdentifyOp(ret_val) == OP_TYPE::CONSTANT) {
            PushInstruction(
                    NASM::MOV,
                    "[" + string(NASM::BASE_POINTER_32) + " + " + to_string(ret_addr_offset) + "]", OP_TYPE::CONSTANT,
                    ret_val, OP_TYPE::CONSTANT,
                    ret_type
            );
        } else {
            PushInstruction(
                    NASM::MOV,
                    NASM::SizeToReg(ret_type_size, 'A'), OP_TYPE::REGISTER,
                    ret_val, OP_TYPE::MEMORY
            );

            PushInstruction(
                    NASM::MOV,
                    "[" + string(NASM::BASE_POINTER_32) + " + " + to_string(ret_addr_offset) + "]", OP_TYPE::CONSTANT,
                    NASM::SizeToReg(ret_type_size, 'A'), OP_TYPE::REGISTER
            );
        }
    }

    string label = scope_frame[target].is_fun ?
            NASM::ToScopeEnd(NASM::ToFun(scope_frame[target].label)) :
            NASM::ToScopeEnd(scope_frame[target].label);

    AsmPush(string(NASM::JMP) + " " + label); AsmNewLine();
}

void Compiler::Break(int cnt) {
    unsigned int free_mem = 0;
    int target = -1;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        free_mem += scope_frame[i].stack_alloc;

        if (scope_frame[i].loop) cnt--;

        if (!cnt) {
            target = i;

            break;
        }
    }

    FreeStack(free_mem, false);

    string label = scope_frame[target].is_fun ?
            NASM::ToScopeEnd(NASM::ToFun(scope_frame[target].label)) :
            NASM::ToScopeEnd(scope_frame[target].label);

    if (cnt > -1) {
        AsmPush(string(NASM::JMP) + " " + label);

        AsmNewLine();
    }
}

void Compiler::Continue(int cnt) {
    unsigned int free_mem = 0;
    int target = -1;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        free_mem += scope_frame[i].stack_alloc;

        if (scope_frame[i].loop) cnt--;

        if (!cnt) {
            target = i;

            break;
        }
    }

    FreeStack(free_mem, false);

    string label = scope_frame[target].is_fun ?
            NASM::ToFun(scope_frame[target].label) :
            "." + scope_frame[target].label;

    if (cnt > -1) {
        AsmPush(string(NASM::JMP) + " " + label);

        AsmNewLine();
    }
}

void Compiler::Skip(int cnt) {
    unsigned int free_mem = 0;
    int target = -1;

    for (int i = (int)scope_frame.size() - 1; i >= 0; i--) {
        free_mem += scope_frame[i].stack_alloc;

        cnt--;

        if (!cnt) {
            target = i;

            break;
        }
    }

    FreeStack(free_mem, false);

    string label = scope_frame[target].is_fun ?
            NASM::ToScopeEnd(NASM::ToFun(scope_frame[target].label)) :
            NASM::ToScopeEnd(scope_frame[target].label);

    if (cnt > -1) {
        AsmPush(string(NASM::JMP) + " " + label);

        AsmNewLine();
    }
}

void Compiler::PushVarToStack(const string& str, int type_id) {
    if (stack_frame.empty()) {
        // todo Push to globals
    } else {
        unsigned int size = Data::GetPrimitiveSize(type_id);

        AllocStack(size);
        scope_frame[scope_frame.size() - 1].stack_alloc += size;

        if (!local_var_defs.contains(str)) {
            local_var_defs[str] = vector <int> ();
        }

        VAR_STACK vs = {};

        vs.type_id = type_id;
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
    unsigned int size = Data::GetPrimitiveSize(stack_mem[ind].type_id);

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
    } else if (IsFun(str)) {
        return OP_TYPE::MEMORY;
    } else {
        return OP_TYPE::CONSTANT;
    }
}

void Compiler::PushInstruction(const string& instr, const string& op1, OP_TYPE t1, const string& op2, OP_TYPE t2, const string& type_specifier) {
    AsmPush(instr);

    if (!type_specifier.empty()) {
        AsmSpace();
        AsmPush(type_specifier);
    }

    if (t1 == OP_TYPE::UNDEFINED) {
        AsmNewLine();

        return;
    }

    AsmSpace();
    PushOperand(op1, t1);

    if (t2 == OP_TYPE::UNDEFINED) {
        AsmNewLine();

        return;
    }

    AsmSeparator();
    AsmSpace();
    PushOperand(op2, t2);
    AsmNewLine();
}

void Compiler::PushOperand(const string& op, OP_TYPE t) {
    if (t == OP_TYPE::CONSTANT) {
        AsmPush(op);
    } else if (t == OP_TYPE::REGISTER) {
        AsmPush(op);
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

        AsmPush(str);
    }
}

void Compiler::AsmPush(const string& str) {
    out_file << str;
}

void Compiler::AsmSeparator() {
    out_file << ',';
}

void Compiler::AsmSpace() {
    out_file << ' ';
}

void Compiler::AsmNewLine() {
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

                res.push(Ass(op2, op1));
            } else if (op == ":") {
                vector <string> args;

                while (!IsFun(res.top())) {
                    args.push_back(res.top()); res.pop();
                } reverse(args.begin(), args.end());

                string fun_name = res.top(); res.pop();

                res.push(Call(fun_name, args));
            } else if (op == "?") {
                string op1 = res.top(); res.pop();
                string op2 = res.top(); res.pop();

                res.push(Def(op2, op1));
            } else if (op == "$") {
                string op1 = res.top(); res.pop();

                res.push(Ref(op1));
            }
        } else {
            res.push(op);
        }
    }

    return res.top();
}

string Compiler::Add(const string& a, const string& b) {
    OP_TYPE type1 = IdentifyOp(a);
    OP_TYPE type2 = IdentifyOp(b);

    bool tmp1 = IsTempVar(a);
    bool tmp2 = IsTempVar(b);

    string op1 = a;
    string op2 = b;

    string out_var_name;

    //PushToAsm(";Add "  + op1 + " " + op2); NewLineAsm();

    if (type1 == OP_TYPE::CONSTANT && type2 == OP_TYPE::CONSTANT) {
        /*out_var_name = NewTempVar(NASM::DEFAULT_EXP_SIZE);
        const string type_specifier = NASM::IdToType(Data::SizeToId(NASM::DEFAULT_EXP_SIZE));

        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, a, OP_TYPE::CONSTANT, type_specifier);
        PushInstruction(NASM::ADD, out_var_name, OP_TYPE::MEMORY, b, OP_TYPE::CONSTANT, type_specifier);*/

        return to_string(stoi(op1) + stoi(op2));
    } else if (type1 == OP_TYPE::MEMORY && type2 == OP_TYPE::MEMORY) {
        unsigned int size = GetVarSize(op1);
        out_var_name = tmp1 ? op1 : tmp2 ? op2 : NewTempVar(Data::SizeToId(size));
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
            out_var_name = NewTempVar(Data::SizeToId(size));
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

    bool tmp1 = IsTempVar(a);
    bool tmp2 = IsTempVar(b);

    string op1 = a;
    string op2 = b;

    string out_var_name;

    //PushToAsm(";Add "  + op1 + " " + op2); NewLineAsm();

    if (type1 == OP_TYPE::CONSTANT && type2 == OP_TYPE::CONSTANT) {
        /*out_var_name = NewTempVar(NASM::DEFAULT_EXP_SIZE);
        const string type_specifier = NASM::IdToType(Data::SizeToId(NASM::DEFAULT_EXP_SIZE));

        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, a, OP_TYPE::CONSTANT, type_specifier);
        PushInstruction(NASM::ADD, out_var_name, OP_TYPE::MEMORY, b, OP_TYPE::CONSTANT, type_specifier);*/

        return to_string(stoi(op1) - stoi(op2));
    } else if (type1 == OP_TYPE::MEMORY && type2 == OP_TYPE::MEMORY) {
        unsigned int size = GetVarSize(op1);
        out_var_name = tmp1 ? op1 : tmp2 ? op2 : NewTempVar(Data::SizeToId(size));
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
        PushInstruction(NASM::SUB, reg, OP_TYPE::REGISTER, op2, OP_TYPE::MEMORY);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    } else if (type1 == OP_TYPE::MEMORY && type2 == OP_TYPE::CONSTANT) {
        unsigned int size = GetVarSize(op1);
        out_var_name = NewTempVar(Data::SizeToId(size));
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::MEMORY);
        PushInstruction(NASM::SUB, reg, OP_TYPE::REGISTER, op2, OP_TYPE::CONSTANT);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    } else {
        unsigned int size = GetVarSize(op1);
        out_var_name = NewTempVar(Data::SizeToId(size));
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, op1, OP_TYPE::CONSTANT);
        PushInstruction(NASM::SUB, reg, OP_TYPE::REGISTER, op2, OP_TYPE::MEMORY);
        PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    }

    return out_var_name;
}

string Compiler::Ass(const string& a, const string& b) {
    OP_TYPE type2 = IdentifyOp(b);

    if (type2 == OP_TYPE::CONSTANT) {
        unsigned int size = GetVarSize(a);
        const string type_specifier = NASM::IdToType(Data::SizeToId(size));

        PushInstruction(NASM::MOV, a, OP_TYPE::MEMORY, b, OP_TYPE::CONSTANT, type_specifier);
    } else {
        unsigned int size = GetVarSize(a);
        const string type_specifier = NASM::IdToType(Data::SizeToId(size));
        string reg = NASM::SizeToReg(size, 'A');

        PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, b, OP_TYPE::MEMORY);
        PushInstruction(NASM::MOV, a, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
    }

    return a;
}

string Compiler::Def(const string& a, const string& b) {
    unsigned int size = Data::GetPrimitiveSize(Radix::GetPrimitiveId(b));

    string out_var_name = NewTempVar(Data::SizeToId(size));
    string reg = NASM::SizeToReg(size, 'A');

    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, a, OP_TYPE::MEMORY);
    AsmPush(string(NASM::MOV) + " " + NASM::IdToType(Data::SizeToId(size)) + " " + reg + ", [" + NASM::A_32 + "] \n");
    PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);

    return out_var_name;
}

string Compiler::Ref(const string& a) {
    string out_var_name = NewTempVar(Data::SizeToId(NASM::DEFAULT_EXP_SIZE));
    string reg = NASM::SizeToReg(NASM::DEFAULT_EXP_SIZE, 'A');

    PushInstruction(NASM::LEA, reg, OP_TYPE::REGISTER, a, OP_TYPE::MEMORY);
    PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);

    return out_var_name;
}

string Compiler::Call(const string& a, vector <string>& args) {
    FUN_DEF& fd = fun_defs[a];

    string ret_val = fd.ret_type == -1 ? "0" : NewTempVar(fd.ret_type);

    for (int i = (int)args.size() - 1; i >= 0; i--) {
        string var = NewTempVar(fd.arg_types[i]);

        if (IdentifyOp(args[i]) == OP_TYPE::CONSTANT) {
            string type = NASM::IdToType(fd.arg_types[i]);

            PushInstruction(NASM::MOV, var, OP_TYPE::MEMORY, args[i], OP_TYPE::CONSTANT, type);
        } else {
            string reg = NASM::SizeToReg(Data::GetPrimitiveSize(fd.arg_types[i]), 'A');

            PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, args[i], OP_TYPE::MEMORY);
            PushInstruction(NASM::MOV, var, OP_TYPE::MEMORY, reg, OP_TYPE::REGISTER);
        }
    }

    AsmPush(NASM::CALL); AsmSpace();
    AsmPush(NASM::ToFun(a)); AsmNewLine();

    return ret_val;
}

void Compiler::In(const string& a) {
    PushInstruction(NASM::LEA, NASM::C_32, OP_TYPE::REGISTER, a, OP_TYPE::MEMORY);
    PushInstruction(NASM::MOV, NASM::D_32, OP_TYPE::REGISTER, to_string(GetVarSize(a)), OP_TYPE::CONSTANT);
    PushInstruction(NASM::CALL, "in", OP_TYPE::CONSTANT);
}

void Compiler::Out(const string& a) {
    string op = a;

    if (IdentifyOp(a) == OP_TYPE::CONSTANT) {
        int type_id = Data::SizeToId(NASM::DEFAULT_EXP_SIZE);

        op = NewTempVar(type_id);

        PushInstruction(NASM::MOV, op, OP_TYPE::MEMORY, a, OP_TYPE::CONSTANT, NASM::IdToType(type_id));
    }

    PushInstruction(NASM::LEA, NASM::C_32, OP_TYPE::REGISTER, op, OP_TYPE::MEMORY);
    PushInstruction(NASM::MOV, NASM::D_32, OP_TYPE::REGISTER, to_string(GetVarSize(op)), OP_TYPE::CONSTANT);
    PushInstruction(NASM::CALL, "out", OP_TYPE::CONSTANT);
}

void Compiler::Exit() {
    PushInstruction(NASM::CALL, "exit", OP_TYPE::CONSTANT);
}

#pragma endregion