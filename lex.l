%{
#include"parser.tab.h"
#include "tree.h"

extern union YYSTYPE yylval;

%}
%option noyywrap
%option yylineno

%x COMMENT

FLOAT [0-9]*[.]?[0-9]+([eE][+-]?[0-9]*|[0])?f?
INT [1-9][0-9]*|[0]
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
    #ifdef DEBUG
    printf("SEMI: %s\n", yytext);
    #endif
    return SEMI;
}

{TYPE} {
    yylval.a = newastTK(makeTextVal(yytext), TYPE);
    #ifdef DEBUG
    printf("TYPE: %s\n", yytext);
    #endif
    return TYPE;
}

{COMMA} {
    yylval.a = newastTK(makeTextVal(yytext), COMMA);
    #ifdef DEBUG
    printf("COMMA: %s\n", yytext);
    #endif
    return COMMA;
}

{ASSIGNOP} {
    yylval.a = newastTK(makeTextVal(yytext), ASSIGNOP);
    #ifdef DEBUG
    printf("ASSIGNOP: %s\n", yytext);
    #endif
    return ASSIGNOP;
}

{INT} {
    yylval.a = newastTK(makeIntVal(atoi(yytext)), INT);
    #ifdef DEBUG
    printf("INT: %s\n", yytext);
    #endif
    return INT;
}

{FLOAT} {
    yylval.a = newastTK(makeFloatVal(atof(yytext)), FLOAT);
    #ifdef DEBUG
    printf("FLOAT: %s\n", yytext);
    #endif
    return FLOAT;
}

{PLUS}	{
    yylval.a = newastTK(makeTextVal(yytext), PLUS);
    #ifdef DEBUG
    printf("PLUS: %s\n", yytext);
    #endif
    return PLUS;
}

{MINUS} {
    yylval.a = newastTK(makeTextVal(yytext), MINUS);
    #ifdef DEBUG
    printf("MINUS: %s\n", yytext);
    #endif
    return MINUS;
}

{DIV} {
    yylval.a = newastTK(makeTextVal(yytext), DIV);
    #ifdef DEBUG
    printf("DIV: %s\n", yytext);
    #endif
    return DIV;
}

{STAR} {
    yylval.a = newastTK(makeTextVal(yytext), STAR);
    #ifdef DEBUG
    printf("STAR: %s\n", yytext);
    #endif
    return STAR;
}

{RELOP} {
    yylval.a = newastTK(makeTextVal(yytext), RELOP);
    #ifdef DEBUG
    printf("RELOP: %s\n", yytext);
    #endif
    return RELOP;
}

{AND} {
    yylval.a = newastTK(makeTextVal(yytext), AND);
    #ifdef DEBUG
    printf("AND: %s\n", yytext);
    #endif
    return AND;
}

{OR} {
    yylval.a = newastTK(makeTextVal(yytext), OR);
    #ifdef DEBUG
    printf("OR: %s\n", yytext);
    #endif
    return OR;
}

{NOT} {
    yylval.a = newastTK(makeTextVal(yytext), NOT);
    #ifdef DEBUG
    printf("NOT: %s\n", yytext);
    #endif
    return NOT;
}

{DOT} {
    yylval.a = newastTK(makeTextVal(yytext), DOT);
    #ifdef DEBUG
    printf("DOT: %s\n", yytext);
    #endif
    return DOT;
}

{STRUCT} {
    yylval.a = newastTK(makeTextVal(yytext), STRUCT);
    #ifdef DEBUG
    printf("STRUCT: %s\n", yytext);
    #endif
    return STRUCT;
}

{RETURN} {
    yylval.a = newastTK(makeTextVal(yytext), RETURN);
    #ifdef DEBUG
    printf("RETURN: %s\n", yytext);
    #endif
    return RETURN;
}

{IF}  {
    yylval.a = newastTK(makeTextVal(yytext), IF);
    #ifdef DEBUG
    printf("IF: %s\n", yytext);
    #endif
    return IF;
}

{ELSE}	{
    yylval.a = newastTK(makeTextVal(yytext), ELSE);
    #ifdef DEBUG
    printf("ELSE: %s\n", yytext);
    #endif
    return ELSE;
}
 
{WHILE} {
    yylval.a = newastTK(makeTextVal(yytext), WHILE);
    #ifdef DEBUG
    printf("WHILE: %s\n", yytext);
    #endif
    return WHILE;
}

{LP} {
    yylval.a = newastTK(makeTextVal(yytext), LP);
    #ifdef DEBUG
    printf("LP: %s\n", yytext);
    #endif
    return LP;
}

{RP} {
    yylval.a = newastTK(makeTextVal(yytext), RP);
    #ifdef DEBUG
    printf("RP: %s\n", yytext);
    #endif
    return RP;
}

{LB} {
    yylval.a = newastTK(makeTextVal(yytext), LB);
    #ifdef DEBUG
    printf("LB: %s\n", yytext);
    #endif
    return LB;
}

{RB} {
    yylval.a = newastTK(makeTextVal(yytext), RB);
    #ifdef DEBUG
    printf("RB: %s\n", yytext);
    #endif
    return RB;
}

{LC} {
    yylval.a = newastTK(makeTextVal(yytext), LC);
    #ifdef DEBUG
    printf("LC: %s\n", yytext);
    #endif
    return LC;
}

{RC} {
    yylval.a = newastTK(makeTextVal(yytext), RC);
    #ifdef DEBUG
    printf("RC: %s\n", yytext);
    #endif
    return RC;
}

{ID} {
    yylval.a = newastTK(makeTextVal(yytext), ID);
    #ifdef DEBUG
    printf("ID: %s\n", yytext);
    #endif
    return ID;
}

{SPACE} {
/* ABANDON THESE CHARACTERS */
}

. {
    fprintf(stderr, "Error type A at line %d: Mysterious character \"%s\".\n", yylineno, yytext);
}
%%
