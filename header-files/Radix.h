#pragma once

#include "globals.h"

class Radix {
public:

    static int IsTypeSpecifier(const string&);
    static bool IsFuncSpecifier(const string&);

private:

    static constexpr char const* type_specifiers[] = { "i8", "i16" };

};