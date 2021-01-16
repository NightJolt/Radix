#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(const char* const* delims_arr, int delims_cnt, bool inc_delims, MODE m)
: token_index(-1), delims_count(delims_cnt), include_delims(inc_delims), mode(m) {
    LOOP(i, 0, delims_cnt) {
        delims_data.push_back(new SuffixData(delims_arr[i]));
    }

    sort(delims_data.begin(), delims_data.end(), [] (SuffixData* a, SuffixData* b) { return *a < *b; });
}

StringTokenizer::~StringTokenizer() {
    LOOP(it, delims_data.begin(), delims_data.end()) {
        delete *it;
    }
}

void StringTokenizer::Process(const string& str) {
    ClearGarbageTokens();

    if (mode == MODE::FIRST_TO_FIT)
        FirstToFit(str);
    else
        LastToFit(str);
}

void StringTokenizer::ResetBuffer(string& str) {
    if (tokens.empty()) return;

    str = tokens.back();

    tokens.pop_back();
}

void StringTokenizer::FirstToFit(const string& str) {
    int ind[delims_count];
    fill(ind, ind + delims_count, 0);

    string buffer; //ResetBuffer(buffer);

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

void StringTokenizer::LastToFit(const string& str) {
    string buffer; //ResetBuffer(buffer);

    LOOP(i, 0, str.length()) {
        bool found_anything = false;
        char c = str[i];

        LOOP(j, 0, delims_data.size()) {
            const char* delim = delims_data[delims_data.size() - j - 1]->pat;
            int len = delims_data[delims_data.size() - j - 1]->lps.size();

            bool found = true;

            if (i + len > str.length()) continue;

            LOOP(k, 0, len) {
                int ind = i + k;

                if (str[ind] != delim[k]) {
                    found = false;

                    break;
                }
            }

            if (found) {
                i += len - 1;
                found_anything = true;

                RegToken(buffer); buffer = "";
                if (include_delims) RegToken(delim);

                break;
            }
        } if (!found_anything) buffer += c;
    } RegToken(buffer);
}

void StringTokenizer::RegToken(const string& token) {
    if (count(token.begin(), token.end(), ' ') == token.length()) return;

    tokens.emplace_back(token);
}

string StringTokenizer::NextToken() {
    return tokens[++token_index];
}

string StringTokenizer::NextTokenUnpopped() {
    return tokens[token_index + 1];
}

void StringTokenizer::Pop() {
    token_index++;
}

bool StringTokenizer::TokensLeft() {
    return token_index + 1 < tokens.size();
}

int StringTokenizer::TokensCount() {
    return (int)tokens.size() - token_index - 1;
}

void StringTokenizer::ClearGarbageTokens() {
    tokens.erase(tokens.begin(), tokens.begin() + token_index + 1);

    token_index = -1;
}

void StringTokenizer::Clear() {
    tokens.clear();

    token_index = -1;
}