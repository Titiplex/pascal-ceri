#pragma once
#include <functional>
#include <optional>
#include <set>
#include <source_location>
#include <string>
#include <unordered_map>

#ifndef UTILS_H
#define UTILS_H

enum OPREL { EQU, DIFF, INF, SUP, INFE, SUPE, WTFR };

enum OPADD { ADD, SUB, OR, WTFA };

enum OPMUL { MUL, DIV, MOD, AND, WTFM };

enum TYPE { BOOL, INT, DB, CH, ARR, STR, VOID };

enum KEYWORDS
{
    DISPLAY, IF, THEN, ELSE, BEGIN, END, FOR, TO, DOWNTO, WHILE, DO, VAR, INTEGER, BOOLEAN, DOUBLE, CHAR, STEP,
    CASE, OF, STRING, ARRAY, FUNCTION, PROCEDURE, RETURN, UKN
};

struct ArrayInfo
{
    TYPE type;
    int length;
};

extern std::unordered_map<std::string, ArrayInfo> Arrays;

// généré automatiquement par CLion
template<>
struct std::hash<std::pair<int, TYPE>> {
    size_t operator()(const pair<int, TYPE>& p) const noexcept
    {
        return hash<int>()(p.first) ^ hash<TYPE>()(p.second);
    }
};

struct FunctionInfo
{
    std::string name;
    TYPE returnType;
    std::unordered_map<std::string, std::pair<int, TYPE>> args; // nom + type

    FunctionInfo() : returnType(VOID) {}
};

struct Functions
{
    std::unordered_map<std::string, FunctionInfo*> FunctionList{};
	FunctionInfo* CurrentFunction = nullptr;
};

extern std::set<std::string> DeclaredVariables;
extern std::unordered_map<std::string, TYPE> VariableType;

void Error(const std::string& s, const std::source_location& loc = std::source_location::current());
void TypeError(const std::string& s);
std::optional<std::string> getTypeDeclarationString(TYPE type, const std::string& var);
TYPE getCurrentType();
KEYWORDS getCurrentKeyword();
bool IsDeclared(const char* id);
void opDouble(const std::string& op);
int getTypeSize(TYPE t);
unsigned long getTagNumber();
unsigned long incrementTagNumber();
std::string captureOutputOf(const std::function<void()>& f);
void setNextLbl();
std::string getNextLbl();
std::string escapeString(const std::string& in);
void CheckArrayIndex(const std::string& name);

#endif //UTILS_H
