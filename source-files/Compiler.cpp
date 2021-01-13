#include "Compiler.h"

ofstream Compiler::out_file = ofstream();

void Compiler::Init(const char* file_name) {
    out_file.open(file_name);
}
