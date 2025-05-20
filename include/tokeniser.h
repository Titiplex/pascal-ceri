// tokeniser.h : shared definition for tokeniser.l and compilateur.cpp
#pragma once

enum TOKEN
{
    FEOF, UNKNOWN, KEYWORD, NUMBER, ID, STRINGCONST, RBRACKET, LBRACKET, RPARENT, LPARENT, COMMA,
    SEMICOLON, COLON, DOT, ADDOP, MULOP, RELOP, NOT, ASSIGN, FLOAT, CHARCONST
};

TOKEN getCurrent();
std::string getCurrentString();
int getLineNo();
void next();