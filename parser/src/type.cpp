#include "type.h"
#include "tokeniser.h"
#include "utils.h"
#include "proposition.h"
#include "function.h"

#include <iostream>

using namespace std;

TYPE ArrayUnit(const string& name)
{
    if (!Arrays.contains(name))
        TypeError(name + " n'est pas un tableau");
    const TYPE at = Arrays[name].type;
    next();
    if (const TYPE t = Expression(); t != INT)
        TypeError("index non entier");
    if (getCurrent() != LBRACKET)
        Error("']' attendu");
    next();

    cout << "\tpop %rax" << endl;
    CheckArrayIndex(name);
    const int size = getTypeSize(at);
    if (size != 1)
        cout << "\timul $" << size << ", %rax" << endl;
    cout << "\tleaq " << name << "(%rip), %rbx" << endl;
    cout << "\tadd %rax, %rbx" << endl;
    if (at == STR)
    {
        cout << "\tpush %rbx" << endl;
    }
    else
    {
        if (size == 1)
            cout << "\tmovzbq (%rbx), %rax" << endl;
        else
            cout << "\tmov" << (size >= 8 ? "q" : size == 4 ? "l" : "w") << " (%rbx), %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
    return at;
}

TYPE FuncProc(const string& name, const bool wantResult)
{
    next();
    vector<TYPE> inputArgs;
    vector<string> asmArgs;
    TYPE evaluatedExpression;
    inputArgs.push_back(Expression());
    while (getCurrent() == COMMA)
    {
        next();

        asmArgs.push_back(captureOutputOf([&evaluatedExpression] { evaluatedExpression = Expression(); }));
        inputArgs.push_back(evaluatedExpression);
    }

    if (getCurrent() != LPARENT) Error("')' attendu");
    next();

    const FunctionInfo& func = *Functions.FunctionList[name];
    if (func.args.size() != inputArgs.size())
        Error("Nombre d'arguments incorrect");

    const unsigned long long inputSize = inputArgs.size();

    for (int i = 0; i < inputSize; ++i)
    {
        auto it = func.args.begin();
        advance(it, i);
        if (it->second.second != inputArgs[i])
            TypeError("Type d'argument incompatible");
    }

    // push les args en ordre inverse (pile, pas queue)
    for (long long i = static_cast<long long>(asmArgs.size()) - 1; i >= 0; --i)
    {
        cout << asmArgs[i] << endl;
    }
    cout << "\tcall " << name << endl;
#ifndef _WIN32 // pas de stack frame sur windows
    cout << "\taddq $" << actual.size()*8 << ", %rsp\n";
#endif
    if (wantResult)   cout << "\tpush %rax\n";

    return func.returnType;
}

TYPE Identifier()
{
    const string name = getCurrentString();
    const bool in_vars = IsDeclared(name.c_str());
    const bool in_arrays = Arrays.contains(name);
    const bool in_funcs = Functions.FunctionList.contains(name);
    const bool in_args   = Functions.CurrentFunction
                     && Functions.CurrentFunction->args.contains(name);

    if (!(in_vars || in_arrays || in_funcs || in_args)) {
        Error("Variable '" + name + "' non déclarée");
    }
    const TYPE t = VariableType[name];
    next();
    // élément tableau
    if (Arrays.contains(name) && getCurrent() == RBRACKET)
        return ArrayUnit(name);
    // si dans def de fonction et ID est un arg de la fonction
    if (in_args)
    {
        FunctionInfo* func = Functions.CurrentFunction;
        const TYPE paramT = func->args[name].second;
        const int off = func->args[name].first;
        cout << "\tpushq " << off << "(%rbp)" << endl;
        return paramT;
    }
    // on execute une fonction pour l'assignation : a = func(b)
    if (in_funcs && getCurrent() == RPARENT)
    {
        return FuncProc(name, true);
    }
    if (t == STR)
    {
        cout << "\tleaq " << name << "(%rip), %rax" << endl;
        cout << "\tpush %rax" << endl;
    }
    else
        cout << "\tpushq " << name << "(%rip)" << endl;
    return t;
}

TYPE Number()
{
    cout << "\tpush $" << stoi(getCurrentString()) << endl;
    next();
    return INT;
}

TYPE Float()
{
    const double d = stof(getCurrentString());
    // i est un pointeur sur un espace de 8 octets sensé contenir un entier non signé en binaire naturel

    // const auto* i = reinterpret_cast<long long unsigned int*>(&d); // Maintenant, *i est un entier non signé sur 64
    // bits qui contient l'interprétation entière du codage IEEE754 du flottant 123.4 cout << "\tpush $"<<*i<<"\t#
    // empile le flottant "<<d<<endl;

    // solution du professeur ne marchant pas, voici une solution trouvée :
    const union
    {
        double f;
        unsigned long long i;
    } u{d}; // réinterprétation
    cout << "\tmovabs $" << u.i << ", %rax\t# bits du double " << d << endl;
    cout << "\tpush   %rax" << endl;

    next();
    return DB;
}

TYPE Char()
{
    const char c = getCurrentString()[1]; // apostrophes ?
    cout << "\tpush $" << static_cast<int>(c) << "\t# char '" << c << "'" << endl;
    next();
    return CH;
}

TYPE String()
{
    // push $ impossible car string jusqu'à 256 bits.
    const string s = getCurrentString().substr(1, getCurrentString().size() - 2);
    const string lbl = "String" + to_string(incrementTagNumber());
    cout << "\t.section .rodata" << endl;
    cout << lbl << ":\t.string \"" << escapeString(s) << "\"" << endl;
    cout << "\t.text" << endl;
    cout << "\tleaq " << lbl << "(%rip), %rax" << endl;
    cout << "\tpush %rax" << endl;
    next();
    return STR;
}
