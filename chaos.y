%{

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utilities/platform.h"
#include "utilities/language.h"
#include "utilities/helpers.h"
#include "symbol.h"
#include "loop.h"
#include "function.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;

extern int yylineno;
extern char *yytext;
extern enum Phase { INIT_PREPARSE, PREPARSE, INIT_PROGRAM, PROGRAM };
extern enum Phase phase;

void yyerror(const char* s);

bool is_interactive = true;
bool inject_mode = false;
%}

%union {
    bool bval;
    int ival;
    float fval;
    char *sval;
}

%token START_PROGRAM START_PREPARSE
%token<bval> T_TRUE T_FALSE
%token<ival> T_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_VAR
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_EQUAL
%token T_LEFT_BRACKET T_RIGHT_BRACKET T_LEFT_CURLY_BRACKET T_RIGHT_CURLY_BRACKET T_COMMA T_COLON
%token T_NEWLINE T_QUIT
%token T_PRINT
%token T_VAR_BOOL T_VAR_NUMBER T_VAR_STRING T_VAR_ARRAY T_VAR_DICT
%token T_DEL T_RETURN T_VOID
%token T_SYMBOL_TABLE
%token T_TIMES_DO T_FOREACH T_AS T_END T_FUNCTION
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%type<ival> expression
%type<fval> mixed_expression
%type<sval> variable
%type<sval> arraystart
%type<ival> array

%start meta_start

%%

meta_start:
    | START_PROGRAM parser                                          { }
    | START_PREPARSE preparser                                      { }
;

preparser:
    | preparser preparser_line                                      { }
;

preparser_line: T_NEWLINE
    | function T_NEWLINE                                            { }
    | error T_NEWLINE                                               { yyerrok; }
;

function:
    | T_VAR_BOOL T_FUNCTION T_VAR function_parameters_start         { startFunction($3, BOOL); }
    | T_VAR_NUMBER T_FUNCTION T_VAR function_parameters_start       { startFunction($3, NUMBER); }
    | T_VAR_STRING T_FUNCTION T_VAR function_parameters_start       { startFunction($3, STRING); }
    | T_VAR_ARRAY T_FUNCTION T_VAR function_parameters_start        { startFunction($3, ARRAY); }
    | T_VAR_DICT T_FUNCTION T_VAR function_parameters_start         { startFunction($3, DICT); }
    | T_VOID T_FUNCTION T_VAR function_parameters_start             { startFunction($3, VOID); }
    | T_END                                                         { endLoop(); endFunction(); }
    | T_PRINT T_VAR T_LEFT function_call_parameters_start           { if (phase == PROGRAM) { callFunction($2); printFunctionReturn($2); } }
    | T_VAR T_LEFT function_call_parameters_start                   { if (phase == PROGRAM) callFunction($1); }
;

function_parameters_start:                                          { startFunctionParameters(); }
    | function_parameters_start T_LEFT function_parameters T_RIGHT  { }
;

function_call_parameters_start:                                     { }
    | function_parameters T_RIGHT                                   { }
;

function_parameters:                                                { }
    | T_NEWLINE function_parameters                                 { }
;

