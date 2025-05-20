//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#undef BEGIN
#undef END
#include <cstring>
#include <optional>
#include "parser.h"
#include <sstream>
#include <unordered_map>

#include "tokeniser.h"
#include "utils.h"

using namespace std;

void CheckArrayIndex(const string& name)
{
    cout << "\t# Check array index for " << name << endl;
    cout << "\tcmp $0, %rax" << endl;
    cout << "\tjl ErrorArrayIndex \t# < 0 ?" << endl;
    cout << "\tcmp $" << Arrays[name].length << ", %rax" << endl;
    cout << "\tjge ErrorArrayIndex \t# > len ?" << endl;
}

TYPE Expression();

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

TYPE Identifier()
{
    const string name = getCurrentString();
    if (!IsDeclared(name.c_str()) && !Arrays.contains(name) && Functions.FunctionList.contains(name) && !Functions.
        CurrentFunction->args.contains(name))
    {
        Error("Variable '" + name + "' non déclarée");
    }
    const TYPE t = VariableType[name];
    next();
    if (Arrays.contains(name) && getCurrent() == RBRACKET)
        return ArrayUnit(name);
    if (FunctionInfo* func = Functions.CurrentFunction; func->args.contains(name))
    {
        const TYPE paramT = func->args[name].second;
        const int off = func->args[name].first;
        cout << "\tpushq " << off << "(%rbp)" << endl;
        return paramT;
    }
    if (getCurrent() == RPARENT)
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
        cout << "\taddq $" << inputSize * 8 << ", %rsp" << endl; // met le pointer en haut
        cout << "\tpush %rax" << endl;

        return func.returnType;
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

TYPE Factor()
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

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
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

// Term := Factor {MultiplicativeOperator Factor}
TYPE Term()
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
        else
            TypeError("operation interdite sur ce type");
    }
    return factor;
}

// AdditiveOperator := "+" | "-" | "||"
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

TYPE SimpleExpression()
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
        else
            TypeError("operation interdite sur ce type");

        cout << "\tpush %rax" << endl; // store result
    }
    return term;
}

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
    cout << "\t.data" << endl;
    cout << "\t.align 8" << endl;

    cout << "EAIMsg:\t.string \"Array index out of bounds\\n\"" << endl;

    cout << "FormatStringInt:\t.string \"%llu\\n\"" << endl; // le display
    cout << "FormatStringDouble:\t.string \"%f\\n\"" << endl; // le display
    cout << "FormatStringChar:\t.string \"%c\\n\"" << endl; // le display
    cout << "FormatStringString:\t.string \"%s\\n\"" << endl;
    cout << "TrueString:\t.string \"TRUE\\n\"" << endl;
    cout << "FalseString:\t.string \"FALSE\\n\"" << endl;

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

TYPE Expression()
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
        cout << "\tjmp Suite" << getTagNumber << endl;
        cout << "Vrai" << getTagNumber << ":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True" << endl;
        cout << "Suite" << getTagNumber << ":" << endl;
        return BOOL;
    }
    return se;
}

