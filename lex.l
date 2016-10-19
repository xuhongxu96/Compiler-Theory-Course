%{
#include"parser.tab.h"
#include "common.h"

extern union YYSTYPE yylval;

%}
%option noyywrap
%option yylineno

%x COMMENT

INT [1-9][0-9]*|[0]
FLOAT [0-9]*[.][0-9]+([eE][+-]?[0-9]*|[0])?f?
TYPE (int)|(float)
ID (_|[A-Za-z])([A-Za-z]|[0-9]|_)*
SEMI [;]
COMMA [,]
ASSIGNOP [=]
RELOP [>]|[<]|[>][=]|[<][=]|[=][=]|[!][=](^[=])
PLUS [+]
MINUS [-]
STAR [*]
DIV [/]
AND [&][&]
OR [|][|]
DOT [.]
NOT [!]
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else 
WHILE while
SPACE [ \n\t]

%%
 /* comments */
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>> {
    fprintf(stderr, "Error type A at line %d: Untermintated comment.\n", yylineno);
}
"//".*\n { }

{SEMI} {
    yylval.a = newastTK(makeTextVal(yytext), SEMI);
    return SEMI;
}

{TYPE} {
    yylval.a = newastTK(makeTextVal(yytext), TYPE);
    return TYPE;
}

{COMMA} {
    yylval.a = newastTK(makeTextVal(yytext), COMMA);
    return COMMA;
}

{ASSIGNOP} {
    yylval.a = newastTK(makeTextVal(yytext), ASSIGNOP);
    return ASSIGNOP;
}

{INT} {
    yylval.a = newastTK(makeIntVal(atoi(yytext)), INT);
    return INT;
}

{FLOAT} {
    yylval.a = newastTK(makeFloatVal(atof(yytext)), FLOAT);
    return FLOAT;
}

{PLUS}	{
    yylval.a = newastTK(makeTextVal(yytext), PLUS);
    return PLUS;
}

{MINUS} {
    yylval.a = newastTK(makeTextVal(yytext), MINUS);
    return MINUS;
}

{DIV} {
    yylval.a = newastTK(makeTextVal(yytext), DIV);
    return DIV;
}

{STAR} {
    yylval.a = newastTK(makeTextVal(yytext), STAR);
    return STAR;
}

{RELOP} {
    yylval.a = newastTK(makeTextVal(yytext), RELOP);
    return RELOP;
}

{AND} {
    yylval.a = newastTK(makeTextVal(yytext), AND);
    return AND;
}

{OR} {
    yylval.a = newastTK(makeTextVal(yytext), OR);
    return OR;
}

{NOT} {
    yylval.a = newastTK(makeTextVal(yytext), NOT);
    return NOT;
}

{DOT} {
    yylval.a = newastTK(makeTextVal(yytext), DOT);
    return DOT;
}

{STRUCT} {
    yylval.a = newastTK(makeTextVal(yytext), STRUCT);
    return STRUCT;
}

{RETURN} {
    yylval.a = newastTK(makeTextVal(yytext), RETURN);
    return RETURN;
}

{IF}  {
    yylval.a = newastTK(makeTextVal(yytext), IF);
    return IF;
}

{ELSE}	{
    yylval.a = newastTK(makeTextVal(yytext), ELSE);
    return ELSE;
}
 
{WHILE} {
    yylval.a = newastTK(makeTextVal(yytext), WHILE);
    return WHILE;
}

{LP} {
    yylval.a = newastTK(makeTextVal(yytext), LP);
    return LP;
}

{RP} {
    yylval.a = newastTK(makeTextVal(yytext), RP);
    return RP;
}

{LB} {
    yylval.a = newastTK(makeTextVal(yytext), LB);
    return LB;
}

{RB} {
    yylval.a = newastTK(makeTextVal(yytext), RB);
    return RB;
}

{LC} {
    yylval.a = newastTK(makeTextVal(yytext), LC);
    return LC;
}

{RC} {
    yylval.a = newastTK(makeTextVal(yytext), RC);
    return RC;
}

{ID} {
    yylval.a = newastTK(makeTextVal(yytext), ID);
    return ID;
}

{SPACE} {
/* ABANDON THESE CHARACTERS */
}

. {
    fprintf(stderr, "Error type A at line %d: Mysterious character \"%s\".\n", yylineno, yytext);
}
%%
