#include "globals.h"

#include "StringTokenizer.h"
#include "Preprocessor.h"
#include "Compiler.h"

int main(int arg_cnt, char** arg_vec) {
    ios::sync_with_stdio(false);

    string main_file;
    string project_path;

    {
        const char* const delims[1] = { "/" };
        StringTokenizer st(delims, 1, true, StringTokenizer::FIRST_TO_FIT);

        st.Process(arg_vec[1]);

        while (true) {
            string token = st.NextToken();

            if (st.TokensLeft()) project_path += token;
            else {
                main_file = token;

                break;
            }
        }
    }

    /*StringTokenizer st(Radix::exp_delims, sizeof Radix::exp_delims / sizeof *Radix::exp_delims, true, StringTokenizer::LAST_TO_FIT);
    st.Process("b = a$ + (wtf:);");
    vector <string> exp = Expression::InfixToPostfix(st);

    LOOP(i, 0, exp.size()) {
        cout << exp[i] << " ";
    } cout << endl;*/

    Preprocessor::Init(project_path, "out.prad");
    Preprocessor::ProcessFile(main_file);
    Preprocessor::Term();

    Compiler::Init("out.asm");
    Compiler::Assemble("out.prad");

    system("nasm -f elf out.asm");
    system("ld -m elf_i386 -s -o out out.o");

    return 0;
}