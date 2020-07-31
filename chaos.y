%{

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/syslimits.h>
#elif !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include <linux/limits.h>
#endif

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include <readline/history.h>
#endif

#include "utilities/platform.h"
#include "utilities/language.h"
#include "utilities/helpers.h"
#include "utilities/injector.h"
#include "symbol.h"
#include "loop.h"
#include "function.h"
#include "module.h"

extern int yylex();
extern int yyparse();
extern int yylex_destroy();
extern FILE* yyin;

extern int yylineno;
extern char *yytext;
extern enum Phase phase;

void yyerror(const char* s);
void freeEverything();

bool is_interactive = true;
bool inject_mode = false;

jmp_buf InteractiveShellErrorAbsorber;

FILE *fp;
bool fp_opened;
char *program_file_path;
char *program_file_dir;
char *program_code;
%}

%union {
    bool bval;
    long long ival;
    long double fval;
    char *sval;
    unsigned long long lluval;
}

%token START_PROGRAM START_PREPARSE START_JSON_PARSE
%token<bval> T_TRUE T_FALSE
%token<ival> T_INT T_TIMES_DO_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_VAR T_FOREACH_VAR
%token<lluval> T_UNSIGNED_LONG_LONG_INT
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_EQUAL
%token T_LEFT_BRACKET T_RIGHT_BRACKET T_LEFT_CURLY_BRACKET T_RIGHT_CURLY_BRACKET T_COMMA T_DOT T_COLON
%token T_NEWLINE T_QUIT
%token T_PRINT T_ECHO T_PRETTY
%token T_VAR_BOOL T_VAR_NUMBER T_VAR_STRING T_VAR_LIST T_VAR_DICT T_VAR_ANY T_NULL
%token T_DEL T_RETURN T_VOID T_DEFAULT
%token T_SYMBOL_TABLE T_FUNCTION_TABLE
%token T_TIMES_DO T_FOREACH T_AS T_END T_FUNCTION T_IMPORT T_FROM T_BACKSLASH T_INFINITE T_FOREACH_AS_COLON
%token T_REL_EQUAL T_REL_NOT_EQUAL T_REL_GREAT T_REL_SMALL T_REL_GREAT_EQUAL T_REL_SMALL_EQUAL
%token T_LOGIC_AND T_LOGIC_OR T_LOGIC_NOT
%token T_BITWISE_AND T_BITWISE_OR T_BITWISE_XOR T_BITWISE_NOT T_BITWISE_LEFT_SHIFT T_BITWISE_RIGHT_SHIFT
%token T_INCREMENT T_DECREMENT
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%type<ival> expression
%type<fval> mixed_expression
%type<bval> boolean_expression
%type<sval> variable
%type<ival> list
%type<lluval> left_right_bracket

%destructor {
    free($$);
} <sval>

%start meta_start

%%

meta_start:
    | START_PROGRAM parser                                          { }
    | START_PREPARSE preparser                                      { }
    | START_JSON_PARSE json_parser                                  { }
;

preparser:
    | preparser preparser_line                                      { }
;

preparser_line: T_NEWLINE
    | function T_NEWLINE                                            { }
    | T_IMPORT module T_NEWLINE                                     { handleModuleImport(NULL, false); }
    | T_IMPORT module T_AS T_VAR T_NEWLINE                          { handleModuleImport($4, false); }
    | T_FROM module T_IMPORT T_MULTIPLY                             { handleModuleImport(NULL, true); }
    | T_FROM module T_IMPORT function_name                          { handleModuleImport(NULL, true); }
    | T_END decisionstart                                           { }
    | error T_NEWLINE                                               { yyerrok; }
;

function:
    | T_VAR_BOOL T_FUNCTION T_VAR function_parameters_start         { startFunction($3, K_BOOL, K_ANY); }
    | T_VAR_NUMBER T_FUNCTION T_VAR function_parameters_start       { startFunction($3, K_NUMBER, K_ANY); }
    | T_VAR_STRING T_FUNCTION T_VAR function_parameters_start       { startFunction($3, K_STRING, K_ANY); }
    | T_VAR_ANY T_FUNCTION T_VAR function_parameters_start          { startFunction($3, K_ANY, K_ANY); }
    | T_VAR_LIST T_FUNCTION T_VAR function_parameters_start         { startFunction($3, K_LIST, K_ANY); }
    | T_VAR_DICT T_FUNCTION T_VAR function_parameters_start         { startFunction($3, K_DICT, K_ANY); }
    | T_VAR_BOOL T_VAR_LIST T_FUNCTION T_VAR function_parameters_start         { startFunction($4, K_LIST, K_BOOL); }
    | T_VAR_BOOL T_VAR_DICT T_FUNCTION T_VAR function_parameters_start         { startFunction($4, K_DICT, K_BOOL); }
    | T_VAR_NUMBER T_VAR_LIST T_FUNCTION T_VAR function_parameters_start       { startFunction($4, K_LIST, K_NUMBER); }
    | T_VAR_NUMBER T_VAR_DICT T_FUNCTION T_VAR function_parameters_start       { startFunction($4, K_DICT, K_NUMBER); }
    | T_VAR_STRING T_VAR_LIST T_FUNCTION T_VAR function_parameters_start       { startFunction($4, K_LIST, K_STRING); }
    | T_VAR_STRING T_VAR_DICT T_FUNCTION T_VAR function_parameters_start       { startFunction($4, K_DICT, K_STRING); }
    | T_VOID T_FUNCTION T_VAR function_parameters_start             { startFunction($3, K_VOID, K_ANY); }
    | T_PRINT T_VAR T_LEFT function_call_parameters_start           { if (phase == PROGRAM) { callFunction($2, NULL); printFunctionReturn($2, NULL, "\n", false, true); } free($2); }
    | T_ECHO T_VAR T_LEFT function_call_parameters_start            { if (phase == PROGRAM) { callFunction($2, NULL); printFunctionReturn($2, NULL, "", false, true); } free($2); }
    | T_PRETTY T_PRINT T_VAR T_LEFT function_call_parameters_start          { if (phase == PROGRAM) { callFunction($3, NULL); printFunctionReturn($3, NULL, "\n", true, true); } free($3); }
    | T_PRETTY T_ECHO T_VAR T_LEFT function_call_parameters_start           { if (phase == PROGRAM) { callFunction($3, NULL); printFunctionReturn($3, NULL, "", true, true); } free($3); }
    | T_VAR T_LEFT function_call_parameters_start                   { if (phase == PROGRAM) { callFunction($1, NULL); if (is_interactive && !isFunctionType($1, NULL, K_VOID) && !inject_mode && !decision_execution_mode) printFunctionReturn($1, NULL, "\n", false, false); } free($1); }
    | T_PRINT T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start       { if (phase == PROGRAM) { callFunction($4, $2); printFunctionReturn($4, $2, "\n", false, true); } free($4); free($2); }
    | T_ECHO T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($4, $2); printFunctionReturn($4, $2, "", false, true); } free($4); free($2); }
    | T_PRETTY T_PRINT T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start      { if (phase == PROGRAM) { callFunction($5, $3); printFunctionReturn($5, $3, "\n", true, true); } free($5); free($3); }
    | T_PRETTY T_ECHO T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start       { if (phase == PROGRAM) { callFunction($5, $3); printFunctionReturn($5, $3, "", true, true); } free($5); free($3); }
    | T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start               { if (phase == PROGRAM) { callFunction($3, $1); if (is_interactive && !isFunctionType($3, $1, K_VOID) && !inject_mode && !decision_execution_mode) printFunctionReturn($3, $1, "\n", false, false); } free($3); free($1); }
    | error T_NEWLINE                                               { if (is_interactive) { yyerrok; yyclearin; } }
    | T_FOREACH_AS_COLON function                                   { }
    | T_TIMES_DO_INT function                                       { }
    | T_FOREACH_VAR function                                        { free($1); }
