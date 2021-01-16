#pragma once

#include "globals.h"

class Data {
public:

    static unsigned int TypeSpecifierSize(int);
    static int GetOperatorPrecedence(int);

private:

    static constexpr const unsigned int type_sizes[] = { 1, 2, 4 };

    static constexpr const int operator_precedence[] = {
            2,
            8, 8, 9, 9, 9, 9, 10,
            12, 12, 13, 13, 13, //"++", "--",
            2, 2, 2, 2, 2,
            4, 3,
            7, 5, 6, 15, 11, 11, 15,
            2, 2, 2, 2, 2
    };
};