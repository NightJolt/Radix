#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <memory>
#include <map>
#include <unordered_map>

#define LOOP(LOOP_I, LOOP_S, LOOP_E) for (auto (LOOP_I) = (LOOP_S); (LOOP_I) != (LOOP_E); (LOOP_I)++)

using namespace std;

template <typename T>
std::ostream& operator <<(std::ostream& out, const vector <T>& v) {
    LOOP(i, 0, v.size()) {
        out << v[i] << ((i < v.size() - 1) ? " " : "");
    }

    return out;
}

struct VAR_DEF {
    string name;
    string type;
    string val;
};

/*struct FUN_DEF {
    string name;
    vector <VAR_DEF> args;
};*/

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

    char* pat;
    vector <int> lps;
};