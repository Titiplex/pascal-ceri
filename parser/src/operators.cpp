#include "operators.h"

#include <cstring>

#include "tokeniser.h"

using namespace std;

OPMUL MultiplicativeOperator()
{
    OPMUL opmul;
    if (strcmp(getCurrentString().c_str(), "*") == 0)
        opmul = MUL;
    else if (strcmp(getCurrentString().c_str(), "/") == 0)
        opmul = DIV;
    else if (strcmp(getCurrentString().c_str(), "%") == 0)
        opmul = MOD;
    else if (strcmp(getCurrentString().c_str(), "&&") == 0)
        opmul = AND;
    else
        opmul = WTFM;
    next();
    return opmul;
}

OPADD AdditiveOperator()
{
    OPADD opadd;
    if (strcmp(getCurrentString().c_str(), "+") == 0)
        opadd = ADD;
    else if (strcmp(getCurrentString().c_str(), "-") == 0)
        opadd = SUB;
    else if (strcmp(getCurrentString().c_str(), "||") == 0)
        opadd = OR;
    else
        opadd = WTFA;
    next();
    return opadd;
}

OPREL RelationalOperator()
{
    OPREL oprel;
    if (strcmp(getCurrentString().c_str(), "==") == 0)
        oprel = EQU;
    else if (strcmp(getCurrentString().c_str(), "!=") == 0)
        oprel = DIFF;
    else if (strcmp(getCurrentString().c_str(), "<") == 0)
        oprel = INF;
    else if (strcmp(getCurrentString().c_str(), ">") == 0)
        oprel = SUP;
    else if (strcmp(getCurrentString().c_str(), "<=") == 0)
        oprel = INFE;
    else if (strcmp(getCurrentString().c_str(), ">=") == 0)
        oprel = SUPE;
    else
        oprel = WTFR;
    next();
    return oprel;
}