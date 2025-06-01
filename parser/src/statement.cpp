#include "statement.h"

#include <iostream>

#include "tokeniser.h"
#include "utils.h"
#include "proposition.h"
#include "function.h"
#include "type.h"

using namespace std;

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
            cout << "\tmov  %rbx, %rsi" << endl;
            cout << "\tmov  %rcx, %rdi" << endl;
            cout << "\tmov  $256, %rcx" << endl;
            cout << "\tcld" << endl;
            cout << "\trep movsb" << endl;
        }
        else
        {
            char suf;
            string reg;
            switch (size) {
                case 1:  suf = 'b'; reg = "%bl";   break;
                case 2:  suf = 'w'; reg = "%bx";   break;
                case 4:  suf = 'l'; reg = "%ebx";  break;
                default: suf = 'q'; reg = "%rbx";  break;
            }
            cout << "\tmov" << suf << " " << reg << ", (%rcx)\n";
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
void IfStatement() // NOLINT(*-no-recursion)
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

void WhileStatement() // NOLINT(*-no-recursion)
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

// ForStatement := "FOR" AssignementStatement ("TO" | "DOWNTO") Expression ["STEP" Number] "DO" Statement
void ForStatement() // NOLINT(*-no-recursion)
{
    const auto lbl = incrementTagNumber();
    next();
    if (getCurrent() != ID)
        Error("Identificateur attendu après FOR"); // recup loopvar qui sera consommé dans assign
    const string loopVar = getCurrentString();
    AssignementStatement();
    cout << "\tpushq " << loopVar << "(%rip)" << endl; // on remet le compteur sur la pile

    if (getCurrentKeyword() != TO && getCurrentKeyword() != DOWNTO)
        Error("'TO' attendu dans FOR");
    const bool down = (getCurrentKeyword() == DOWNTO);
    next();
    if (const TYPE lim = Expression(); lim != INT)
        TypeError("Expression non integer");

    // boucle
    cout << "For" << lbl << ":" << endl;
    cout << "\tpop %rbx    # borne sup" << endl;
    cout << "\tpop %rax    # compteur" << endl;
    cout << "\tcmp %rbx, %rax" << endl;
    cout << "\tj" << (down ? 'l' : 'g') << " EndFor" << lbl << endl;

    int incValue = 1;
    if (getCurrentKeyword() == STEP)
    {
        next();
        incValue = abs(stoi(getCurrentString()));
    }

    const string inc = "\t" + string(down ? "sub" : "add") + "q $" + to_string(incValue) + ", " + loopVar + "(%rip)\n";

    next();
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

void BlockStatement() // NOLINT(*-no-recursion)
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
        case BOOL: break;
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
        cout << getNextLbl() << ":" << endl;
    }
    else if (t == STR)
    {
        cout << "\tpop   %rsi" << endl;
        cout << "\tpush  %rsi" << endl;
        cout << "\tmov   %rsi, %rcx      # arg1 strlen (Windows ABI)" << endl;
        cout << "\tcall  strlen" << endl;
        cout << "\tmov   %rax, %rdx" << endl;
        cout << "\tpop   %rsi" << endl;
    }
    if (t != BOOL)
    {
        cout << "\tpop     %rdx" << endl; // valeur à afficher
        cout << "\tleaq    " << format << "(%rip), %rcx" << endl;
    } // format
    cout << "\tsub     $40, %rsp" << endl; // 32 shadow + 8 align
    cout << "\tmov     %rcx, (%rsp)" << endl; // home slots
    cout << "\tmov     %rdx, 8(%rsp)" << endl;
    cout << (t == STR ? "\tmov   %rsi, 16(%rsp)\n" : "");
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
        cout << getNextLbl() << ":" << endl;
    }
    else if (t == STR)
    {
        cout << "\tpop   %rsi" << endl;
        cout << "\tpush  %rsi" << endl;
        cout << "\tmov   %rsi, %rdi" << endl;
        cout << "\tcall  strlen@PLT" << endl;
        cout << "\tmov   %rax, %edx" << endl;
    }
    else
    {
        cout << "\tpop     %rsi" << endl; // 2ᵉ argument
        cout << "\tleaq    " << format << "(%rip), %rdi" << endl;
    } // 1er argument
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
    cout << "\tcmp $" << getCurrentString() << ", %rax" << endl;
    cout << "\tje Case" << lbl << endl;
    next();
    while (getCurrent() == COMMA)
    {
        next();
        if (getCurrent() != NUMBER)
            Error("Nombre entier attendu");
        cout << "\tcmp $" << getCurrentString() << ", %rax" << endl;
        cout << "\tje Case" << lbl << endl;
        next();
    }
    return getTagNumber();
}