// AssignementStatement := Identifier ":=" Expression
void AssignementStatement()
{
    if (getCurrent() != ID)
        Error("Identificateur attendu");
    const string var = getCurrentString();
    if (!IsDeclared(var.c_str()))
        Error("Erreur : Variable non déclarée");
    const TYPE vt = VariableType[var];
    TYPE at = {};
    next();

    // si array
    const bool isArr = (vt == ARR && getCurrent() == RBRACKET);
    if (isArr)
    {
        if (!Arrays.contains(var))
            TypeError("Variable non array");
        at = Arrays[var].type;
        next();
        if (const TYPE exprArr = Expression(); exprArr != INT)
            TypeError("index non entier");
        if (getCurrent() != LBRACKET)
            Error("']' attendu");
        next();
    }
    else if (vt == ARR && getCurrent() != RBRACKET)
        Error("cette variable est un tableau, utilisez [...]");
    else if (vt != ARR && getCurrent() == RBRACKET)
        TypeError("cette variable n'est pas un tableau");

    if (getCurrent() != ASSIGN)
        Error("caractères ':=' attendus");
    next();
    const TYPE expr = Expression();
    if (isArr)
    {
        if (expr != at)
            TypeError("types incompatibles");
        cout << "\tpop %rbx\t# val/ptr" << endl;
        cout << "\tpop %rax\t# index" << endl;
        CheckArrayIndex(var);
        const int size = getTypeSize(at);
        if (size != 1)
            cout << "\timul $" << size << ", %rax" << endl;
        cout << "\tleaq " << var << "(%rip), %rcx" << endl;
        cout << "\tadd %rax, %rcx" << endl;
        if (at == STR)
        {
            cout << "\tmov  %rbx, %rsi\n"
                << "\tmov  %rcx, %rdi\n"
                << "\tmov  $256, %rcx\n"
                << "\tcld\n\trep movsb" << endl;
        }
        else
        {
            const char suf = size == 1 ? 'b' : size == 2 ? 'w' : size == 4 ? 'l' : 'q';
            cout << "\tmov" << suf << " %b" << (suf == 'q' ? 'x' : 'l')
                << ", (%rcx)" << endl;
        }
        return;
    }
    if (vt == STR)
    {
        if (expr != STR)
            TypeError("String attendu");
        cout << "\tpop %rsi" << endl;
        cout << "\tleaq " << var << "(%rip), %rdi" << endl;
        cout << "\tmov $256, %rcx" << endl;
        cout << "\tcld" << endl;
        cout << "\trep movsb" << endl;
        return;
    }

    if (vt != expr)
        TypeError("Affectation de type incompatible à '" + var + "'");
    cout << "\tpopq " << var << "(%rip)" << endl;
}

void Statement();

// IfStatement := "IF" Expression "THEN" Statement ["ELSE" Statement]
void IfStatement()
{
    const unsigned long lbl = incrementTagNumber();
    next();
    if (const TYPE cond = Expression(); cond != BOOL)
        TypeError("Condition IF non boolean");
    // test false
    cout << "\tpop %rax" << endl;
    cout << "\tcmp $0, %rax" << endl;
    cout << "\tje Else" << lbl << endl;
    // THEN
    if (getCurrentKeyword() != THEN)
        Error("'THEN' attendu");
    next();
    Statement();
    cout << "\tjmp EndIf" << lbl << endl;
    cout << "Else" << lbl << ":" << endl;
    if (getCurrentKeyword() == ELSE)
    {
        next();
        Statement();
    }
    cout << "EndIf" << lbl << ":" << endl;
}

void WhileStatement()
{
    const unsigned long lbl = incrementTagNumber();
    cout << "While" << lbl << ":" << endl;
    next();
    if (const TYPE cond = Expression(); cond != BOOL)
        TypeError("Condition WHILE non boolean");
    cout << "\tpop %rax" << endl;
    cout << "\tcmp $0, %rax" << endl;
    cout << "\tje EndWhile" << lbl << endl;
    if (getCurrentKeyword() != DO)
        Error("'DO' attendu");
    next();
    Statement();
    cout << "\tjmp While" << lbl << endl;
    cout << "EndWhile" << lbl << ":" << endl;
}

// ForStatement := "FOR" AssignementStatement "TO" Expression "DO" Statement
void ForStatement()
{
    const auto lbl = incrementTagNumber();
    next();
    if (getCurrent() != ID)
        Error("Identificateur attendu après FOR"); // recup loopvar qui sera consommé dans assign
    const string loopVar = getCurrentString();
    AssignementStatement();
    cout << "\tpushq " << loopVar << "(%rip)" << endl; // on remet le compteur sur la pile

    if (getCurrentKeyword() != TO)
        Error("'TO' attendu dans FOR");
    // TODO DOWNTO
    next();
    if (const TYPE lim = Expression(); lim != INT)
        TypeError("Expression non integer");

    // boucle
    cout << "For" << lbl << ":" << endl;
    cout << "\tpop %rbx    # borne sup" << endl;
    cout << "\tpop %rax    # compteur" << endl;
    cout << "\tcmp %rbx, %rax" << endl;
    cout << "\tjg EndFor" << lbl << "" << endl;

    string inc = "\taddq $1, " + loopVar + "(%rip)\n";
    if (getCurrentKeyword() == STEP)
    {
        next();
        inc = "\taddq $" + getCurrentString() + ", " + loopVar + "(%rip)\n";
    }

    if (getCurrentKeyword() != DO)
        Error("'DO' attendu");
    next();
    Statement();

    // incrément + re-push compteur+borne
    cout << inc;
    cout << "\tpushq " << loopVar << "(%rip)" << endl;
    cout << "\tpushq %rbx    # re-push borne" << endl;

    // retour test
    cout << "\tjmp For" << lbl << "" << endl;
    cout << "EndFor" << lbl << ":" << endl;
}

