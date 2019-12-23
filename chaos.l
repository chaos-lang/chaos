%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define YY_DECL int yylex()

#include "chaos.tab.h"

%}

%%

[ \t]	; // ignore all whitespace
[0-9]+\.[0-9]+                  {yylval.fval = atof(yytext); return T_FLOAT;}
[0-9]+                          {yylval.ival = atoi(yytext); return T_INT;}
\n                              {return T_NEWLINE;}
"="                             {return T_EQUAL;}
"+"                             {return T_PLUS;}
"-"                             {return T_MINUS;}
"*"                             {return T_MULTIPLY;}
"/"                             {return T_DIVIDE;}
"("                             {return T_LEFT;}
")"                             {return T_RIGHT;}
"exit"                          {return T_QUIT;}
"quit"                          {return T_QUIT;}
"print"                         {return T_PRINT;}
"true"                          {yylval.bval = 1; return T_TRUE;}
"false"                         {yylval.bval = 0; return T_FALSE;}
"symbol_table"                  {return T_SYMBOL_TABLE;}
"del"                           {return T_DEL;}

(\"|\')([^\\(\"|\')]|\\.)*(\"|\') {
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    return T_STRING;
}

bool|boolean                    {return T_VAR_BOOL;}
num|number                      {return T_VAR_NUMBER;}
str|string                      {return T_VAR_STRING;}
[a-zA-Z][a-zA-Z0-9]*            {yylval.sval=strdup(yytext); return T_VAR;}
%%