;

function_parameters_start:                                          { startFunctionParameters(); }
    | function_parameters_start T_LEFT function_parameters T_RIGHT  { }
;

function_call_parameters_start:                                     { }
    | function_parameters T_RIGHT                                   { }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;

function_parameters:                                                { }
    | T_NEWLINE function_parameters                                 { }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;
function_parameters: T_VAR_BOOL T_VAR                               { addFunctionParameter($2, K_BOOL, K_ANY); }
;
function_parameters: T_VAR_NUMBER T_VAR                             { addFunctionParameter($2, K_NUMBER, K_ANY); }
;
function_parameters: T_VAR_STRING T_VAR                             { addFunctionParameter($2, K_STRING, K_ANY); }
;
function_parameters: T_VAR_LIST T_VAR                               { addFunctionParameter($2, K_LIST, K_ANY); }
;
function_parameters: T_VAR_BOOL T_VAR_LIST T_VAR                    { addFunctionParameter($3, K_LIST, K_BOOL); }
;
function_parameters: T_VAR_NUMBER T_VAR_LIST T_VAR                  { addFunctionParameter($3, K_LIST, K_NUMBER); }
;
function_parameters: T_VAR_STRING T_VAR_LIST T_VAR                  { addFunctionParameter($3, K_LIST, K_STRING); }
;
function_parameters: T_VAR_DICT T_VAR                               { addFunctionParameter($2, K_DICT, K_ANY); }
;
function_parameters: T_VAR_BOOL T_VAR_DICT T_VAR                    { addFunctionParameter($3, K_DICT, K_BOOL); }
;
function_parameters: T_VAR_NUMBER T_VAR_DICT T_VAR                  { addFunctionParameter($3, K_DICT, K_NUMBER); }
;
function_parameters: T_VAR_STRING T_VAR_DICT T_VAR                  { addFunctionParameter($3, K_DICT, K_STRING); }
;
function_parameters: T_VAR_BOOL T_VAR T_EQUAL boolean_expression        { addFunctionOptionalParameterBool($2, $4); }
;
function_parameters: T_VAR_NUMBER T_VAR T_EQUAL mixed_expression        { addFunctionOptionalParameterFloat($2, $4); }
;
function_parameters: T_VAR_NUMBER T_VAR T_EQUAL expression              { addFunctionOptionalParameterFloat($2, $4); }
;
function_parameters: T_VAR_STRING T_VAR T_EQUAL T_STRING                { addFunctionOptionalParameterString($2, $4); }
;
function_parameters: T_VAR_LIST T_VAR T_EQUAL liststart                 { addFunctionOptionalParameterComplex($2, K_ANY); }
;
function_parameters: T_VAR_BOOL T_VAR_LIST T_VAR T_EQUAL liststart      { addFunctionOptionalParameterComplex($3, K_BOOL); }
;
function_parameters: T_VAR_NUMBER T_VAR_LIST T_VAR T_EQUAL liststart    { addFunctionOptionalParameterComplex($3, K_NUMBER); }
;
function_parameters: T_VAR_STRING T_VAR_LIST T_VAR T_EQUAL liststart    { addFunctionOptionalParameterComplex($3, K_STRING); }
;
function_parameters: T_VAR_DICT T_VAR T_EQUAL dictionarystart           { addFunctionOptionalParameterComplex($2, K_ANY); }
;
function_parameters: T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL dictionarystart        { addFunctionOptionalParameterComplex($3, K_BOOL); }
;
function_parameters: T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL dictionarystart      { addFunctionOptionalParameterComplex($3, K_NUMBER); }
;
function_parameters: T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL dictionarystart      { addFunctionOptionalParameterComplex($3, K_STRING); }
;
function_parameters: T_TRUE                                         { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterBool($1); }
;
function_parameters: T_FALSE                                        { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterBool($1); }
;
function_parameters: expression                                     { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterInt($1); }
;
function_parameters: mixed_expression                               { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterFloat($1); }
;
function_parameters: T_STRING                                       { if (!block(B_FUNCTION) && phase == PROGRAM) { addFunctionCallParameterString($1); } free($1); }
;
function_parameters: T_VAR                                          { if (!block(B_FUNCTION) && phase == PROGRAM) { addFunctionCallParameterSymbol($1); } free($1); }
;
function_parameters: liststart                                      { if (!block(B_FUNCTION) && phase == PROGRAM) { addFunctionCallParameterList(K_ANY); } else { finishComplexMode(NULL, K_ANY); } }
;
function_parameters: dictionarystart                                { if (!block(B_FUNCTION) && phase == PROGRAM) { addFunctionCallParameterList(K_ANY); } else { finishComplexMode(NULL, K_ANY); } }
;

parser:
    | parser line                                                   {
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        is_interactive ? (
            loop_mode || function_mode || isComplexMode() || decision_mode ? printf("%s ", __KAOS_SHELL_INDICATOR_BLOCK__) : (
                inject_mode ? : printf("%s ", __KAOS_SHELL_INDICATOR__)
            )
        ) : printf("");
        #endif
    }
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE                                    { if (is_interactive && isStreamOpen() && !inject_mode) printf("%Lg\n", $1); }
    | expression T_NEWLINE                                          { if (is_interactive && isStreamOpen() && !inject_mode) printf("%lld\n", $1); }
    | variable T_NEWLINE                                            { if ($1[0] != '\0' && is_interactive && !inject_mode) { printSymbolValueEndWithNewLine(getSymbol($1), false, false); free($1); } }
    | expression T_TIMES_DO                                         { startTimesDo($1, false); }
    | T_TIMES_DO_INT T_TIMES_DO                                     { startTimesDo($1, false); }
    | T_INFINITE T_TIMES_DO                                         { startTimesDo(0, true); }
    | T_VAR T_TIMES_DO                                              { startTimesDo((unsigned) getSymbolValueInt($1), false); }
    | T_FOREACH T_VAR T_AS T_VAR                                    { startForeach($2, $4); }
    | T_FOREACH T_VAR T_AS T_VAR T_COLON T_VAR                      { startForeachDict($2, $4, $6); }
    | T_FOREACH T_VAR T_AS T_VAR T_FOREACH_AS_COLON T_VAR           { startForeachDict($2, $4, $6); }
    | T_FOREACH T_FOREACH_VAR T_AS T_FOREACH_VAR                                            { startForeach($2, $4); }
    | T_FOREACH T_FOREACH_VAR T_AS T_FOREACH_VAR T_COLON T_FOREACH_VAR                      { startForeachDict($2, $4, $6); }
    | T_FOREACH T_FOREACH_VAR T_AS T_FOREACH_VAR T_FOREACH_AS_COLON T_FOREACH_VAR           { startForeachDict($2, $4, $6); }
    | T_QUIT quit                                                   { }
    | T_PRINT print T_NEWLINE                                       { }
    | T_ECHO echo T_NEWLINE                                         { }
    | T_PRETTY T_PRINT pretty_print T_NEWLINE                       { }
    | T_PRETTY T_ECHO pretty_echo T_NEWLINE                         { }
    | T_SYMBOL_TABLE T_NEWLINE                                      { printSymbolTable(); }
    | T_DEL T_VAR T_NEWLINE                                         { removeSymbolByName($2); free($2); }
    | T_DEL T_VAR left_right_bracket T_NEWLINE                      { removeComplexElementByLeftRightBracketStack($2); }
    | T_FUNCTION_TABLE T_NEWLINE                                    { printFunctionTable(); }
    | function T_NEWLINE                                            { }
    | T_END decisionstart                                           { }
    | T_IMPORT module T_NEWLINE                                     { if (is_interactive) handleModuleImport(NULL, false); }
    | T_IMPORT module T_AS T_VAR T_NEWLINE                          { if (is_interactive) { handleModuleImport($4, false); } else { free($4); } }
    | T_FROM module T_IMPORT T_MULTIPLY                             { if (is_interactive) handleModuleImport(NULL, true); }
    | T_FROM module T_IMPORT function_name                          { if (is_interactive) handleModuleImport(NULL, true); }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;

