#include "NASM.h"

ofstream NASM::out_file = ofstream();

bool NASM::global_scope = true;

StringTokenizer NASM::st = StringTokenizer(delims, sizeof delims / sizeof *delims, true, true);

void NASM::Init(const char* file_name) {
    out_file.open(file_name);
}

void NASM::Assemble(const char* file_name) {
    ifstream file(file_name);

    string buffer;
    string code;

    while (getline(file, buffer)) {
        code += buffer;

        if (code[code.size() - 1] == '\\') {
            code.erase(code.end() - 1);
        } else {
            Analyse(code);
        }
    } Analyse(code);
}

void NASM::Analyse(const string& code) {
    if (code.empty()) return;

    st.Process(code);

    /*string first = st.NextToken();
    int type_id = Radix::IsTypeSpecifier(first);

    tokens.push_back(first);*/

    while (st.TokenLeft()) {

    }
}

void NASM::PushToAsm(const string& str) {
    out_file << str << endl;
}