#pragma once

#include "globals.h"

#include "StringTokenizer.h"

class Preprocessor {
public:

    static void Init(const string&, const char*);
    static void Term();

    static void ProcessFile(const string&);

private:

    static void ProcessLine(string&);
    static void ProcessMacro(const string&);

    static string main_path;
    static ofstream out_file;

    static constexpr char const* const delims[] = { "#", ":", " " };
    static StringTokenizer st;
};