void BlockStatement()
{
    next();
    Statement();
    while (getCurrent() == SEMICOLON)
    {
        next();
        if (getCurrent() == KEYWORD && getCurrentKeyword() == END)
            break;
        Statement();
    }
    if (getCurrent() == KEYWORD && getCurrentKeyword() != END)
        Error("'END' attendu");
    next();
}

void DisplayStatement()
{
    next();
    string format = "FormatString";
    TYPE t = Expression();
    switch (t)
    {
        case INT: format += "Int";
            break;
        case DB: format += "Double";
            break;
        case CH: format += "Char";
            break;
        case BOOL:
        case STR: format += "String";
            break;
        default: Error("Erreur Display, type non pris en charge");
    }

#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    // --------- Microsoft x64 ABI (Windows / PE) -------------------
    cout << "\tpop     %rdx" << endl; // valeur à afficher
    if (t == BOOL)
    {
        setNextLbl();
        cout << "\tcmp $0,%rdx" << endl;
        cout << "\tjne True" << getTagNumber() << endl;
        cout << "\tleaq FalseString(%rip), %rcx" << endl;
        cout << "\tjmp " << getNextLbl() << endl;
        cout << "True" << getTagNumber() << ":" << endl;
        cout << "\tleaq TrueString(%rip), %rcx" << endl;
        cout << getNextLbl() << endl;
    }
    else
        cout << "\tleaq    " << format << "(%rip), %rcx" << endl; // format
    cout << "\tsub     $40, %rsp" << endl; // 32 shadow + 8 align
    cout << "\tmov     %rcx, (%rsp)" << endl; // home slots
    cout << "\tmov     %rdx, 8(%rsp)" << endl;
    cout << "\txor     %eax, %eax" << endl; // 0 registre XMM
    cout << "\tcall    printf" << endl;
    cout << "\tadd     $40, %rsp" << endl; // restaure RSP
#else
    // --------- SysV / ELF (Linux, BSD, …) ------------------------
    if (t == BOOL)
    {
        setNextLbl();
        cout << "\tpop %rax" << endl;
        cout << "\tcmp $0,%rax" << endl;
        cout << "\tjne True" << getTagNumber() << endl;
        cout << "\tleaq FalseString(%rip), %rsi" << endl;
        cout << "\tjmp " << getNextLbl() << endl;
        cout << "True" << getTagNumber() << ":" << endl;
        cout << "\tleaq TrueString(%rip), %rsi" << endl;
        cout << getNextLbl() << endl;
    }
    else
        cout << "\tpop     %rsi" << endl; // 2ᵉ argument
    cout << "\tleaq    " << format << "(%rip), %rdi" << endl; // 1er argument
    cout << "\txor     %eax, %eax" << endl; // 0 registre XMM
    cout << "\tcall    printf@PLT" << endl; // saut vers la PLT
#endif
}

unsigned long CaseLabelList()
{
    if (getCurrent() != NUMBER)
        Error("Nombre entier attendu");
    // nb de comp -> tag
    const unsigned long lbl = incrementTagNumber();
    cout << "\tcmp %rax, $" << getCurrentString() << endl;
    cout << "\tje Case" << lbl << endl;
    next();
    while (getCurrent() == COMMA)
    {
        next();
        if (getCurrent() != NUMBER)
            Error("Nombre entier attendu");
        cout << "\tcmp %rax, $" << getCurrentString() << endl;
        cout << "\tje Case" << lbl << endl;
        next();
    }
    return getTagNumber();
}

