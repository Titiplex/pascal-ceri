//
// Created by Titiplex on 03/05/2025.
//

#include "../include/utils.h"
#include "../include/tokeniser.h"
#include <iostream>
#undef BEGIN
#undef END
#include <functional>
#include <set>
#include <string>
#include <optional>
#include <sstream>
using namespace std;

void Error(const string& s)
{
    cerr << "Ligne n°" << getLineNo() << ", lu : '" << getCurrentString() << "'(" << getCurrent() << "), mais ";
    cerr << s << endl;
    exit(-1);
}

void TypeError(const string& s)
{
    Error(s + " -> TypeError");
}

optional<string> getTypeDeclarationString(const TYPE type, const string& var)
{
    switch (type)
    {
        case BOOL: return var + ":\t.quad 0 # Boolean";
        case INT: return var + ":\t.quad 0 # Integer";
        case DB: return var + ":\t.double 0.0 # Double";
        case CH: return var + ":\t.byte 0 # CHAR";
        case STR: return var + ":\t.space 256 # String";
        default: TypeError("Type inconnu");
    }
    return nullopt;
}

KEYWORDS getCurrentKeyword()
{
    if (getCurrent() != KEYWORD)
    {
        Error("Token Illegal");
    }

    const string kw = getCurrentString();

    if (kw == "DISPLAY") return DISPLAY;
    if (kw == "IF") return IF;
    if (kw == "THEN") return THEN;
    if (kw == "ELSE") return ELSE;
    if (kw == "BEGIN") return BEGIN;
    if (kw == "END") return END;
    if (kw == "FOR") return FOR;
    if (kw == "TO") return TO;
    if (kw == "DOWNTO") return DOWNTO;
    if (kw == "WHILE") return WHILE;
    if (kw == "DO") return DO;
    if (kw == "VAR") return VAR;
    if (kw == "INTEGER") return INTEGER;
    if (kw == "BOOLEAN") return BOOLEAN;
    if (kw == "DOUBLE") return DOUBLE;
    if (kw == "CHAR") return CHAR;
    if (kw == "STEP") return STEP;
    if (kw == "STRING") return STRING;
    if (kw == "ARRAY") return ARRAY;
    if (kw == "OF") return OF;
    if (kw == "CASE") return CASE;
    if (kw == "FUNCTION") return FUNCTION;
    if (kw == "PROCEDURE") return PROCEDURE;
    if (kw == "RETURN") return RETURN;

    Error("Mot-clé non reconnu");
    exit(-1);
}

TYPE getCurrentType()
{
    switch (getCurrentKeyword())
    {
        case INTEGER: return INT;
        case BOOLEAN: return BOOL;
        case DOUBLE: return DB;
        case CHAR: return CH;
        case STRING: return STR;
        default: TypeError("Type non reconnu");
    }
    return INT;
}

bool IsDeclared(const char* id)
{
    return DeclaredVariables.contains(id);
}

// pour les doubles :
void opDouble(const string& op)
{
    cout << "\t# op double " << op << '\n'
        << "\tfldl   (%rsp)        # ST0 = op2\n"
        << "\tfldl   8(%rsp)       # ST0 = op1, ST1 = op2\n"
        << '\t' << op << "p            # ST0 = op1 " << op.substr(1) << " op2, dépile\n"
        << "\tfstpl  8(%rsp)       # écrase op1 par le résultat\n"
        << "\taddq   $8, %rsp      # jette op2 → RSP pointe sur le résultat\n";
}

int getTypeSize(const TYPE t)
{
    switch (t)
    {
        case VOID: return 0;
        case CH:
        case BOOL: return 1;
        case INT:
        case DB: return 8;
        case STR: return 256;
        default: Error("Type inconnu ou non mesurable");
    }
    return 8;
}

unsigned long TagNumber = 0;

unsigned long getTagNumber()
{
    return TagNumber;
}

unsigned long incrementTagNumber()
{
    return ++TagNumber;
}

string captureOutputOf(const function<void()>& f)
{
    const ostringstream oss;
    streambuf* oldCout = cout.rdbuf();
    cout.rdbuf(oss.rdbuf());
    f();
    cout.rdbuf(oldCout);
    return oss.str();
}

string nextLbl;

void setNextLbl()
{
    nextLbl = "Next" + to_string(incrementTagNumber()) + ":";
}

string getNextLbl()
{
    if (nextLbl.empty()) setNextLbl();
    return nextLbl;
}

string escapeString(const string& in)
{
    string out;
    for (const char c : in)
    {
        switch (c)
        {
            case '\\': out += "\\\\";
                break;
            case '"': out += "\\\"";
                break;
            case '\n': out += "\\n";
                break;
            case '\t': out += "\\t";
                break;
            default: out += c;
        }
    }
    return out;
}
