#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(const char* const* delims_arr, int delims_cnt, bool inc_delims, bool exc_empty)
: token_index(-1), delims_count(delims_cnt), include_delims(inc_delims), exclude_empty(exc_empty) {
    LOOP(i, 0, delims_cnt) {
        delims_data.push_back(new SuffixData(delims_arr[i]));
    }
}

StringTokenizer::~StringTokenizer() {
    LOOP(it, delims_data.begin(), delims_data.end()) {
        delete *it;
    }
}

void StringTokenizer::Process(const string& str) {
    ClearTokens();

    int ind[delims_count];
    fill(ind, ind + delims_count, 0);

    string buffer;

    LOOP(i, 0, str.length()) {
        buffer += str[i];

        LOOP(j, 0, delims_count) {
            SuffixData* sd = delims_data[j];

            while (ind[j] && str[i] != sd->pat[ind[j]]) {
                ind[j] = sd->lps[ind[j] - 1];
            }

            if (str[i] == sd->pat[ind[j]]) {
                if (++ind[j] == strlen(sd->pat)) {
                    RegToken(buffer.substr(0, buffer.size() - ind[j]));
                    if (include_delims) RegToken(sd->pat);

                    buffer = "";

                    fill(ind, ind + delims_count, 0);

                    break;
                }
            }
        }
    } RegToken(buffer);
}

void StringTokenizer::RegToken(const string& token) {
    if (exclude_empty && count(token.begin(), token.end(), ' ') == token.length()) return;

    tokens.emplace_back(token);
}

string StringTokenizer::NextToken() {
    return tokens[++token_index];
}

bool StringTokenizer::TokenLeft() {
    return token_index + 1 < tokens.size();
}

void StringTokenizer::ClearTokens() {
    tokens.erase(tokens.begin(), tokens.begin() + token_index + 1);

    token_index = -1;
}

void StringTokenizer::Clear() {
    tokens.clear();

    token_index = -1;
}