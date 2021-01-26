#include "Preprocessor.h"

string Preprocessor::main_path = string();
ofstream Preprocessor::out_file = ofstream();

StringTokenizer Preprocessor::st = StringTokenizer(delims, sizeof delims / sizeof *delims, false, StringTokenizer::FIRST_TO_FIT);

void Preprocessor::Init(const string& path, const char* file_name) {
    main_path = path;

    out_file.open(file_name);
}

void Preprocessor::Term() {}

void Preprocessor::ProcessFile(const string& file_name) {
    ifstream file(main_path + file_name);

    string buffer;
    string line;

    while (getline(file, buffer)) {
        LOOP(i, 0, buffer.size()) {
            if (buffer[i] != ' ') {
                buffer.erase(0, i);

                break;
            }
        }

        line += buffer;

        if (!line.empty()) {
            if (line[line.size() - 1] == '\\') {
                line.erase(line.end() - 1);
            } else {
                ProcessLine(line);
            }
        }
    } ProcessLine(line);
}

void Preprocessor::ProcessLine(string& line) {
    if (line.empty()) return;

    if (line[0] == '#') {
        ProcessMacro(line);
    } else if (line[0] != '`') {
        out_file << line << endl;
    }

    line.clear();
}

void Preprocessor::ProcessMacro(const string& line) {
    st.Clear();

    st.Process(line);

    string tag = st.NextToken();

    if (tag == "add") {
        ProcessFile(st.NextToken());
    }
}