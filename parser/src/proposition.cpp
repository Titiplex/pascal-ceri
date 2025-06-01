#include "proposition.h"
#include "type.h"
#include "operators.h"

#include "tokeniser.h"
#include <iostream>

using namespace std;

TYPE Factor() // NOLINT(*-no-recursion)
{
    if (getCurrent() == RPARENT)
    {
        next();
        const TYPE t = Expression();
        if (getCurrent() != LPARENT)
            Error("')' était attendu");
        next();
        return t;
    }
    if (getCurrent() == NOT)
    {
        next();
        if (const TYPE t = Factor(); t != BOOL)
            TypeError("'!' appliqué à un type non boolean");
        cout << "\t# NOT opération" << endl;
        return BOOL;
    }
    if (getCurrent() == NUMBER)
        return Number();
    if (getCurrent() == ID)
        return Identifier();
    if (getCurrent() == FLOAT)
        return Float();
    if (getCurrent() == CHARCONST)
        return Char();
    if (getCurrent() == STRINGCONST)
        return String();
    Error("'(' ou chiffre ou lettre attendue");
    return INT;
}

TYPE Term() // NOLINT(*-no-recursion)
{
    const TYPE factor = Factor();
    while (getCurrent() == MULOP)
    {
        const OPMUL mulop = MultiplicativeOperator(); // Save operator in local variable
        if (const TYPE nextfactor = Factor(); factor != nextfactor)
            TypeError("Opérateur multiplicatif sur types incompatibles");

        if (factor == DB)
        {
            switch (mulop)
            {
                case MUL: opDouble("fmul");
                    break;
                case DIV: opDouble("fdiv");
                    break;
                default: Error("op flottant interdit");
            }
        }
        else if (factor == INT)
        {
            cout << "\tpop %rbx" << endl; // get first operand
            cout << "\tpop %rax" << endl; // get second operand
            switch (mulop)
            {
                case AND: cout << "\tmulq	%rbx" << endl; // a * b -> %rdx:%rax
                    cout << "\tpush %rax\t# AND" << endl; // store result
                    break;
                case MUL: cout << "\tmulq	%rbx" << endl; // a * b -> %rdx:%rax
                    cout << "\tpush %rax\t# MUL" << endl; // store result
                    break;
                case DIV: cout << "\tmovq $0, %rdx" << endl; // Higher part of numerator
                    cout << "\tdiv %rbx" << endl; // quotient goes to %rax
                    cout << "\tpush %rax\t# DIV" << endl; // store result
                    break;
                case MOD: cout << "\tmovq $0, %rdx" << endl; // Higher part of numerator
                    cout << "\tdiv %rbx" << endl; // remainder goes to %rdx
                    cout << "\tpush %rdx\t# MOD" << endl; // store result
                    break;
                default: Error("opérateur multiplicatif attendu");
            }
        }
        else if (factor == BOOL)
        {
            if (mulop != AND)          // seul && est permis dans *Term*
                Error("Seul '&&' est permis sur les booléens");

            cout << "\tpop %rbx\n";    // op2
            cout << "\tpop %rax\n";    // op1
            cout << "\tand %rbx,%rax\n";   // TRUE = -1, FALSE = 0
            cout << "\tpush %rax\n";
        }
        else
            TypeError("operation interdite sur ce type");
    }
    return factor;
}

TYPE SimpleExpression() // NOLINT(*-no-recursion)
{
    const TYPE term = Term();
    while (getCurrent() == ADDOP)
    {
        const OPADD adop = AdditiveOperator(); // Save operator in local variable
        if (const TYPE nextterm = Term(); term != nextterm)
            TypeError("Opérateur additif sur types incompatibles");

        if (term == DB)
        {
            switch (adop)
            {
                case ADD: opDouble("fadd");
                    break;
                case SUB: opDouble("fsub");
                    break;
                default: Error("op flottant interdit");
            }
        }
        else if (term == INT)
        {
            cout << "\tpop %rbx" << endl; // get first operand
            cout << "\tpop %rax" << endl; // get second operand
            switch (adop)
            {
                case OR: cout << "\taddq	%rbx, %rax\t# OR" << endl; // operand1 OR operand2
                    break;
                case ADD: cout << "\taddq	%rbx, %rax\t# ADD" << endl; // add both operands
                    break;
                case SUB: cout << "\tsubq	%rbx, %rax\t# SUB" << endl; // substract both operands
                    break;
                default: Error("opérateur additif inconnu");
            }
        }
        else if (term == BOOL)
        {
            cout << "\tpop %rbx\n";
            cout << "\tpop %rax\n";
            if (adop != OR) Error("'||' attendu sur booléens");
            cout << "\tor %rbx,%rax\n";
            cout << "\tpush %rax\n";
        }
        else
            TypeError("operation interdite sur ce type");

        cout << "\tpush %rax" << endl; // store result
    }
    return term;
}

TYPE Expression() // NOLINT(*-no-recursion)
{
    const TYPE se = SimpleExpression();
    if (getCurrent() == RELOP)
    {
        const OPREL oprel = RelationalOperator();
        if (const TYPE nextse = SimpleExpression(); se != nextse)
            TypeError("Comparaison sur types incompatibles");
        cout << "\tpop %rax" << endl;
        cout << "\tpop %rbx" << endl;
        cout << "\tcmpq %rax, %rbx" << endl;
        switch (oprel)
        {
            case EQU: cout << "\tje Vrai" << incrementTagNumber() << "\t# If equal" << endl;
                break;
            case DIFF: cout << "\tjne Vrai" << incrementTagNumber() << "\t# If different" << endl;
                break;
            case SUPE: cout << "\tjae Vrai" << incrementTagNumber() << "\t# If above or equal" << endl;
                break;
            case INFE: cout << "\tjbe Vrai" << incrementTagNumber() << "\t# If below or equal" << endl;
                break;
            case INF: cout << "\tjb Vrai" << incrementTagNumber() << "\t# If below" << endl;
                break;
            case SUP: cout << "\tja Vrai" << incrementTagNumber() << "\t# If above" << endl;
                break;
            default: Error("Opérateur de comparaison inconnu");
        }
        cout << "\tpush $0\t\t# False" << endl;
        cout << "\tjmp Suite" << getTagNumber() << endl;
        cout << "Vrai" << getTagNumber() << ":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True" << endl;
        cout << "Suite" << getTagNumber() << ":" << endl;
        return BOOL;
    }
    return se;
}