pair<unsigned long, string> CaseListElement()
{
    next();
    pair<unsigned long, string> caseElement;
    caseElement.first = CaseLabelList();
    if (getCurrent() != COLON)
        Error(": attendu");
    next();
    if (getCurrent() != SEMICOLON && getCurrentKeyword() != END)
    {
        next();
        caseElement.second = captureOutputOf([] { Statement(); });
    }
    else
    {
        caseElement.second = "\tcmp $" + getCurrentString() + ", %rax\n\tje " + getNextLbl() + "\n";
    }
    return caseElement;
}

void CaseStatement()
{
    setNextLbl();
    next();
    if (Expression() != INT)
        TypeError("variable non entière");
    if (getCurrentKeyword() != OF)
        Error("'OF' attendu");
    cout << "\tpop %rax" << endl;
    next();

    vector<pair<unsigned long, string>> cases;

    cases.push_back(CaseListElement());
    while (getCurrent() == SEMICOLON)
    {
        next();
        cases.push_back(CaseListElement());
        next();
    }
    if (getCurrentKeyword() != END)
        Error("'END' attendu");
    next();

    for (const auto& [fst, snd] : cases)
    {
        cout << "Case " << fst << ":" << endl;
        cout << snd << endl;
        cout << "\tjmp " << getNextLbl() << endl;
    }
    cout << getNextLbl() << endl;
}

void Statement()
{
    if (getCurrent() == KEYWORD)
    {
        switch (getCurrentKeyword())
        {
            case IF: IfStatement();
                return;
            case WHILE: WhileStatement();
                return;;
            case FOR: ForStatement();
                return;
            case BEGIN: BlockStatement();
                return;
            case DISPLAY: DisplayStatement();
                return;
            case CASE: CaseStatement();
                return;
            default: Error("Instruction non supportée pour ce mot-clé");
        }
    }
    AssignementStatement();
}

void StatementPart()
{
    cout << "\t.text\t\t# The following lines contain the program" << endl;
    cout << "ErrorArrayIndex:" << endl;
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    cout << "\tleaq  EAIMsg(%rip), %rcx" << endl;
    cout << "\txor   %eax, %eax" << endl;
    cout << "\tcall  printf" << endl;
#else
    cout <<"\tleaq EAIMsg(%rip), %rdi   # 1er arg = format"<< endl;
    cout <<"\txor  %eax, %eax"<< endl;
    cout <<"\tcall printf@PLT"<< endl;
#endif
    cout << "\tmov   $1, %edi # exit(1)" << endl;
    cout << "\tcall  exit" << endl;
    cout << "\t.globl main\t# The main function must be visible from outside" << endl;
    cout << "main:\t\t\t# The main function body :" << endl;
    cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top" << endl;
    Statement();
    while (getCurrent() == SEMICOLON)
    {
        next();
        Statement();
    }
    if (getCurrent() != DOT)
        Error("caractère '.' attendu");
    next();
}

void Program()
{
    if (getCurrentKeyword() == VAR)
        VarDeclarationPart();
    if (getCurrent() == KEYWORD && (getCurrentKeyword() == FUNCTION || getCurrentKeyword() == PROCEDURE))
        FunctionDeclarationPart();
    StatementPart();
}

int main()
{
    // First version: Source code on standard input and assembly code on standard output
    // Header for gcc assembler / linker
    cout << "\t\t\t# This code was produced by the CERI Compiler" << endl;
    // Let's proceed to the analysis and code production
    next();
    Program();
    // Trailer for the gcc assembler / linker
    cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top" << endl;
    cout << "\txor     %eax, %eax\t\t# 0 output code" << endl;
    cout << "\tret\t\t\t# Return from main function" << endl;
    if (getCurrent() != FEOF)
    {
        cerr << "Caractères en trop à la fin du programme : [" << getCurrent() << "]";
        Error("."); // unexpected characters at the end of the program
    }
}
