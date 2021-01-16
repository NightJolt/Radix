#include "Compiler.h"

ofstream Compiler::out_file = ofstream();

StringTokenizer Compiler::instruct_tokenizer = StringTokenizer(instruct_delims, sizeof instruct_delims / sizeof *instruct_delims, true, StringTokenizer::FIRST_TO_FIT);
StringTokenizer Compiler::exp_tokenizer = StringTokenizer(Radix::exp_delims, sizeof Radix::exp_delims / sizeof *Radix::exp_delims, true, StringTokenizer::LAST_TO_FIT);

vector <unsigned int> Compiler::stack_frame = vector <unsigned int> ();
vector <Compiler::STACK_VAR_DEF> Compiler::stack_mem = vector <STACK_VAR_DEF> ();

unordered_map <string, vector <int>> Compiler::stack_var_ind = unordered_map <string, vector <int>> ();
int Compiler::temp_var_def = -1;

vector <int> Compiler::scope_frame = vector <int> ();

void Compiler::Init(const char* file_name) {
    out_file.open(file_name);

    /*PushToAsm(CORE_FUNCTION_CALL); NewLineAsm();
    PushToAsm("_start:"); NewLineAsm();

    PushStack();

    PushVarToStack("cnt", 4);
    PushVarToStack("b", 2);

    PushStack();

    PushVarToStack("a", 4);
    PushVarToStack("b", 2);
    PushVarToStack("c", 1);
    PushVarToStack("d", 2);

    PushInstruction(MOV, "b", OP_TYPE::MEMORY, "99", OP_TYPE::CONSTANT, "byte");

    PushInstruction(MOV, A_32, OP_TYPE::REGISTER, "4", OP_TYPE::CONSTANT);
    PushInstruction(MOV, B_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushInstruction(LEA, C_32, OP_TYPE::REGISTER, "b", OP_TYPE::MEMORY);
    PushInstruction(MOV, D_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushToAsm(KERNEL_INTERRUPT); NewLineAsm();

    PushInstruction(MOV, A_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushToAsm(KERNEL_INTERRUPT); NewLineAsm();

    PopStack();

    PushInstruction(LEA, C_32, OP_TYPE::REGISTER, "b", OP_TYPE::MEMORY);

    PopStack();*/
}

void Compiler::Assemble(const char* file_name) {
    ifstream file(file_name);

    TokenizeCodeIntoInstructions(file);

    PushToAsm(NASM::LOGIC_SEGMENT); NewLineAsm();

    PushToAsm(NASM::CORE_FUNCTION_CALL); NewLineAsm();
    PushToAsm("_start:"); NewLineAsm();

    PushStack();
    PushScope();

    string line;

    while (instruct_tokenizer.TokensLeft()) {
        //cout << "New Instrcution" << endl;

        ProcessInstrcution(instruct_tokenizer.NextToken());

        /*exp_tokenizer.Process(instruct_tokenizer.NextToken());

        while (exp_tokenizer.TokensLeft()) {
            cout << "Token [" << exp_tokenizer.NextToken() << "]" << endl;
        }*/
    }

    PopScope();
    PopStack();

    PushInstruction(NASM::MOV, NASM::A_32, OP_TYPE::REGISTER, "1", OP_TYPE::CONSTANT);
    PushToAsm(NASM::KERNEL_INTERRUPT); NewLineAsm();

    PushToAsm(NASM::GLOBAL_SEGMENT); NewLineAsm();

    // ...

    PushToAsm(NASM::CONSTANT_SEGMENT); NewLineAsm();

    // ...
}

void Compiler::TokenizeCodeIntoInstructions(ifstream& file) {
    string line;
    string code;

    while (getline(file, line)) {
        if (!line.empty())
            code.append(" "),
            code += line;
    }

    instruct_tokenizer.Process(code);

    /*while (st.TokensLeft()) {
        cout << "Token: [" << st.NextToken() << "]" << endl;
    }*/
}

void Compiler::ProcessInstrcution(const string& str) {
    exp_tokenizer.Clear(); // Remove
    exp_tokenizer.Process(str);

    string first_token = exp_tokenizer.NextToken();

    int type_specifier_id = Radix::TypeSpecifierToId(first_token);

    if (type_specifier_id != -1) {
        unsigned int size = Data::TypeSpecifierSize(type_specifier_id);
        string var_name = NASM::ToVar(exp_tokenizer.NextTokenUnpopped());

        PushVarToStack(var_name, size);

        if (exp_tokenizer.TokensCount() > 1) {
            EvalExp();
        } else {
            exp_tokenizer.Pop();
        }
    } else if (Radix::IsFunSpecifier(first_token)) {
        string fun_name = NASM::ToFun(exp_tokenizer.NextToken());

        // todo def fun
    }
}

void Compiler::EvalExp() {
    vector <string> exp = Expression::InfixToPostfix(exp_tokenizer);

    stack <string> res;

    LOOP(i, 0, exp.size()) {
        const string& op = exp[i];
        int ip_ind = Radix::GetOperatorId(op);

        if (ip_ind != -1) {
            if (op == "+") {
                string op1 = res.top(); res.pop();
                string op2 = res.top(); res.pop();

                Add(op1, op2);
            }
        } else {
            res.push(op);
        }
    }

    temp_var_def = -1;
}

Compiler::OP_TYPE Compiler::IdentifyOp(const string& str) {
    if (str[0] == '$') {
        return OP_TYPE::MEMORY;
    } else if (stack_var_ind.contains(NASM::ToVar(str))) {
        return OP_TYPE::MEMORY;
    } else {
        return OP_TYPE::CONSTANT;
    }
}

string Compiler::Add(const string& a, const string& b) {
    OP_TYPE type_1 = IdentifyOp(a);
    OP_TYPE type_2 = IdentifyOp(b);

    string out_var_name = NASM::ToSVar(++temp_var_def);
    string reg = NASM::SizeToReg(4, 'A');

    PushVarToStack(out_var_name, 4);

    PushInstruction(NASM::MOV, reg, OP_TYPE::REGISTER, a, type_1);
    PushInstruction(NASM::ADD, reg, OP_TYPE::REGISTER, b, OP_TYPE::CONSTANT);
    PushInstruction(NASM::MOV, out_var_name, OP_TYPE::MEMORY, reg, OP_TYPE::CONSTANT);

    return out_var_name;
}

void Compiler::PushStack() {
    stack_frame.push_back(0);

    PushInstruction(NASM::PUSH, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, "", OP_TYPE::UNDEFINED);
    PushInstruction(NASM::MOV, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, NASM::STACK_POINTER_32, OP_TYPE::REGISTER);
}

void Compiler::PopStack() {
    PushInstruction(NASM::POP, NASM::BASE_POINTER_32, OP_TYPE::REGISTER, "", OP_TYPE::UNDEFINED);

    stack_frame.pop_back();
}

void Compiler::PushScope() {
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
}

void Compiler::AllocStack(unsigned int size) {
    stack_frame[stack_frame.size() - 1] += size;

    PushInstruction(NASM::SUB, NASM::STACK_POINTER_32, OP_TYPE::REGISTER, to_string(size), OP_TYPE::CONSTANT);
}

void Compiler::FreeStack(unsigned int size) {
    stack_frame[stack_frame.size() - 1] -=  size;

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