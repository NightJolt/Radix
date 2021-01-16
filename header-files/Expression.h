#pragma once

#include "globals.h"

#include "Radix.h"
#include "Data.h"
#include "StringTokenizer.h"

class Expression {
public:

    static vector <string> InfixToPostfix(StringTokenizer&);

private:

    struct OP {
        string op;
        int prec;
    };
};