function_parameters: T_VAR_BOOL T_VAR                               { addFunctionParameter($2, BOOL); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_NUMBER T_VAR                             { addFunctionParameter($2, NUMBER); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_STRING T_VAR                             { addFunctionParameter($2, STRING); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_ARRAY T_VAR                              { addFunctionParameter($2, ARRAY); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_DICT T_VAR                               { addFunctionParameter($2, DICT); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_TRUE                                         { if (phase == PROGRAM) addFunctionCallParameterBool($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_FALSE                                        { if (phase == PROGRAM) addFunctionCallParameterBool($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_INT                                          { if (phase == PROGRAM) addFunctionCallParameterInt($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_FLOAT                                        { if (phase == PROGRAM) addFunctionCallParameterFloat($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_STRING                                       { if (phase == PROGRAM) addFunctionCallParameterString($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR                                          { if (phase == PROGRAM) addFunctionCallParameterSymbol($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

parser:
    | parser line                                                   {
        is_interactive ? (
            loop_mode || function_mode ? printf("%s ", __SHELL_INDICATOR_BLOCK__) : (
                inject_mode ? : printf("%s ", __SHELL_INDICATOR__)
            )
        ) : printf("");
    }
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE                                    { if (is_interactive) printf("%f\n", $1); }
    | expression T_NEWLINE                                          { if (is_interactive) printf("%i\n", $1); }
    | variable T_NEWLINE                                            { if ($1[0] != '\0' && is_interactive) printSymbolValueEndWithNewLine(getSymbol($1)); }
    | loop T_NEWLINE                                                { }
    | T_QUIT T_NEWLINE                                              { is_interactive ? printf("%s\n", __BYE_BYE__) : printf(""); exit(0); }
    | T_PRINT print T_NEWLINE                                       { }
    | T_SYMBOL_TABLE T_NEWLINE                                      { printSymbolTable(); }
    | function T_NEWLINE                                            { }
;

print: T_VAR T_LEFT_BRACKET T_INT T_RIGHT_BRACKET                   { printSymbolValueEndWithNewLine(getArrayElement($1, $3)); }
;
print: T_VAR T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET           { printSymbolValueEndWithNewLine(getArrayElement($1, -$4)); }
;
print: T_VAR T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET                { printSymbolValueEndWithNewLine(getDictElement($1, $3)); }
;
print: T_VAR                                                        { printSymbolValueEndWithNewLine(getSymbol($1)); }
;
print: T_INT                                                        { printf("%i\n", $1); }
;
print: T_FLOAT                                                      { printf("%f\n", $1); }
;
print: T_STRING                                                     { printf("%s\n", $1); }
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
    | T_VAR T_PLUS T_VAR                                            { $$ = getSymbolValueFloat($1) + getSymbolValueFloat($3); }
    | T_VAR T_MINUS T_VAR                                           { $$ = getSymbolValueFloat($1) + getSymbolValueFloat($3); }
    | T_VAR T_MULTIPLY T_VAR                                        { $$ = getSymbolValueFloat($1) + getSymbolValueFloat($3); }
    | T_VAR T_DIVIDE T_VAR                                          { $$ = getSymbolValueFloat($1) + getSymbolValueFloat($3); }
    | T_LEFT T_VAR T_RIGHT                                          { $$ = getSymbolValueFloat($2); }
    | mixed_expression T_PLUS T_VAR                                 { $$ = $1 + getSymbolValueFloat($3); }
    | mixed_expression T_MINUS T_VAR                                { $$ = $1 + getSymbolValueFloat($3); }
    | mixed_expression T_MULTIPLY T_VAR                             { $$ = $1 + getSymbolValueFloat($3); }
    | mixed_expression T_DIVIDE T_VAR                               { $$ = $1 + getSymbolValueFloat($3); }
    | expression T_PLUS T_VAR                                       { $$ = $1 + getSymbolValueFloat($3); }
    | expression T_MINUS T_VAR                                      { $$ = $1 + getSymbolValueFloat($3); }
    | expression T_MULTIPLY T_VAR                                   { $$ = $1 + getSymbolValueFloat($3); }
    | expression T_DIVIDE T_VAR                                     { $$ = $1 + getSymbolValueFloat($3); }
    | T_VAR T_PLUS mixed_expression                                 { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_MINUS mixed_expression                                { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_MULTIPLY mixed_expression                             { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_DIVIDE mixed_expression                               { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_PLUS expression                                       { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_MINUS expression                                      { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_MULTIPLY expression                                   { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_DIVIDE expression                                     { $$ = getSymbolValueFloat($1) + $3; }
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
    | variable T_EQUAL T_VAR                                        { updateSymbolByClonning($1, $3); $$ = ""; }
    | variable T_EQUAL mixed_expression                             { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL expression                                   { updateSymbolFloat($1, $3); $$ = ""; }
    | T_DEL variable                                                { removeSymbolByName($2); $$ = ""; }
    | T_DEL variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET           { removeComplexElement($2, $4, NULL); $$ = ""; }
    | T_DEL variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET   { removeComplexElement($2, -$5, NULL); $$ = ""; }
    | T_DEL variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET        { removeComplexElement($2, NULL, $4); $$ = ""; }
    | T_RETURN variable                                             { returnSymbol($2); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET                 { if ($1[0] != '\0' && is_interactive) printSymbolValueEndWithNewLine(getArrayElement($1, $3)); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET         { if ($1[0] != '\0' && is_interactive) printSymbolValueEndWithNewLine(getArrayElement($1, -$4)); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL T_TRUE              { updateComplexElementBool($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL T_TRUE      { updateComplexElementBool($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL T_FALSE             { updateComplexElementBool($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL T_FALSE     { updateComplexElementBool($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL T_INT               { updateComplexElementInt($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL T_INT       { updateComplexElementInt($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL T_FLOAT             { updateComplexElementFloat($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL T_FLOAT     { updateComplexElementFloat($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL T_STRING            { updateComplexElementString($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL T_STRING    { updateComplexElementString($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL T_VAR               { updateComplexElementSymbol($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL T_VAR       { updateComplexElementSymbol($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL mixed_expression            { updateComplexElementFloat($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_INT T_RIGHT_BRACKET T_EQUAL expression                  { updateComplexElementFloat($1, $3, NULL, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL mixed_expression    { updateComplexElementFloat($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET T_EQUAL expression          { updateComplexElementFloat($1, -$4, NULL, $7); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET              { if ($1[0] != '\0' && is_interactive) printSymbolValueEndWithNewLine(getDictElement($1, $3)); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL T_TRUE           { updateComplexElementBool($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL T_FALSE          { updateComplexElementBool($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL T_INT            { updateComplexElementInt($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL T_FLOAT          { updateComplexElementFloat($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL T_STRING         { updateComplexElementString($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL T_VAR            { updateComplexElementSymbol($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL mixed_expression         { updateComplexElementFloat($1, NULL, $3, $6); $$ = ""; }
    | variable T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET T_EQUAL expression               { updateComplexElementFloat($1, NULL, $3, $6); $$ = ""; }
;

variable: T_VAR_BOOL                                                { }
    | T_VAR_BOOL T_VAR T_EQUAL T_TRUE                               { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_FALSE                              { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, BOOL, $4, ANY); $$ = ""; }
    | T_VAR_BOOL T_VAR_ARRAY T_VAR T_EQUAL T_VAR                    { createCloneFromSymbolByName($3, ARRAY, $5, BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL T_VAR                     { createCloneFromSymbolByName($3, DICT, $5, BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_ARRAY T_VAR T_EQUAL arraystart               { finishComplexMode($3, BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL dictionarystart           { finishComplexMode($3, BOOL); $$ = ""; }
;

variable: T_VAR_NUMBER                                              { }
    | T_VAR_NUMBER T_VAR T_EQUAL T_INT                              { addSymbolInt($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_FLOAT                            { addSymbolFloat($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR                              { createCloneFromSymbolByName($2, NUMBER, $4, ANY); $$ = ""; }
    | T_VAR_NUMBER T_VAR_ARRAY T_VAR T_EQUAL T_VAR                  { createCloneFromSymbolByName($3, ARRAY, $5, NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, DICT, $5, NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_ARRAY T_VAR T_EQUAL arraystart             { finishComplexMode($3, NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL dictionarystart         { finishComplexMode($3, NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL mixed_expression                   { addSymbolFloat($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL expression                         { addSymbolFloat($2, $4); $$ = ""; }
;

variable: T_VAR_STRING                                              { }
    | T_VAR_STRING T_VAR T_EQUAL T_STRING                           { addSymbolString($2, $4); $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR                              { createCloneFromSymbolByName($2, STRING, $4, ANY); $$ = ""; }
    | T_VAR_STRING T_VAR_ARRAY T_VAR T_EQUAL T_VAR                  { createCloneFromSymbolByName($3, ARRAY, $5, STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, DICT, $5, STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_ARRAY T_VAR T_EQUAL arraystart             { finishComplexMode($3, STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL dictionarystart         { finishComplexMode($3, STRING); $$ = ""; }
;

variable: T_VAR_ARRAY                                               { }
    | T_VAR_ARRAY T_VAR T_EQUAL T_VAR                               { createCloneFromSymbolByName($2, ARRAY, $4, ANY); $$ = "";}
    | T_VAR_ARRAY T_VAR T_EQUAL arraystart                          { finishComplexMode($2, ANY); $$ = ""; }
;

arraystart:                                                         { addSymbolArray(NULL); }
    | arraystart T_LEFT_BRACKET array T_RIGHT_BRACKET               { }
;

array:                                                              { }
    | T_NEWLINE array                                               { }
;

array: T_TRUE                                                       { addSymbolBool(NULL, $1); }
    | array T_COMMA array                                           { }
    | array T_NEWLINE                                               { }
;
array: T_FALSE                                                      { addSymbolBool(NULL, $1); }
    | array T_COMMA array                                           { }
    | array T_NEWLINE                                               { }
;
array: T_INT                                                        { addSymbolFloat(NULL, $1); }
    | array T_COMMA array                                           { }
    | array T_NEWLINE                                               { }
;
array: T_FLOAT                                                      { addSymbolFloat(NULL, $1); }
    | array T_COMMA array                                           { }
    | array T_NEWLINE                                               { }
;
array: T_STRING                                                     { addSymbolString(NULL, $1); }
    | array T_COMMA array                                           { }
    | array T_NEWLINE                                               { }
;
array: T_VAR                                                        { cloneSymbolToComplex($1, NULL); }
    | array T_COMMA array                                           { }
    | array T_NEWLINE                                               { }
;

variable: T_VAR_DICT                                                { }
    | T_VAR_DICT T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, DICT, $4, ANY); $$ = "";}
    | T_VAR_DICT T_VAR T_EQUAL dictionarystart                      { finishComplexMode($2, ANY); $$ = ""; }
;

dictionarystart:                                                                { addSymbolDict(NULL); }
    | dictionarystart T_LEFT_CURLY_BRACKET dictionary T_RIGHT_CURLY_BRACKET     { }
;

dictionary:                                                         { }
    | T_NEWLINE dictionary                                          { }
;

dictionary: T_STRING T_COLON T_TRUE                                 { addSymbolBool($1, $3); }
    | dictionary T_COMMA dictionary                                 { }
    | dictionary T_NEWLINE                                          { }
;

dictionary: T_STRING T_COLON T_FALSE                                { addSymbolBool($1, $3); }
    | dictionary T_COMMA dictionary                                 { }
    | dictionary T_NEWLINE                                          { }
;

dictionary: T_STRING T_COLON T_INT                                  { addSymbolFloat($1, $3); }
    | dictionary T_COMMA dictionary                                 { }
    | dictionary T_NEWLINE                                          { }
;

dictionary: T_STRING T_COLON T_FLOAT                                { addSymbolFloat($1, $3); }
    | dictionary T_COMMA dictionary                                 { }
    | dictionary T_NEWLINE                                          { }
;

dictionary: T_STRING T_COLON T_STRING                               { addSymbolString($1, $3); }
    | dictionary T_COMMA dictionary                                 { }
    | dictionary T_NEWLINE                                          { }
;

dictionary: T_STRING T_COLON T_VAR                                  { cloneSymbolToComplex($3, $1); }
    | dictionary T_COMMA dictionary                                 { }
    | dictionary T_NEWLINE                                          { }
;

loop:
    | T_INT T_TIMES_DO                                              { startTimesDo($1); }
    | T_FOREACH T_VAR T_AS T_VAR                                    { startForeach($2, $4); }
    | T_FOREACH T_VAR T_AS T_VAR T_COLON T_VAR                      { startForeachDict($2, $4, $6); }
    | T_END                                                         { endLoop(); endFunction(); }
;

%%

int main(int argc, char** argv) {
    FILE *fp = argc > 1 ? fopen (argv[1], "r") : stdin;

    is_interactive = (fp != stdin) ? false : true;

    yyin = fp;

    if (is_interactive) {
        printf("%s Language %s (%s %s)\n", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
        printf("GCC version: %d.%d.%d on %s\n",__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);
        printf("%s\n\n", __LANGUAGE_MOTTO__);
        phase = INIT_PROGRAM;
    }

    do {
        !is_interactive ?: printf("%s ", __SHELL_INDICATOR__);
        yyparse();
    } while(!feof(yyin));

    return 0;
}

void yyerror(const char* s) {
    if (phase == PREPARSE) return;
    fprintf(stderr, "Parse error: %s\nLine: %i\nCause: %s\n", s, yylineno, yytext);
    exit(1);
}
