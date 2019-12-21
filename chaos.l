%option noyywrap

%{
#include <stdio.h>

#define YY_DECL int yylex()

#include "chaos.tab.h"

%}

%%

[ \t]	; // ignore all whitespace
[0-9]+\.[0-9]+ 	                {yylval.fval = atof(yytext); return T_FLOAT;}
[0-9]+		                    {yylval.ival = atoi(yytext); return T_INT;}
\n		                        {return T_NEWLINE;}
"+"		                        {return T_PLUS;}
"-"		                        {return T_MINUS;}
"*"		                        {return T_MULTIPLY;}
"/"		                        {return T_DIVIDE;}
"("		                        {return T_LEFT;}
")"		                        {return T_RIGHT;}
"exit"		                    {return T_QUIT;}
"quit"		                    {return T_QUIT;}
"print"                         {return T_PRINT;}

\"([^\\\"]|\\.)*\" {
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    return T_F_STRING;
}
\'([^\\\']|\\.)*\' {
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    return T_STRING;
}

%%