pair<unsigned long, string> CaseListElement()
{
    pair<unsigned long, string> caseElement;
    caseElement.first = CaseLabelList();
    if (getCurrent() != COLON)
        Error(": attendu");
    next();
    if (getCurrent() != SEMICOLON && getCurrentKeyword() != END)
    {
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
    }
    if (getCurrentKeyword() != END)
        Error("'END' attendu");
    next();

    for (const auto& [fst, snd] : cases)
    {
        cout << "Case" << fst << ":" << endl;
        cout << snd << endl;
        cout << "\tjmp " << getNextLbl() << endl;
    }
    cout << getNextLbl() << ":" << endl;
}

void ReturnStatement()
{
    if (Functions.CurrentFunction == nullptr)
        Error("RETURN hors d'une fonction ou procédure");

    const FunctionInfo* info = Functions.CurrentFunction;
    next();
    // proc retour vide
    if (info->returnType == VOID)
    {
        if (getCurrent() != SEMICOLON    &&
            !(getCurrent()==KEYWORD && getCurrentKeyword()==END))
            Error("RETURN d'une procédure ne doit pas être suivi d'expression");

        cout << "\tmov %rbp, %rsp" << endl;
        cout << "\tpop %rbp"       << endl;
        cout << "\tret"            << endl;
        return;
    }

    // fonction retour obligatoire
    if (const TYPE expr = Expression(); expr != info->returnType)
        TypeError("Type de valeur retournée incompatible");

    cout << "\tpop %rax" << endl;
    cout << "\tmov %rbp, %rsp" << endl;
    cout << "\tpop %rbp"       << endl;
    cout << "\tret"            << endl;
}

void Statement() // NOLINT(*-no-recursion)
{
    if (getCurrent() == KEYWORD)
    {
        switch (getCurrentKeyword())
        {
            case IF: IfStatement();
                return;
            case WHILE: WhileStatement();
                return;
            case FOR: ForStatement();
                return;
            case BEGIN: BlockStatement();
                return;
            case DISPLAY: DisplayStatement();
                return;
            case CASE: CaseStatement();
                return;
            case RETURN: ReturnStatement();
                return;
            default: return;
        }
    }
    if (getCurrent() == ID)
    {
        if (const string var = getCurrentString(); Functions.FunctionList.contains(var))
        {
            if (Functions.FunctionList.at(var)->returnType != VOID) Error("Procédure attendue");
            next();
            if (getCurrent() != RPARENT) Error("'(' attendu");
            FuncProc(var, false);
            return;
        }
    }
    AssignementStatement();
}

void StatementPart()
{
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    cout << ".section .rdata" << endl;
#else
    cout << ".rodata" << endl;
#endif
    cout << "EAIMsg:\t.string \"Array index out of bounds\\n\"" << endl;
    cout << "\t.text\t\t# The following lines contain the program" << endl;
    cout << "\t.globl ErrorArrayIndex\t# The main function must be visible from outside" << endl;
    cout << "ErrorArrayIndex:" << endl;
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    cout << "\tleaq  EAIMsg(%rip), %rcx" << endl;
    cout << "\txor   %eax, %eax" << endl;
    cout << "\tcall  printf" << endl;
    cout << "\tmov   $1, %edi # exit(1)" << endl;
    cout << "\tcall  exit" << endl;
#else
    cout <<"\tleaq EAIMsg(%rip), %rdi   # 1er arg = format"<< endl;
    cout <<"\txor  %eax, %eax"<< endl;
    cout <<"\tcall printf@PLT"<< endl;
    cout << "\tmov   $1, %edi # exit(1)" << endl;
    cout << "\tcall  exit@PLT" << endl;
#endif
    cout << "\t.globl main\t# The main function must be visible from outside" << endl;
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__) || defined(__MINGW32__)
    cout << "\t.globl WinMain" << endl;
    cout << "\t.set WinMain, main" << endl;
#endif
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
