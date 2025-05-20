#pragma once
#include <functional>

using namespace std;
#include <unordered_map>
#include <map>
#include <optional>
#include <set>
#include <string>

#ifndef UTILS_H
#define UTILS_H

enum OPREL { EQU, DIFF, INF, SUP, INFE, SUPE, WTFR };

enum OPADD { ADD, SUB, OR, WTFA };

enum OPMUL { MUL, DIV, MOD, AND, WTFM };

enum TYPE { BOOL, INT, DB, CH, ARR, STR, VOID };

enum KEYWORDS
{
    DISPLAY, IF, THEN, ELSE, BEGIN, END, FOR, TO, DOWNTO, WHILE, DO, VAR, INTEGER, BOOLEAN, DOUBLE, CHAR, STEP,
    CASE, OF, STRING, ARRAY, FUNCTION, PROCEDURE, RETURN
};

struct ArrayInfo
{
    TYPE type;
    int length;
};

inline unordered_map<string, ArrayInfo> Arrays;

// généré automatiquement par CLion
template<>
struct std::hash<pair<int, string>> {
    size_t operator()(const pair<int, TYPE>& p) const noexcept
    {
        return hash<int>()(p.first) ^ hash<TYPE>()(p.second);
    }
};

struct FunctionInfo
{
    string name;
    TYPE returnType;
    unordered_map<string, pair<int, TYPE>> args; // nom + type

    FunctionInfo() : returnType(VOID) {}
};

struct Functions
{
    unordered_map<string, FunctionInfo*> FunctionList{};
	FunctionInfo* CurrentFunction = nullptr;
};

inline set<string> DeclaredVariables;
inline unordered_map<string, TYPE> VariableType;

void Error(const string& s);
void TypeError(const string& s);
optional<string> getTypeDeclarationString(TYPE type, const string& var);
TYPE getCurrentType();
KEYWORDS getCurrentKeyword();
bool IsDeclared(const char* id);
void opDouble(const string& op);
int getTypeSize(TYPE t);
unsigned long getTagNumber();
unsigned long incrementTagNumber();
string captureOutputOf(const function<void()>& f);
void setNextLbl();
string getNextLbl();
string escapeString(const string& in);

#endif //UTILS_H
