#include "Data.h"

unsigned int Data::TypeSpecifierSize(int i) {
    return type_sizes[i];
}

int Data::GetOperatorPrecedence(int i) {
    return operator_precedence[i];
}