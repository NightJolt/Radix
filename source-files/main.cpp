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

    char* prad_file = strdup(arg_vec[2]);
    char* asm_file = strdup(arg_vec[2]);

    strcat(prad_file, ".prad");
    strcat(asm_file, ".asm");

    Preprocessor::Init(project_path, prad_file);
    Preprocessor::ProcessFile(main_file);
    Preprocessor::Term();

    Compiler::Init(asm_file);
    Compiler::Assemble(prad_file);

    system((string("nasm -f elf ") + asm_file).c_str());
    system((string("ld -m elf_i386 -s -o ") + arg_vec[2] + " " + arg_vec[2] + ".o").c_str());

    return 0;
}