%{
// This is our Lexical tokeniser
// It should be compiled into cpp with :
// flex++ -d -otokeniser.cpp tokeniser.l
// And then compiled into object with
// g++ -c tokeniser.cpp
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

#include "tokeniser.h"
#include <iostream>

using namespace std;

%}

%option noyywrap
%option c++
%option yylineno

stringconst  \"[^\n"]*\"
charconst \'[^\n"]\'
ws      [ \t\n\r]+
alpha   [A-Za-z]
digit   [0-9]
number  {digit}+
float   {number}\.{number}
keyword	(DISPLAY|IF|THEN|ELSE|BEGIN|END|FOR|TO|DOWNTO|WHILE|DO|VAR|INTEGER|BOOLEAN|STEP|CHAR|DOUBLE|CASE|OF|STRING|ARRAY|FUNCTION|PROCEDURE|RETURN)
id	{alpha}({alpha}|{digit})*
addop	(\+|\-|\|\|)
mulop	(\*|\/|%|\&\&)
relop	(\<|\>|"=="|\<=|\>=|!=)
unknown [^\"A-Za-z0-9 \n\r\t\(\)\<\>\=\!\%\&\|\}\-\;\.]+

%%

{addop}		return ADDOP;
{mulop}		return MULOP;
{relop}		return RELOP;
{float}     return FLOAT;
{number}	return NUMBER;
{keyword}	return KEYWORD;
{id}		return ID;
{charconst} return CHARCONST;
{stringconst}	return STRINGCONST;
"["		return RBRACKET;
"]"		return LBRACKET;
","		return COMMA;
";"		return SEMICOLON;
"."		return DOT;
":"		return COLON;
":="	return ASSIGN;
"("		return RPARENT;
")"		return LPARENT;
"!"		return NOT;
<<EOF>>		return FEOF;
{ws}    {/* skip blanks and tabs */};
"(*"    { /* Skip comments between '(*' and '*)' */
		int c;
		while((c = yyinput()) != 0){
     			if(c == '*'){
     	    			if((c = yyinput()) == ')')
    	        			break;
    	     			else
  	          			unput(c);
  	   		}
		}
	};

{unknown}	return UNKNOWN;

%%
#include <string>

FlexLexer* lexer = new yyFlexLexer; // This is the flex tokenizer
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

TOKEN current;

TOKEN getCurrent() {
    return current;
}

string getCurrentString() {
    std::string str = lexer->YYText();
    std::cerr << "Read : " << str << std::endl;
    return str;
}

void next() {
    current=(TOKEN) lexer->yylex();
    std::cerr << "Current token: " << lexer->YYText() << std::endl;
}

int getLineNo() {
    return lexer->lineno();
}