print: T_VAR left_right_bracket                                     { printSymbolValueEndWithNewLine(getComplexElementThroughLeftRightBracketStack($1, 0), false, true); }
;
print: T_VAR                                                        { printSymbolValueEndWithNewLine(getSymbol($1), false, true); free($1); }
;
print: expression                                                   { printf("%lld\n", $1); }
;
print: mixed_expression                                             { printf("%Lg\n", $1); }
;
print: T_STRING                                                     { printf("%s\n", $1); free($1); }
;

echo: T_VAR left_right_bracket                                      { printSymbolValueEndWith(getComplexElementThroughLeftRightBracketStack($1, 0), "", false, true); }
;
echo: T_VAR                                                         { printSymbolValueEndWith(getSymbol($1), "", false, true); free($1); }
;
echo: expression                                                    { printf("%lld", $1); }
;
echo: mixed_expression                                              { printf("%Lg", $1); }
;
echo: T_STRING                                                      { printf("%s", $1); free($1); }
;

pretty_print: T_VAR left_right_bracket                              { printSymbolValueEndWithNewLine(getComplexElementThroughLeftRightBracketStack($1, 0), true, true); }
;
pretty_print: T_VAR                                                 { printSymbolValueEndWithNewLine(getSymbol($1), true, true); free($1); }
;
pretty_echo: T_VAR left_right_bracket                               { printSymbolValueEndWith(getComplexElementThroughLeftRightBracketStack($1, 0), "", true, true); }
;
pretty_echo: T_VAR                                                  { printSymbolValueEndWith(getSymbol($1), "", true, true); free($1); }
;

