#pragma once

#include "globals.h"

#include "StringTokenizer.h"

class Preprocessor {
public:

    static void Init(string, const char*);
    static void Term();

    static void ProcessFile(const string&);

private:

    static void ProcessLine(string&);
    static void ProcessTag(const string&);

    static string main_path;
    static ofstream out_file;

    static constexpr char const* delims[] = { "#", ":", " " };
    static StringTokenizer st;

    //static map <string, string> def_val;
    //static map <string, vector <string>> def_args;
    //static vector <SuffixData*> def_data;
};