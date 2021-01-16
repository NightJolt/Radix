#include "Expression.h"

vector <string> Expression::InfixToPostfix(StringTokenizer& st) {
    stack <OP> ops;
    vector <string> exp;

    while (st.TokensLeft()) {
        string op = st.NextToken();

        if (op == "(") {
            ops.emplace("(", -1); continue;
        } else if (op == ")") {
            while (ops.top().op != "(") {
                exp.push_back(ops.top().op); ops.pop();
            } ops.pop();

            continue;
        }

        int op_id = Radix::GetOperatorId(op);

        if (op_id != -1) {
            int prec = Data::GetOperatorPrecedence(op_id);

            while (!ops.empty() && ops.top().prec >= prec) {
                exp.push_back(ops.top().op); ops.pop();
            }

            ops.emplace(op, prec);
        } else {
            exp.push_back(op);
        }
    }

    while (!ops.empty()) {
        exp.push_back(ops.top().op);
        ops.pop();
    }

    return exp;
}