mixed_expression: T_FLOAT                                           { $$ = (long double) $1; }
    | T_MINUS mixed_expression                                      { $$ = - $2; }
    | mixed_expression T_PLUS mixed_expression                      { $$ = $1 + $3; }
    | mixed_expression T_MINUS mixed_expression                     { $$ = $1 - $3; }
    | mixed_expression T_MULTIPLY mixed_expression                  { $$ = $1 * $3; }
    | mixed_expression T_DIVIDE mixed_expression                    { $$ = $1 / $3; }
    | expression T_PLUS mixed_expression                            { $$ = $1 + $3; }
    | expression T_MINUS mixed_expression                           { $$ = $1 - $3; }
    | expression T_MULTIPLY mixed_expression                        { $$ = $1 * $3; }
    | expression T_DIVIDE mixed_expression                          { $$ = $1 / $3; }
    | mixed_expression T_PLUS expression                            { $$ = $1 + $3; }
    | mixed_expression T_MINUS expression                           { $$ = $1 - $3; }
    | mixed_expression T_MULTIPLY expression                        { $$ = $1 * $3; }
    | mixed_expression T_DIVIDE expression                          { $$ = $1 / $3; }
    | expression T_DIVIDE expression                                { $$ = $1 / (long double)$3; }
    | T_VAR T_PLUS T_VAR                                            { $$ = getSymbolValueFloat($1) + getSymbolValueFloat($3); }
    | T_VAR T_MINUS T_VAR                                           { $$ = getSymbolValueFloat($1) - getSymbolValueFloat($3); }
    | T_VAR T_MULTIPLY T_VAR                                        { $$ = getSymbolValueFloat($1) * getSymbolValueFloat($3); }
    | T_VAR T_DIVIDE T_VAR                                          { $$ = getSymbolValueFloat($1) / getSymbolValueFloat($3); }
    | T_LEFT T_VAR T_RIGHT                                          { $$ = getSymbolValueFloat($2); }
    | mixed_expression T_PLUS T_VAR                                 { $$ = $1 + getSymbolValueFloat($3); }
    | mixed_expression T_MINUS T_VAR                                { $$ = $1 - getSymbolValueFloat($3); }
    | mixed_expression T_MULTIPLY T_VAR                             { $$ = $1 * getSymbolValueFloat($3); }
    | mixed_expression T_DIVIDE T_VAR                               { $$ = $1 / getSymbolValueFloat($3); }
    | expression T_PLUS T_VAR                                       { $$ = $1 + getSymbolValueFloat($3); }
    | expression T_MINUS T_VAR                                      { $$ = $1 - getSymbolValueFloat($3); }
    | expression T_MULTIPLY T_VAR                                   { $$ = $1 * getSymbolValueFloat($3); }
    | expression T_DIVIDE T_VAR                                     { $$ = $1 / getSymbolValueFloat($3); }
    | T_VAR T_PLUS mixed_expression                                 { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_MINUS mixed_expression                                { $$ = getSymbolValueFloat($1) - $3; }
    | T_VAR T_MULTIPLY mixed_expression                             { $$ = getSymbolValueFloat($1) * $3; }
    | T_VAR T_DIVIDE mixed_expression                               { $$ = getSymbolValueFloat($1) / $3; }
    | T_VAR T_PLUS expression                                       { $$ = getSymbolValueFloat($1) + $3; }
    | T_VAR T_MINUS expression                                      { $$ = getSymbolValueFloat($1) - $3; }
    | T_VAR T_MULTIPLY expression                                   { $$ = getSymbolValueFloat($1) * $3; }
    | T_VAR T_DIVIDE expression                                     { $$ = getSymbolValueFloat($1) / $3; }
    | T_LEFT mixed_expression T_RIGHT                               { $$ = $2; }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;

expression: T_INT                                                   { $$ = (long long) $1; }
    | T_MINUS expression                                            { $$ = - $2; }
    | expression T_PLUS expression                                  { $$ = $1 + $3; }
    | expression T_MINUS expression                                 { $$ = $1 - $3; }
    | expression T_MULTIPLY expression                              { $$ = $1 * $3; }
    | expression T_BITWISE_AND expression                           { $$ = $1 & $3; }
    | expression T_BITWISE_OR expression                            { $$ = $1 | $3; }
    | expression T_BITWISE_XOR expression                           { $$ = $1 ^ $3; }
    | expression T_BITWISE_LEFT_SHIFT expression                    { $$ = $1 << $3; }
    | expression T_BITWISE_RIGHT_SHIFT expression                   { $$ = $1 >> $3; }
    | T_VAR T_BITWISE_AND expression                                { $$ = getSymbolValueInt($1) & $3; }
    | T_VAR T_BITWISE_OR expression                                 { $$ = getSymbolValueInt($1) | $3; }
    | T_VAR T_BITWISE_XOR expression                                { $$ = getSymbolValueInt($1) ^ $3; }
    | T_VAR T_BITWISE_LEFT_SHIFT expression                         { $$ = getSymbolValueInt($1) << $3; }
    | T_VAR T_BITWISE_RIGHT_SHIFT expression                        { $$ = getSymbolValueInt($1) >> $3; }
    | expression T_BITWISE_AND T_VAR                                { $$ = $1 & getSymbolValueInt($3); }
    | expression T_BITWISE_OR T_VAR                                 { $$ = $1 | getSymbolValueInt($3); }
    | expression T_BITWISE_XOR T_VAR                                { $$ = $1 ^ getSymbolValueInt($3); }
    | expression T_BITWISE_LEFT_SHIFT T_VAR                         { $$ = $1 << getSymbolValueInt($3); }
    | expression T_BITWISE_RIGHT_SHIFT T_VAR                        { $$ = $1 >> getSymbolValueInt($3); }
    | T_VAR T_BITWISE_AND T_VAR                                     { $$ = getSymbolValueInt($1) & getSymbolValueInt($3);; }
    | T_VAR T_BITWISE_OR T_VAR                                      { $$ = getSymbolValueInt($1) | getSymbolValueInt($3);; }
    | T_VAR T_BITWISE_XOR T_VAR                                     { $$ = getSymbolValueInt($1) ^ getSymbolValueInt($3);; }
    | T_VAR T_BITWISE_LEFT_SHIFT T_VAR                              { $$ = getSymbolValueInt($1) << getSymbolValueInt($3);; }
    | T_VAR T_BITWISE_RIGHT_SHIFT T_VAR                             { $$ = getSymbolValueInt($1) >> getSymbolValueInt($3);; }
    | T_BITWISE_NOT expression                                      { $$ = ~ $2; }
    | T_BITWISE_NOT T_VAR                                           { $$ = ~ getSymbolValueInt($2); }
    | T_INCREMENT expression                                        { $$ = ++ $2; }
    | expression T_INCREMENT                                        { $$ = $1 ++; }
    | T_DECREMENT expression                                        { $$ = -- $2; }
    | expression T_DECREMENT                                        { $$ = $1 --; }
    | T_INCREMENT T_VAR                                             { $$ = incrementThenAssign($2, 1); }
    | T_VAR T_INCREMENT                                             { $$ = assignThenIncrement($1, 1); }
    | T_DECREMENT T_VAR                                             { $$ = incrementThenAssign($2, -1); }
    | T_VAR T_DECREMENT                                             { $$ = assignThenIncrement($1, -1); }
    | T_LEFT expression T_RIGHT                                     { $$ = $2; }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;

boolean_expression: T_TRUE                                          { if (!block(B_EXPRESSION)) $$ = $1; }
    | boolean_expression T_REL_EQUAL boolean_expression             { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | boolean_expression T_REL_NOT_EQUAL boolean_expression         { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | boolean_expression T_REL_GREAT boolean_expression             { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | boolean_expression T_REL_SMALL boolean_expression             { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | boolean_expression T_REL_GREAT_EQUAL boolean_expression       { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | boolean_expression T_REL_SMALL_EQUAL boolean_expression       { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | boolean_expression T_LOGIC_AND boolean_expression             { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | boolean_expression T_LOGIC_OR boolean_expression              { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | T_LOGIC_NOT boolean_expression                                { if (!block(B_EXPRESSION)) $$ = ! $2; }
    | T_LOGIC_NOT T_VAR                                             { if (!block(B_EXPRESSION)) { $$ = ! getSymbolValueBool($2); } else { free($2); } }
    | T_VAR T_REL_EQUAL boolean_expression                          { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) == $3; } else { free($1); } }
    | T_VAR T_REL_NOT_EQUAL boolean_expression                      { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) != $3; } else { free($1); } }
    | T_VAR T_REL_GREAT boolean_expression                          { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) > $3; } else { free($1); } }
    | T_VAR T_REL_SMALL boolean_expression                          { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) < $3; } else { free($1); } }
    | T_VAR T_REL_GREAT_EQUAL boolean_expression                    { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) >= $3; } else { free($1); } }
    | T_VAR T_REL_SMALL_EQUAL boolean_expression                    { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) <= $3; } else { free($1); } }
    | T_VAR T_LOGIC_AND boolean_expression                          { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) && $3; } else { free($1); } }
    | T_VAR T_LOGIC_OR boolean_expression                           { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) || $3; } else { free($1); } }
    | boolean_expression T_REL_EQUAL T_VAR                          { if (!block(B_EXPRESSION)) { $$ = $1 == getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_REL_NOT_EQUAL T_VAR                      { if (!block(B_EXPRESSION)) { $$ = $1 != getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_REL_GREAT T_VAR                          { if (!block(B_EXPRESSION)) { $$ = $1 > getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_REL_SMALL T_VAR                          { if (!block(B_EXPRESSION)) { $$ = $1 < getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_REL_GREAT_EQUAL T_VAR                    { if (!block(B_EXPRESSION)) { $$ = $1 >= getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_REL_SMALL_EQUAL T_VAR                    { if (!block(B_EXPRESSION)) { $$ = $1 <= getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_LOGIC_AND T_VAR                          { if (!block(B_EXPRESSION)) { $$ = $1 && getSymbolValueBool($3); } else { free($3); } }
    | boolean_expression T_LOGIC_OR T_VAR                           { if (!block(B_EXPRESSION)) { $$ = $1 || getSymbolValueBool($3); } else { free($3); } }
    | T_VAR T_REL_EQUAL T_VAR                                       { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) == getSymbolValueFloat($3); } else { free($1); free($3); } }
    | T_VAR T_REL_NOT_EQUAL T_VAR                                   { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) != getSymbolValueFloat($3); } else { free($1); free($3); } }
    | T_VAR T_REL_GREAT T_VAR                                       { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) > getSymbolValueFloat($3); } else { free($1); free($3); } }
    | T_VAR T_REL_SMALL T_VAR                                       { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) < getSymbolValueFloat($3); } else { free($1); free($3); } }
    | T_VAR T_REL_GREAT_EQUAL T_VAR                                 { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) >= getSymbolValueFloat($3); } else { free($1); free($3); } }
    | T_VAR T_REL_SMALL_EQUAL T_VAR                                 { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) <= getSymbolValueFloat($3); } else { free($1); free($3); } }
    | T_VAR T_LOGIC_AND T_VAR                                       { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) && getSymbolValueBool($3); } else { free($1); free($3); } }
    | T_VAR T_LOGIC_OR T_VAR                                        { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) || getSymbolValueBool($3); free($3); } else { free($1); free($3); } }
    | mixed_expression T_REL_EQUAL mixed_expression                 { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | mixed_expression T_REL_NOT_EQUAL mixed_expression             { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | mixed_expression T_REL_GREAT mixed_expression                 { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | mixed_expression T_REL_SMALL mixed_expression                 { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | mixed_expression T_REL_GREAT_EQUAL mixed_expression           { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | mixed_expression T_REL_SMALL_EQUAL mixed_expression           { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | mixed_expression T_LOGIC_AND mixed_expression                 { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | mixed_expression T_LOGIC_OR mixed_expression                  { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | mixed_expression T_REL_EQUAL boolean_expression               { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | mixed_expression T_REL_NOT_EQUAL boolean_expression           { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | mixed_expression T_REL_GREAT boolean_expression               { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | mixed_expression T_REL_SMALL boolean_expression               { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | mixed_expression T_REL_GREAT_EQUAL boolean_expression         { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | mixed_expression T_REL_SMALL_EQUAL boolean_expression         { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | mixed_expression T_LOGIC_AND boolean_expression               { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | mixed_expression T_LOGIC_OR boolean_expression                { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | boolean_expression T_REL_EQUAL mixed_expression               { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | boolean_expression T_REL_NOT_EQUAL mixed_expression           { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | boolean_expression T_REL_GREAT mixed_expression               { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | boolean_expression T_REL_SMALL mixed_expression               { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | boolean_expression T_REL_GREAT_EQUAL mixed_expression         { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | boolean_expression T_REL_SMALL_EQUAL mixed_expression         { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | boolean_expression T_LOGIC_AND mixed_expression               { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | boolean_expression T_LOGIC_OR mixed_expression                { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | T_LOGIC_NOT mixed_expression                                  { if (!block(B_EXPRESSION)) $$ = ! $2; }
    | T_VAR T_REL_EQUAL mixed_expression                            { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) == $3; } else { free($1); } }
    | T_VAR T_REL_NOT_EQUAL mixed_expression                        { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) != $3; } else { free($1); } }
    | T_VAR T_REL_GREAT mixed_expression                            { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) > $3; } else { free($1); } }
    | T_VAR T_REL_SMALL mixed_expression                            { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) < $3; } else { free($1); } }
    | T_VAR T_REL_GREAT_EQUAL mixed_expression                      { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) >= $3; } else { free($1); } }
    | T_VAR T_REL_SMALL_EQUAL mixed_expression                      { if (!block(B_EXPRESSION)) { $$ = getSymbolValueFloat($1) <= $3; } else { free($1); } }
    | T_VAR T_LOGIC_AND mixed_expression                            { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) && $3; } else { free($1); } }
    | T_VAR T_LOGIC_OR mixed_expression                             { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) || $3; } else { free($1); } }
    | mixed_expression T_REL_EQUAL T_VAR                            { if (!block(B_EXPRESSION)) { $$ = $1 == getSymbolValueFloat($3); } else { free($3); } }
    | mixed_expression T_REL_NOT_EQUAL T_VAR                        { if (!block(B_EXPRESSION)) { $$ = $1 != getSymbolValueFloat($3); } else { free($3); } }
    | mixed_expression T_REL_GREAT T_VAR                            { if (!block(B_EXPRESSION)) { $$ = $1 > getSymbolValueFloat($3); } else { free($3); } }
    | mixed_expression T_REL_SMALL T_VAR                            { if (!block(B_EXPRESSION)) { $$ = $1 < getSymbolValueFloat($3); } else { free($3); } }
    | mixed_expression T_REL_GREAT_EQUAL T_VAR                      { if (!block(B_EXPRESSION)) { $$ = $1 >= getSymbolValueFloat($3); } else { free($3); } }
    | mixed_expression T_REL_SMALL_EQUAL T_VAR                      { if (!block(B_EXPRESSION)) { $$ = $1 <= getSymbolValueFloat($3); } else { free($3); } }
    | mixed_expression T_LOGIC_AND T_VAR                            { if (!block(B_EXPRESSION)) { $$ = $1 && getSymbolValueBool($3); } else { free($3); } }
    | mixed_expression T_LOGIC_OR T_VAR                             { if (!block(B_EXPRESSION)) { $$ = $1 || getSymbolValueBool($3); } else { free($3); } }
    | expression T_REL_EQUAL expression                             { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | expression T_REL_NOT_EQUAL expression                         { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | expression T_REL_GREAT expression                             { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | expression T_REL_SMALL expression                             { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | expression T_REL_GREAT_EQUAL expression                       { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | expression T_REL_SMALL_EQUAL expression                       { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | expression T_LOGIC_AND expression                             { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | expression T_LOGIC_OR expression                              { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | expression T_REL_EQUAL boolean_expression                     { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | expression T_REL_NOT_EQUAL boolean_expression                 { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | expression T_REL_GREAT boolean_expression                     { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | expression T_REL_SMALL boolean_expression                     { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | expression T_REL_GREAT_EQUAL boolean_expression               { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | expression T_REL_SMALL_EQUAL boolean_expression               { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | expression T_LOGIC_AND boolean_expression                     { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | expression T_LOGIC_OR boolean_expression                      { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | boolean_expression T_REL_EQUAL expression                     { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | boolean_expression T_REL_NOT_EQUAL expression                 { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | boolean_expression T_REL_GREAT expression                     { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | boolean_expression T_REL_SMALL expression                     { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | boolean_expression T_REL_GREAT_EQUAL expression               { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | boolean_expression T_REL_SMALL_EQUAL expression               { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | boolean_expression T_LOGIC_AND expression                     { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | boolean_expression T_LOGIC_OR expression                      { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | T_LOGIC_NOT expression                                        { if (!block(B_EXPRESSION)) $$ = ! $2; }
    | T_VAR T_REL_EQUAL expression                                  { if (!block(B_EXPRESSION)) { $$ = getSymbolValueInt($1) == $3; } else { free($1); } }
    | T_VAR T_REL_NOT_EQUAL expression                              { if (!block(B_EXPRESSION)) { $$ = getSymbolValueInt($1) != $3; } else { free($1); } }
    | T_VAR T_REL_GREAT expression                                  { if (!block(B_EXPRESSION)) { $$ = getSymbolValueInt($1) > $3; } else { free($1); } }
    | T_VAR T_REL_SMALL expression                                  { if (!block(B_EXPRESSION)) { $$ = getSymbolValueInt($1) < $3; } else { free($1); } }
    | T_VAR T_REL_GREAT_EQUAL expression                            { if (!block(B_EXPRESSION)) { $$ = getSymbolValueInt($1) >= $3; } else { free($1); } }
    | T_VAR T_REL_SMALL_EQUAL expression                            { if (!block(B_EXPRESSION)) { $$ = getSymbolValueInt($1) <= $3; } else { free($1); } }
    | T_VAR T_LOGIC_AND expression                                  { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) && $3; } else { free($1); } }
    | T_VAR T_LOGIC_OR expression                                   { if (!block(B_EXPRESSION)) { $$ = getSymbolValueBool($1) || $3; } else { free($1); } }
    | expression T_REL_EQUAL T_VAR                                  { if (!block(B_EXPRESSION)) { $$ = $1 == getSymbolValueInt($3); } else { free($3); } }
    | expression T_REL_NOT_EQUAL T_VAR                              { if (!block(B_EXPRESSION)) { $$ = $1 != getSymbolValueInt($3); } else { free($3); } }
    | expression T_REL_GREAT T_VAR                                  { if (!block(B_EXPRESSION)) { $$ = $1 > getSymbolValueInt($3); } else { free($3); } }
    | expression T_REL_SMALL T_VAR                                  { if (!block(B_EXPRESSION)) { $$ = $1 < getSymbolValueInt($3); } else { free($3); } }
    | expression T_REL_GREAT_EQUAL T_VAR                            { if (!block(B_EXPRESSION)) { $$ = $1 >= getSymbolValueInt($3); } else { free($3); } }
    | expression T_REL_SMALL_EQUAL T_VAR                            { if (!block(B_EXPRESSION)) { $$ = $1 <= getSymbolValueInt($3); } else { free($3); } }
    | expression T_LOGIC_AND T_VAR                                  { if (!block(B_EXPRESSION)) { $$ = $1 && getSymbolValueBool($3); } else { free($3); } }
    | expression T_LOGIC_OR T_VAR                                   { if (!block(B_EXPRESSION)) { $$ = $1 || getSymbolValueBool($3); } else { free($3); } }
    | mixed_expression T_REL_EQUAL expression                       { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | mixed_expression T_REL_NOT_EQUAL expression                   { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | mixed_expression T_REL_GREAT expression                       { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | mixed_expression T_REL_SMALL expression                       { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | mixed_expression T_REL_GREAT_EQUAL expression                 { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | mixed_expression T_REL_SMALL_EQUAL expression                 { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | mixed_expression T_LOGIC_AND expression                       { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | mixed_expression T_LOGIC_OR expression                        { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | expression T_REL_EQUAL mixed_expression                       { if (!block(B_EXPRESSION)) $$ = $1 == $3; }
    | expression T_REL_NOT_EQUAL mixed_expression                   { if (!block(B_EXPRESSION)) $$ = $1 != $3; }
    | expression T_REL_GREAT mixed_expression                       { if (!block(B_EXPRESSION)) $$ = $1 > $3; }
    | expression T_REL_SMALL mixed_expression                       { if (!block(B_EXPRESSION)) $$ = $1 < $3; }
    | expression T_REL_GREAT_EQUAL mixed_expression                 { if (!block(B_EXPRESSION)) $$ = $1 >= $3; }
    | expression T_REL_SMALL_EQUAL mixed_expression                 { if (!block(B_EXPRESSION)) $$ = $1 <= $3; }
    | expression T_LOGIC_AND mixed_expression                       { if (!block(B_EXPRESSION)) $$ = $1 && $3; }
    | expression T_LOGIC_OR mixed_expression                        { if (!block(B_EXPRESSION)) $$ = $1 || $3; }
    | T_LEFT boolean_expression T_RIGHT                             { if (!block(B_EXPRESSION)) $$ = $2; }
;

boolean_expression: T_FALSE                                         { }
;

left_right_bracket: T_UNSIGNED_LONG_LONG_INT                        { $$ = $1; }
    | T_LEFT_BRACKET expression T_RIGHT_BRACKET                     { disable_complex_mode = true; Symbol* symbol = addSymbolInt(NULL, $2); symbol->sign = 1; $$ = symbol->id; pushLeftRightBracketStack(symbol->id); disable_complex_mode = false; }
    | T_LEFT_BRACKET T_MINUS expression T_RIGHT_BRACKET             { disable_complex_mode = true; Symbol* symbol = addSymbolInt(NULL, -$3); symbol->sign = 1; $$ = symbol->id; pushLeftRightBracketStack(symbol->id); disable_complex_mode = false; }
    | T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET                       { disable_complex_mode = true; Symbol* symbol = addSymbolString(NULL, $2); symbol->sign = 1; $$ = symbol->id; pushLeftRightBracketStack(symbol->id); disable_complex_mode = false; }
    | T_LEFT_BRACKET T_VAR T_RIGHT_BRACKET                          { disable_complex_mode = true; Symbol* symbol = createCloneFromSymbolByName(NULL, K_ANY, $2, K_ANY); symbol->sign = 1; $$ = symbol->id; pushLeftRightBracketStack(symbol->id); disable_complex_mode = false; }
    | T_LEFT_BRACKET T_MINUS T_VAR T_RIGHT_BRACKET                  { disable_complex_mode = true; Symbol* symbol = createCloneFromSymbolByName(NULL, K_ANY, $3, K_ANY); symbol->sign = -1; $$ = symbol->id; pushLeftRightBracketStack(symbol->id); disable_complex_mode = false; }
    | left_right_bracket left_right_bracket                         { }
;

variable: T_VAR                                                     { $$ = $1; }
    | variable T_EQUAL T_STRING                                     { updateSymbolString($1, $3); $$ = ""; }
    | variable T_EQUAL T_VAR                                        { updateSymbolByClonningName($1, $3); $$ = ""; }
    | variable T_EQUAL T_VAR left_right_bracket                     { updateSymbolByClonningComplexElement($1, $3); $$ = ""; }
    | variable T_EQUAL mixed_expression                             { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL expression                                   { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL boolean_expression                           { updateSymbolBool($1, $3); $$ = ""; }
    | variable T_EQUAL liststart                                    { finishComplexModeWithUpdate($1); $$ = ""; free($1); }
    | variable T_EQUAL dictionarystart                              { finishComplexModeWithUpdate($1); $$ = ""; free($1); }
    | variable T_EQUAL T_VAR T_LEFT function_call_parameters_start                                      { if (phase == PROGRAM) { callFunction($3, NULL); updateSymbolByClonningFunctionReturn($1, $3, NULL); } else { free($1); free($3); } $$ = ""; }
    | variable T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start                          { if (phase == PROGRAM) { callFunction($5, $3); updateSymbolByClonningFunctionReturn($1, $5, $3); } else { free($1); free($3); free($5); } $$ = ""; }
    | T_RETURN variable                                             { returnSymbol($2); $$ = ""; }
    | variable_complex_element                                      { if (is_interactive && !inject_mode) { printSymbolValueEndWithNewLine(getComplexElementBySymbolId(variable_complex_element, variable_complex_element_symbol_id), false, false); $$ = ""; } else { yyerror("Syntax error"); } }
    | variable_complex_element T_EQUAL T_STRING                     { updateComplexElementString($3); $$ = ""; }
    | variable_complex_element T_EQUAL T_VAR                        { updateComplexElementSymbol(getSymbol($3)); free($3); $$ = ""; }
    | variable_complex_element T_EQUAL T_VAR left_right_bracket     { updateComplexElementSymbol(getComplexElementThroughLeftRightBracketStack($3, 0)); $$ = ""; }
    | variable_complex_element T_EQUAL mixed_expression             { updateComplexElementFloat($3); $$ = ""; }
    | variable_complex_element T_EQUAL expression                   { updateComplexElementFloat($3); $$ = ""; }
    | variable_complex_element T_EQUAL boolean_expression           { updateComplexElementBool($3); $$ = ""; }
    | variable_complex_element T_EQUAL liststart                    { updateComplexElementComplex(); $$ = ""; }
    | variable_complex_element T_EQUAL dictionarystart              { updateComplexElementComplex(); $$ = ""; }
    | variable_complex_element T_EQUAL T_VAR T_LEFT function_call_parameters_start                      { if (phase == PROGRAM) { callFunction($3, NULL); updateComplexSymbolByClonningFunctionReturn($3, NULL); } else { free($3); } $$ = ""; }
    | variable_complex_element T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start          { if (phase == PROGRAM) { callFunction($5, $3); updateComplexSymbolByClonningFunctionReturn($5, $3); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL boolean_expression                   { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, K_BOOL, $4, K_ANY); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR left_right_bracket             { createCloneFromComplexElement($2, K_BOOL, $4, K_ANY); $$ = ""; }
    | T_VAR_BOOL T_VAR_LIST T_VAR T_EQUAL T_VAR                     { createCloneFromSymbolByName($3, K_LIST, $5, K_BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL T_VAR                     { createCloneFromSymbolByName($3, K_DICT, $5, K_BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_LIST T_VAR T_EQUAL liststart                 { finishComplexMode($3, K_BOOL); $$ = ""; free($3); }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL dictionarystart           { finishComplexMode($3, K_BOOL); $$ = ""; free($3); }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($4, NULL); createCloneFromFunctionReturn($2, K_BOOL, $4, NULL, K_ANY); } else { free($2); free($4); } $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($6, $4); createCloneFromFunctionReturn($2, K_BOOL, $6, $4, K_ANY); } else { free($2); free($4); free($6); } $$ = ""; }
    | T_VAR_BOOL T_VAR_LIST T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($5, NULL); createCloneFromFunctionReturn($3, K_LIST, $5, NULL, K_BOOL); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_BOOL T_VAR_LIST T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($7, $5); createCloneFromFunctionReturn($3, K_LIST, $7, $5, K_BOOL); } else { free($3); free($5); free($7); } $$ = ""; }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($5, NULL); createCloneFromFunctionReturn($3, K_DICT, $5, NULL, K_BOOL); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($7, $5); createCloneFromFunctionReturn($3, K_DICT, $7, $5, K_BOOL); } else { free($3); free($5); free($7); } $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR                              { createCloneFromSymbolByName($2, K_NUMBER, $4, K_ANY); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR left_right_bracket           { createCloneFromComplexElement($2, K_NUMBER, $4, K_ANY); $$ = ""; }
    | T_VAR_NUMBER T_VAR_LIST T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, K_LIST, $5, K_NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, K_DICT, $5, K_NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_LIST T_VAR T_EQUAL liststart               { finishComplexMode($3, K_NUMBER); $$ = ""; free($3); }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL dictionarystart         { finishComplexMode($3, K_NUMBER); $$ = ""; free($3); }
    | T_VAR_NUMBER T_VAR T_EQUAL mixed_expression                   { addSymbolFloat($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL expression                         { addSymbolFloat($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($4, NULL); createCloneFromFunctionReturn($2, K_NUMBER, $4, NULL, K_ANY); } else { free($2); free($4); } $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($6, $4); createCloneFromFunctionReturn($2, K_NUMBER, $6, $4, K_ANY); } else { free($2); free($4); free($6); } $$ = ""; }
    | T_VAR_NUMBER T_VAR_LIST T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($5, NULL); createCloneFromFunctionReturn($3, K_LIST, $5, NULL, K_NUMBER); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_NUMBER T_VAR_LIST T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($7, $5); createCloneFromFunctionReturn($3, K_LIST, $7, $5, K_NUMBER); } else { free($3); free($5); free($7); } $$ = ""; }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($5, NULL); createCloneFromFunctionReturn($3, K_DICT, $5, NULL, K_NUMBER); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($7, $5); createCloneFromFunctionReturn($3, K_DICT, $7, $5, K_NUMBER); } else { free($3); free($5); free($7); } $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_STRING                           { addSymbolString($2, $4); $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR                              { createCloneFromSymbolByName($2, K_STRING, $4, K_ANY); $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR left_right_bracket           { createCloneFromComplexElement($2, K_STRING, $4, K_ANY); $$ = ""; }
    | T_VAR_STRING T_VAR_LIST T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, K_LIST, $5, K_STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, K_DICT, $5, K_STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_LIST T_VAR T_EQUAL liststart               { finishComplexMode($3, K_STRING); $$ = ""; free($3); }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL dictionarystart         { finishComplexMode($3, K_STRING); $$ = ""; free($3); }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($4, NULL); createCloneFromFunctionReturn($2, K_STRING, $4, NULL, K_ANY); } else { free($2); free($4); } $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($6, $4); createCloneFromFunctionReturn($2, K_STRING, $6, $4, K_ANY); } else { free($2); free($4); free($6); } $$ = ""; }
    | T_VAR_STRING T_VAR_LIST T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($5, NULL); createCloneFromFunctionReturn($3, K_LIST, $5, NULL, K_STRING); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_STRING T_VAR_LIST T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($7, $5); createCloneFromFunctionReturn($3, K_LIST, $7, $5, K_STRING); } else { free($3); free($5); free($7); } $$ = ""; }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($5, NULL); createCloneFromFunctionReturn($3, K_DICT, $5, NULL, K_STRING); } else { free($3); free($5); } $$ = ""; }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($7, $5); createCloneFromFunctionReturn($3, K_DICT, $7, $5, K_STRING); } else { free($3); free($5); free($7); } $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_STRING                              { addSymbolAnyString($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_VAR                                 { createCloneFromSymbolByName($2, K_ANY, $4, K_ANY); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_VAR left_right_bracket              { createCloneFromComplexElement($2, K_ANY, $4, K_ANY); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL boolean_expression                    { addSymbolAnyBool($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL mixed_expression                      { addSymbolAnyFloat($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL expression                            { addSymbolAnyFloat($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($4, NULL); createCloneFromFunctionReturn($2, K_ANY, $4, NULL, K_ANY); } else { free($2); free($4); } $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($6, $4); createCloneFromFunctionReturn($2, K_ANY, $6, $4, K_ANY); } else { free($2); free($4); free($6); } $$ = ""; }
    | T_VAR_LIST T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, K_LIST, $4, K_ANY); $$ = "";}
    | T_VAR_LIST T_VAR T_EQUAL liststart                            { finishComplexMode($2, K_ANY); $$ = ""; free($2); }
    | T_VAR_LIST T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($4, NULL); createCloneFromFunctionReturn($2, K_LIST, $4, NULL, K_ANY); } else { free($2); free($4); } $$ = ""; }
    | T_VAR_LIST T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($6, $4); createCloneFromFunctionReturn($2, K_LIST, $6, $4, K_ANY); } else { free($2); free($4); free($6); } $$ = ""; }
    | T_VAR_DICT T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, K_DICT, $4, K_ANY); $$ = "";}
    | T_VAR_DICT T_VAR T_EQUAL dictionarystart                      { finishComplexMode($2, K_ANY); $$ = ""; free($2); }
    | T_VAR_DICT T_VAR T_EQUAL T_VAR T_LEFT function_call_parameters_start                    { if (phase == PROGRAM) { callFunction($4, NULL); createCloneFromFunctionReturn($2, K_DICT, $4, NULL, K_ANY); } else { free($2); free($4); } $$ = ""; }
    | T_VAR_DICT T_VAR T_EQUAL T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start        { if (phase == PROGRAM) { callFunction($6, $4); createCloneFromFunctionReturn($2, K_DICT, $6, $4, K_ANY); } else { free($2); free($4); free($6); } $$ = ""; }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } $$ = "";}
;

variable_complex_element:                                           { }
    | T_VAR left_right_bracket                                      { buildVariableComplexElement($1, NULL); }
;

liststart:                                                          { addSymbolList(NULL); }
    | liststart T_LEFT_BRACKET list T_RIGHT_BRACKET                 { if (isNestedComplexMode()) { pushNestedComplexModeStack(getComplexMode()); finishComplexMode(NULL, K_ANY); } }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;

list:                                                               { }
    | T_NEWLINE list                                                { }
    | liststart T_COMMA list                                        { }
    | list T_COMMA liststart                                        { }
    | list T_COMMA T_NEWLINE liststart                              { }
    | liststart T_COMMA liststart                                   { }
    | liststart T_COMMA T_NEWLINE liststart                         { }
    | dictionarystart T_COMMA list                                  { }
    | list T_COMMA dictionarystart                                  { }
    | list T_COMMA T_NEWLINE dictionarystart                        { }
    | dictionarystart T_COMMA dictionarystart                       { }
    | dictionarystart T_COMMA T_NEWLINE dictionarystart             { }
    | dictionarystart                                               { }
    | list T_COMMA list                                             { }
    | list T_NEWLINE                                                { }
;
list: T_TRUE                                                        { addSymbolBool(NULL, $1); }
;
list: T_FALSE                                                       { addSymbolBool(NULL, $1); }
;
list: expression                                                    { addSymbolFloat(NULL, $1); }
;
list: mixed_expression                                              { addSymbolFloat(NULL, $1); }
;
list: T_STRING                                                      { addSymbolString(NULL, $1); }
;
list: T_VAR                                                         { cloneSymbolToComplex($1, NULL); }
;
list: T_VAR left_right_bracket                                      { buildVariableComplexElement($1, NULL); }
;

dictionarystart:                                                                { addSymbolDict(NULL); }
    | dictionarystart T_LEFT_CURLY_BRACKET dictionary T_RIGHT_CURLY_BRACKET     { if (isNestedComplexMode()) { pushNestedComplexModeStack(getComplexMode()); finishComplexMode(NULL, K_ANY); } }
    | error T_NEWLINE parser                                                    { if (is_interactive) { yyerrok; yyclearin; } }
;

dictionary:                                                             { }
    | T_NEWLINE dictionary                                              { }
    | T_STRING T_COLON dictionarystart T_COMMA dictionary               { popNestedComplexModeStack($1); }
    | T_STRING T_COLON dictionarystart                                  { popNestedComplexModeStack($1); }
    | T_STRING T_COLON liststart T_COMMA dictionary                     { popNestedComplexModeStack($1); }
    | T_STRING T_COLON liststart                                        { popNestedComplexModeStack($1); }
    | dictionary T_COMMA dictionary                                     { }
    | dictionary T_NEWLINE                                              { }
    | error T_NEWLINE parser                                            { if (is_interactive) { yyerrok; yyclearin; } }
;
dictionary: T_STRING T_COLON T_TRUE                                 { addSymbolBool($1, $3); }
;
dictionary: T_STRING T_COLON T_FALSE                                { addSymbolBool($1, $3); }
;
dictionary: T_STRING T_COLON expression                             { addSymbolFloat($1, $3); }
;
dictionary: T_STRING T_COLON mixed_expression                       { addSymbolFloat($1, $3); }
;
dictionary: T_STRING T_COLON T_STRING                               { addSymbolString($1, $3); }
;
dictionary: T_STRING T_COLON T_VAR                                  { cloneSymbolToComplex($3, $1); }
;
dictionary: T_STRING T_COLON T_VAR left_right_bracket               { buildVariableComplexElement($3, $1); }
;

decisionstart:                                                                      { decision_mode = function_mode; handle_end_keyword(); }
    | decisionstart T_LEFT_CURLY_BRACKET decision T_RIGHT_CURLY_BRACKET             { decision_mode = NULL; }
    | decisionstart T_NEWLINE                                                       { decision_mode = NULL; }
;

decision:                                                                           { }
    | T_NEWLINE decision                                                            { }
    | decision T_COMMA decision                                                     { }
    | decision T_NEWLINE                                                            { }
;
decision: boolean_expression T_COLON T_VAR T_LEFT function_call_parameters_start    { addBooleanDecision(); free($3); }
;
decision: T_DEFAULT T_COLON T_VAR T_LEFT function_call_parameters_start             { addDefaultDecision(); free($3); }
;

module: T_VAR                                                                       { appendModuleToModuleBuffer($1); }
    | module T_DOT module                                                           { }
    | module T_DIVIDE module                                                        { }
    | module T_BACKSLASH module                                                     { }
    | T_DOT T_DOT module                                                            { prependModuleToModuleBuffer(".."); }
    | module T_DOT T_DOT module                                                     { prependModuleToModuleBuffer(".."); }
    | T_DOT T_DOT T_DIVIDE module                                                   { prependModuleToModuleBuffer(".."); }
    | T_DOT T_DOT T_BACKSLASH module                                                { prependModuleToModuleBuffer(".."); }
;

function_name: T_VAR                                                                { addFunctionNameToFunctionNamesBuffer($1); }
    | function_name T_COMMA function_name                                           { }
;

expression:                                                         { }
    | T_TIMES_DO_INT expression                                     { }
;

preparser_line:                                                     { }
    | T_TIMES_DO_INT preparser_line                                 { }
;

quit:
    | T_NEWLINE                                              {
        if (is_interactive) {
            print_bye_bye();
        } else {
            YYABORT;
        }
        freeEverything();
        exit(E_SUCCESS);
    }
    | expression T_NEWLINE                                   {
        if (is_interactive) {
            print_bye_bye();
        } else {
            YYABORT;
        }
        freeEverything();
        exit($1);
    }
    | T_VAR T_NEWLINE                                        {
        long long code = getSymbolValueInt($1);
        if (is_interactive) {
            print_bye_bye();
        } else {
            YYABORT;
        }
        freeEverything();
        exit(code);
    }
;

json_parser:
    | json_parser dictionarystart                                   { Symbol* symbol = finishComplexMode(NULL, K_ANY); returnVariable(symbol); }
;

%%

int main(int argc, char** argv) {
#if !defined(__clang__) || !(defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
    char ch;
    while ((ch = getopt_long(argc, argv, "hv", long_options, NULL)) != -1)
    {
        switch (ch)
        {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                printf("%s\n", __KAOS_LANGUAGE_VERSION__);
                exit(0);
            case '?':
                printf("Unknown option `-%c'.\n", optopt);
                print_help();
                break;
        }
    }
#endif

    fp = argc > 1 ? fopen (argv[1], "r") : stdin;
    fp_opened = true;

    if (argc > 1) {
        program_file_path = malloc(strlen(argv[1]) + 1);
        strcpy(program_file_path, argv[1]);

        if (!is_file_exists(program_file_path)) {
            initMainFunction();
            is_interactive = false;
            fp_opened = false;
            throw_error(E_PROGRAM_FILE_DOES_NOT_EXISTS_ON_PATH, program_file_path);
        }

        program_file_dir = malloc(strlen(program_file_path) + 1);
        strcpy(program_file_dir, program_file_path);
        char *ptr = strrchr(program_file_dir, __KAOS_PATH_SEPARATOR_ASCII__);
        if (ptr) {
            *ptr = '\0';
        }
    } else {
        char buff[PATH_MAX];
        GetCurrentDir(buff, PATH_MAX);

        program_file_dir = malloc(strlen(buff) + 1);
        strcpy(program_file_dir, buff);

        program_file_path = strcat_ext(program_file_dir, __KAOS_PATH_SEPARATOR__);
        program_file_path = strcat_ext(program_file_path, __KAOS_INTERACTIVE_MODULE_NAME__);
    }

    is_interactive = (fp != stdin) ? false : true;

    yyin = fp;

    if (is_interactive) {
        #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        using_history();
        read_history(NULL);
        #endif
        greet();
        phase = INIT_PROGRAM;
    } else {
        program_code = fileGetContents(program_file_path);
        size_t program_length = strlen(program_code);
        program_code = (char*)realloc(program_code, program_length + 2);
        program_code[program_length] = '\n';
        program_code[program_length + 1] = '\0';
        switchBuffer(program_code, INIT_PREPARSE);
    }

    initMainFunction();

    do {
        if (is_interactive) {
            if (setjmp(InteractiveShellErrorAbsorber)) {
                phase = INIT_PROGRAM;
                freeComplexModeStack();
                //freeLeftRightBracketStack();

                #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
                    printf("\033[1;44m");
                #endif
                printf("%-*s", InteractiveShellErrorAbsorber_ws_col, "    Absorbed by Interactive Shell");
                #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
                    printf("\033[0m");
                #endif
                printf("\n");

                if (loop_execution_mode) longjmp(InteractiveShellLoopErrorAbsorber, 1);
                if (executed_function) longjmp(InteractiveShellFunctionErrorAbsorber, 1);
            }
        }

        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        !is_interactive ?: printf("%s ", __KAOS_SHELL_INDICATOR__);
        #endif
        yyparse();
        if (!is_interactive) break;
    } while(!feof(yyin));

    freeEverything();

    return 0;
}

void yyerror(const char* s) {
    if (phase == PREPARSE) return;

    yyerror_msg(capitalize(s), getCurrentModule(), yytext);

    if (is_interactive) {
        loop_mode = NULL;
        function_mode = NULL;
        if (isComplexMode()) {
            freeComplexModeStack();
        }
        //freeLeftRightBracketStack();
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        printf("%s ", __KAOS_SHELL_INDICATOR__);
        #endif
        flushLexer();
        phase = INIT_PROGRAM;
        yyrestart_interactive();
        freeModulePathStack();
        initMainContext();
        reset_line_no_to = 0;
        yyparse();
    } else {
        freeEverything();
        exit(E_SYNTAX_ERROR);
    }
}

void freeEverything() {
    free(last_token);
    free(main_function);
    free(scopeless);
    freeAllSymbols();
    freeAllFunctions();
    freeModulesBuffer();
    freeFunctionNamesBuffer();
    free(function_names_buffer.arr);
    if (strlen(decision_buffer) > 0) free(decision_buffer);
    freeModulePathStack();
    freeModuleStack();
    freeComplexModeStack();
    freeLeftRightBracketStack();
    freeFreeStringStack();
    freeNestedComplexModeStack();

    yylex_destroy();

    if (!is_interactive) {
        free(program_code);
        if (fp_opened)
            fclose(fp);
        free(program_file_path);
        free(program_file_dir);
    } else {
        #if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        clear_history();
        for (unsigned long long i = __KAOS_LANGUAGE_KEYWORD_COUNT__; i < suggestions_length; i++) {
            free(suggestions[i]);
        }
        #endif
    }

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}
