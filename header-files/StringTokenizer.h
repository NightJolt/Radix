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

    struct SuffixData {
        explicit SuffixData(const char* str) {
            pat = strdup(str);
            lps = vector <int> (strlen(pat));

            int len = 0;

            LOOP(i, 1, lps.size()) {
                if (pat[i] == pat[len]) {
                    lps[i] = ++len;
                } else if (len) {
                    len = lps[len - 1];
                    i--;
                }
            }
        }

        SuffixData(const char* s, vector <int>& l) : pat(strdup(s)), lps(l) {}

        ~SuffixData() {
            free(pat);
        }

        SuffixData(const SuffixData& sd) {
            pat = strdup(sd.pat);
            lps = sd.lps;
        }

        SuffixData& operator =(const SuffixData& sd) {
            if (&sd == this) return *this;

            pat = strdup(sd.pat);
            lps = sd.lps;

            return *this;
        }

        SuffixData(SuffixData&& sd) noexcept {
            pat = sd.pat;
            sd.pat = nullptr;
            lps = move(sd.lps);
        }

        bool operator <(const SuffixData& sd) const {
            return lps.size() < sd.lps.size();
        }

        char* pat;
        vector <int> lps;
    };

    vector <SuffixData*> delims_data;
};