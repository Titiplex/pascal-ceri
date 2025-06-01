#include "function.h"

#include <iostream>

#include "utils.h"
#include "tokeniser.h"

using namespace std;

void ParameterGroup(FunctionInfo *info, int &stackPosition)
{
    vector<string> args;
    while (getCurrent() == ID)
    {
        args.push_back(getCurrentString());
        next();
        if (getCurrent() == COMMA)
        {
            next();
            if (getCurrent() != ID) Error("Identifiant attendu");
        }
    }
    if (getCurrent() != COLON) Error("':' attendu");
    next();
    TYPE type = getCurrentType();
    for (string& arg : args)
    {
        info->args[arg] = make_pair(stackPosition, type);
        stackPosition += 8;
    }
}

void FunctionDeclarationHeading(FunctionInfo *info)
{
    if (getCurrent()!=ID) Error("Votre fonction doit avoir un nom");
    info->name = getCurrentString();
    next();
    if (getCurrent()==RPARENT)
    {
        int stackPosition = 16;
        next();
        ParameterGroup(info, stackPosition);
        next();
        while (getCurrent() == SEMICOLON)
        {
            next();
            ParameterGroup(info, stackPosition);
            next();
        }
        if (getCurrent() != LPARENT) Error("')' attendu");
        next();
    }
    if (info->returnType != VOID)
    {
        if (getCurrent() != COLON) Error("':' attendu");
        next();
        info->returnType = getCurrentType();
        next();
    }
    if (getCurrent() != SEMICOLON) Error("';' attendu");
    next();
}

void BlockStatement();

void FunctionDeclaration(FunctionInfo *info)
{
    FunctionDeclarationHeading(info);
    cout << "\n\t.globl " << info->name << endl;
    cout << info->name << ":" << endl;
    cout << "\tpush %rbp" << endl;
    cout << "\tmov %rsp, %rbp" << endl;
    if (getCurrentKeyword() != BEGIN) Error("BEGIN attendu");
    Functions.CurrentFunction = info;
    BlockStatement();
    if (info->returnType!=VOID) cout << "\tmov $0, %rax" << endl;
    cout << "\tmov %rbp, %rsp" << endl;
    cout << "\tpop %rbp" << endl;
    cout << "\tret" << endl;

    Functions.CurrentFunction = nullptr;
}

void FunctionDeclarationPart()
{
    while (getCurrent() == KEYWORD && (getCurrentKeyword() == FUNCTION || getCurrentKeyword() == PROCEDURE))
    {
        auto *functionInfo = new FunctionInfo{};
        functionInfo->returnType = (getCurrentKeyword() == FUNCTION ? INT : VOID);
        next();
        FunctionDeclaration(functionInfo);
        Functions.FunctionList[functionInfo->name] = functionInfo;
        if (getCurrent() == SEMICOLON) next();
    }

    if (getCurrent() != DOT) Error("'.' attendu");
    next();
}
