#include "Data.h"

int Data::SizeToId(unsigned int i) {
    return i > 2 ? (int)i - 2 : (int)i - 1;
}

unsigned int Data::GetPrimitiveSize(int id) {
    return primitives[id];
}

int Data::GetOperatorPrecedence(int id) {
    return operator_precedence[id];
}