%{

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utilities/platform.h"
#include "utilities/language.h"
#include "utilities/helpers.h"
#include "symbol.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

bool is_interactive = true;
%}

%union {
    bool bval;
    int ival;
    float fval;
    char *sval;
}

%token<bval> T_TRUE T_FALSE
%token<ival> T_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_VAR
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_EQUAL T_LEFT_BRACKET T_RIGHT_BRACKET T_COMMA
%token T_NEWLINE T_QUIT
%token T_PRINT
%token T_VAR_BOOL T_VAR_NUMBER T_VAR_STRING T_VAR_ARRAY
%token T_DEL
%token T_SYMBOL_TABLE
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%type<ival> expression
%type<fval> mixed_expression
%type<sval> variable
%type<sval> arraystart
%type<ival> array

%start parser

%%

parser:
    | parser line                                                   { is_interactive ? printf("%s ", __SHELL_INDICATOR__) : printf(""); }
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE                                    { printf("%f\n", $1); }
    | expression T_NEWLINE                                          { printf("%i\n", $1); }
    | variable T_NEWLINE                                            { if ($1[0] != '\0' && is_interactive) printSymbolValue(getSymbol($1)); }
    | T_QUIT T_NEWLINE                                              { is_interactive ? printf("%s\n", __BYE_BYE__) : printf(""); exit(0); }
    | T_PRINT T_VAR T_NEWLINE                                       { printSymbolValue(getSymbol($2)); }
    | T_PRINT T_INT T_NEWLINE                                       { printf("%i\n", $2); }
    | T_PRINT T_STRING T_NEWLINE                                    { printf("%s\n", $2); }
    | T_SYMBOL_TABLE T_NEWLINE                                      { printSymbolTable(); }
;

mixed_expression: T_FLOAT                                           { $$ = $1; }
    | mixed_expression T_PLUS mixed_expression                      { $$ = $1 + $3; }
    | mixed_expression T_MINUS mixed_expression                     { $$ = $1 - $3; }
    | mixed_expression T_MULTIPLY mixed_expression                  { $$ = $1 * $3; }
    | mixed_expression T_DIVIDE mixed_expression                    { $$ = $1 / $3; }
    | T_LEFT mixed_expression T_RIGHT                               { $$ = $2; }
    | expression T_PLUS mixed_expression                            { $$ = $1 + $3; }
    | expression T_MINUS mixed_expression                           { $$ = $1 - $3; }
    | expression T_MULTIPLY mixed_expression                        { $$ = $1 * $3; }
    | expression T_DIVIDE mixed_expression                          { $$ = $1 / $3; }
    | mixed_expression T_PLUS expression                            { $$ = $1 + $3; }
    | mixed_expression T_MINUS expression                           { $$ = $1 - $3; }
    | mixed_expression T_MULTIPLY expression                        { $$ = $1 * $3; }
    | mixed_expression T_DIVIDE expression                          { $$ = $1 / $3; }
    | expression T_DIVIDE expression                                { $$ = $1 / (float)$3; }
;

expression: T_INT                                                   { $$ = $1; }
    | expression T_PLUS expression                                  { $$ = $1 + $3; }
    | expression T_MINUS expression                                 { $$ = $1 - $3; }
    | expression T_MULTIPLY expression                              { $$ = $1 * $3; }
    | T_LEFT expression T_RIGHT                                     { $$ = $2; }
;

variable: T_VAR                                                     { $$ = $1; }
    | variable T_EQUAL T_TRUE                                       { updateSymbolBool($1, $3); $$ = ""; }
    | variable T_EQUAL T_FALSE                                      { updateSymbolBool($1, $3); $$ = ""; }
    | variable T_EQUAL T_INT                                        { updateSymbolInt($1, $3); $$ = ""; }
    | variable T_EQUAL T_FLOAT                                      { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL T_STRING                                     { updateSymbolString($1, $3); $$ = ""; }
    | variable T_EQUAL T_LEFT_BRACKET array                         { updateSymbolArray($1); $$ = ""; }
    | T_DEL variable                                                { removeSymbol($2); $$ = ""; }
;

variable: T_VAR_BOOL                                                { }
    | T_VAR_BOOL T_VAR T_EQUAL T_TRUE                               { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_FALSE                              { addSymbolBool($2, $4); $$ = ""; }
;

variable: T_VAR_NUMBER                                              { }
    | T_VAR_NUMBER T_VAR T_EQUAL T_INT                              { addSymbolInt($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_FLOAT                            { addSymbolFloat($2, $4); $$ = ""; }
;

variable: T_VAR_STRING                                              { }
    | T_VAR_STRING T_VAR T_EQUAL T_STRING                           { addSymbolString($2, $4); $$ = ""; }
;

variable: T_VAR_ARRAY                                               { }
    | T_VAR_ARRAY T_VAR T_EQUAL arraystart                          { finishArrayMode($2); $$ = ""; }
;

arraystart: T_LEFT_BRACKET                                          { addSymbolArray(NULL); }
    | arraystart array                                              { $$ = ""; }
;

array: T_INT                                                        { addSymbolInt(NULL, $1); }
    | array T_COMMA array                                           { $$ = ""; }
;

array: T_RIGHT_BRACKET                                              { }
;

%%

int main(int argc, char** argv) {
    FILE *fp = argc > 1 ? fopen (argv[1], "r") : stdin;

    is_interactive = (fp != stdin) ? false : true;

    if (is_interactive) {
        printf("%s Language %s (%s %s)\n", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
        printf("GCC version: %d.%d.%d on %s\n",__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);
        printf("%s\n\n", __LANGUAGE_MOTTO__);
    }

    yyin = fp;

    do {
        !is_interactive ?: printf("%s ", __SHELL_INDICATOR__);
        yyparse();
    } while(!feof(yyin));

    return 0;
}

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
    exit(1);
}
