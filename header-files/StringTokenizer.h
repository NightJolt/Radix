#pragma once

#include "globals.h"

using namespace std;

class StringTokenizer {
public:

    StringTokenizer(const char* const*, int, bool, bool);
    ~StringTokenizer();

    void Process(const string&);
    string NextToken();
    bool TokenLeft();

    StringTokenizer& operator = (const StringTokenizer&) = delete;

    void Clear();

private:

    void RegToken(const string&);
    void ClearTokens();

    int token_index;
    int delims_count;
    bool include_delims;
    bool exclude_empty;

    vector <string> tokens;

    vector <SuffixData*> delims_data;
};