#include "variables.h"

#include "tokeniser.h"
#include "utils.h"

#include <iostream>

using namespace std;

void VarDeclaration()
{
    set<string> variables_tempo;

    if (getCurrent() != ID)
        Error("Un identificater était attendu");
    string var = getCurrentString();
    if (IsDeclared(var.c_str()))
        Error("Identificateur déjà déclaré");
    DeclaredVariables.insert(var);
    variables_tempo.insert(var);
    next();

    while (getCurrent() == COMMA)
    {
        next();

        if (getCurrent() != ID)
            Error("Un identificateur était attendu");
        var = getCurrentString();
        if (IsDeclared(var.c_str()))
            Error("Identificateur déjà déclaré");
        DeclaredVariables.insert(var);
        variables_tempo.insert(var);
        next();
    }
    if (getCurrent() != COLON)
        Error("':' attendu");
    next();

    if (getCurrentKeyword() == ARRAY)
    {
        next();
        if (getCurrent() != RBRACKET)
            Error("'[' attendu");
        next();
        const int len = stoi(getCurrentString());
        next();
        if (getCurrent() != LBRACKET)
            Error("']' attendu");
        next();
        if (getCurrentKeyword() != OF)
            Error("'OF' attendu");
        next();
        const TYPE t = getCurrentType();
        cout << var << ":\t.space " << len * getTypeSize(t) << endl;
        Arrays[var] = {t, len};
        VariableType[var] = ARR;
    }
    else
    {
        const TYPE type = getCurrentType();

        for (const string& tempo : variables_tempo)
        {
            cout << getTypeDeclarationString(type, tempo).value() << endl;
            VariableType[tempo] = type;
        }
    }
    next();
}

void VarDeclarationPart()
{
    if (getCurrentKeyword() != VAR)
        Error("VAR attendu");

    next();
    VarDeclaration();

    while (getCurrent() == SEMICOLON)
    {
        next();
        VarDeclaration();
    }
    if (getCurrent() != DOT)
        Error("caractère '.' attendu");
    next();
}
