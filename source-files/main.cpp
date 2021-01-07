#include "globals.h"

#include "StringTokenizer.h"
#include "Preprocessor.h"
#include "NASM.h"

int main(int arg_cnt, char** arg_vec) {
    ios::sync_with_stdio(false);

    string main_file;
    string project_path;

    {
        const char* const delims[1] = { "/" };
        StringTokenizer st(delims, 1, true, false);

        st.Process(arg_vec[1]);

        while (true) {
            string token = st.NextToken();

            if (st.TokenLeft()) project_path += token;
            else {
                main_file = token;

                break;
            }
        }
    }

    //char* processed_file_name = strdup(main_file.c_str());
    //char* assembly_file_name

    Preprocessor::Init(project_path, "out.prad");
    Preprocessor::ProcessFile(main_file);
    Preprocessor::Term();

    /*NASM::Init(assembly_file_name);
    NASM::Assemble(processed_file_name);*/
    //NASM::Term();

    return 0;
}