#pragma once

#include "globals.h"

using namespace std;

class StringTokenizer {
public:

    enum MODE {
        FIRST_TO_FIT,
        LAST_TO_FIT
    };

    StringTokenizer(const char* const*, int, bool, MODE);
    ~StringTokenizer();

    void Process(const string&);

    string NextToken();
    string NextTokenUnpopped();
    void Pop();
    bool TokensLeft();
    int TokensCount();

    StringTokenizer& operator = (const StringTokenizer&) = delete;

    void Clear();

private:

    void ResetBuffer(string&);

    void FirstToFit(const string&);
    void LastToFit(const string&);

    void RegToken(const string&);
    void ClearGarbageTokens();

    int token_index;
    int delims_count;
    bool include_delims;
    MODE mode;

    vector <string> tokens;

    vector <SuffixData*> delims_data;
};