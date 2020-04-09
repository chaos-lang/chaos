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
char *program_file_path;
char *program_file_dir;
%}

%union {
    bool bval;
    long long ival;
    long double fval;
    char *sval;
    unsigned long long lluval;
}

%token START_PROGRAM START_PREPARSE
%token<bval> T_TRUE T_FALSE
%token<ival> T_INT
%token<fval> T_FLOAT
%token<sval> T_STRING T_VAR
%token<lluval> T_UNSIGNED_LONG_LONG_INT
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_EQUAL
%token T_LEFT_BRACKET T_RIGHT_BRACKET T_LEFT_CURLY_BRACKET T_RIGHT_CURLY_BRACKET T_COMMA T_DOT T_COLON
%token T_NEWLINE T_QUIT
%token T_PRINT
%token T_VAR_BOOL T_VAR_NUMBER T_VAR_STRING T_VAR_ARRAY T_VAR_DICT T_VAR_ANY
%token T_DEL T_RETURN T_VOID T_DEFAULT
%token T_SYMBOL_TABLE T_FUNCTION_TABLE
%token T_TIMES_DO T_FOREACH T_AS T_END T_FUNCTION T_IMPORT T_FROM T_BACKSLASH
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
%type<sval> arraystart
%type<ival> array
%type<lluval> left_right_bracket

%destructor {
    free($$);
} <sval>

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
    | T_IMPORT module T_NEWLINE                                     { handleModuleImport(NULL, false); }
    | T_IMPORT module T_AS T_VAR T_NEWLINE                          { handleModuleImport($4, false); }
    | T_FROM module T_IMPORT T_MULTIPLY                             { handleModuleImport(NULL, true); }
    | T_FROM module T_IMPORT function_name                          { handleModuleImport(NULL, true); }
    | T_END decisionstart                                           { }
    | error T_NEWLINE                                               { yyerrok; }
;

function:
    | T_VAR_BOOL T_FUNCTION T_VAR function_parameters_start         { startFunction($3, K_BOOL); }
    | T_VAR_NUMBER T_FUNCTION T_VAR function_parameters_start       { startFunction($3, K_NUMBER); }
    | T_VAR_STRING T_FUNCTION T_VAR function_parameters_start       { startFunction($3, K_STRING); }
    | T_VAR_ANY T_FUNCTION T_VAR function_parameters_start          { startFunction($3, K_ANY); }
    | T_VAR_ARRAY T_FUNCTION T_VAR function_parameters_start        { startFunction($3, K_ARRAY); }
    | T_VAR_DICT T_FUNCTION T_VAR function_parameters_start         { startFunction($3, K_DICT); }
    | T_VOID T_FUNCTION T_VAR function_parameters_start             { startFunction($3, K_VOID); }
    | T_PRINT T_VAR T_LEFT function_call_parameters_start           { if (phase == PROGRAM) { callFunction($2, NULL); printFunctionReturn($2, NULL); } free($2); }
    | T_VAR T_LEFT function_call_parameters_start                   { if (phase == PROGRAM) { callFunction($1, NULL); } free($1); }
    | T_PRINT T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start       { if (phase == PROGRAM) { callFunction($4, $2); printFunctionReturn($4, $2); } free($4); free($2); }
    | T_VAR T_DOT T_VAR T_LEFT function_call_parameters_start               { if (phase == PROGRAM) { callFunction($3, $1); } free($3); free($1); }
    | error T_NEWLINE                                               { if (is_interactive) { yyerrok; yyclearin; } }
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

function_parameters: T_VAR_BOOL T_VAR                               { addFunctionParameter($2, K_BOOL); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_NUMBER T_VAR                             { addFunctionParameter($2, K_NUMBER); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_STRING T_VAR                             { addFunctionParameter($2, K_STRING); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_ARRAY T_VAR                              { addFunctionParameter($2, K_ARRAY); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR_DICT T_VAR                               { addFunctionParameter($2, K_DICT); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_TRUE                                         { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterBool($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_FALSE                                        { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterBool($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_INT                                          { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterInt($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_FLOAT                                        { if (!block(B_FUNCTION) && phase == PROGRAM) addFunctionCallParameterFloat($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_STRING                                       { if (!block(B_FUNCTION) && phase == PROGRAM) { addFunctionCallParameterString($1); } free($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

function_parameters: T_VAR                                          { if (!block(B_FUNCTION) && phase == PROGRAM) { addFunctionCallParameterSymbol($1); } free($1); }
    | function_parameters T_COMMA function_parameters               { }
    | function_parameters T_NEWLINE                                 { }
;

parser:
    | parser line                                                   {
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        is_interactive ? (
            loop_mode || function_mode ? printf("%s ", __KAOS_SHELL_INDICATOR_BLOCK__) : (
                inject_mode ? : printf("%s ", __KAOS_SHELL_INDICATOR__)
            )
        ) : printf("");
        #endif
    }
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE                                    { if (is_interactive && isStreamOpen()) printf("%Lg\n", $1); }
    | expression T_NEWLINE                                          { if (is_interactive && isStreamOpen()) printf("%lld\n", $1); }
    | variable T_NEWLINE                                            { if ($1[0] != '\0' && is_interactive) { printSymbolValueEndWithNewLine(getSymbol($1)); free($1); } }
    | loop T_NEWLINE                                                { }
    | T_QUIT T_NEWLINE                                              {
        if (is_interactive) {
            print_bye_bye();
        } else {
            YYABORT;
        }
        freeEverything();
        exit(E_SUCCESS);
    }
    | T_PRINT print T_NEWLINE                                       { }
    | T_SYMBOL_TABLE T_NEWLINE                                      { printSymbolTable(); }
    | T_DEL T_VAR T_NEWLINE                                         { removeSymbolByName($2); free($2); }
    | T_DEL T_VAR left_right_bracket T_NEWLINE                      { removeComplexElement($2, $3); free($2); }
    | T_FUNCTION_TABLE T_NEWLINE                                    { printFunctionTable(); }
    | function T_NEWLINE                                            { }
    | T_END decisionstart                                           { }
    | T_IMPORT module T_NEWLINE                                     { if (is_interactive) handleModuleImport(NULL, false); }
    | T_IMPORT module T_AS T_VAR T_NEWLINE                          { if (is_interactive) handleModuleImport($4, false); }
    | T_FROM module T_IMPORT T_MULTIPLY                             { if (is_interactive) handleModuleImport(NULL, true); }
    | T_FROM module T_IMPORT function_name                          { if (is_interactive) handleModuleImport(NULL, true); }
    | error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }
;

print: T_VAR left_right_bracket                                     { printSymbolValueEndWithNewLine(getComplexElementBySymbolId($1, $2)); free($1); }
;
print: T_VAR                                                        { printSymbolValueEndWithNewLine(getSymbol($1)); free($1); }
;
print: T_INT                                                        { printf("%lld\n", $1); }
;
print: T_FLOAT                                                      { printf("%Lg\n", $1); }
;
print: T_STRING                                                     { printf("%s\n", $1); free($1); }
;

mixed_expression: T_FLOAT                                           { $$ = $1; }
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
;

expression: T_INT                                                   { $$ = $1; }
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
    | T_LEFT_BRACKET T_INT T_RIGHT_BRACKET                          { Symbol* symbol = addSymbolInt(NULL, $2); symbol->sign = 1; $$ = symbol->id; }
    | T_LEFT_BRACKET T_MINUS T_INT T_RIGHT_BRACKET                  { Symbol* symbol = addSymbolInt(NULL, -$3); symbol->sign = 1; $$ = symbol->id; }
    | T_LEFT_BRACKET T_STRING T_RIGHT_BRACKET                       { Symbol* symbol = addSymbolString(NULL, $2); symbol->sign = 1; $$ = symbol->id; }
    | T_LEFT_BRACKET T_VAR T_RIGHT_BRACKET                          { Symbol* symbol = createCloneFromSymbolByName(NULL, K_ANY, $2, K_ANY); symbol->sign = 1; $$ = symbol->id; }
    | T_LEFT_BRACKET T_MINUS T_VAR T_RIGHT_BRACKET                  { Symbol* symbol = createCloneFromSymbolByName(NULL, K_ANY, $3, K_ANY); symbol->sign = -1; $$ = symbol->id; }
;

variable: T_VAR                                                     { $$ = $1; }
    | variable T_EQUAL T_TRUE                                       { updateSymbolBool($1, $3); $$ = ""; }
    | variable T_EQUAL T_FALSE                                      { updateSymbolBool($1, $3); $$ = ""; }
    | variable T_EQUAL T_INT                                        { updateSymbolInt($1, $3); $$ = ""; }
    | variable T_EQUAL T_FLOAT                                      { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL T_STRING                                     { updateSymbolString($1, $3); $$ = ""; }
    | variable T_EQUAL T_VAR                                        { updateSymbolByClonningName($1, $3); $$ = ""; }
    | variable T_EQUAL T_VAR left_right_bracket                     { updateSymbolByClonningComplexElement($1, $3, $4); $$ = ""; }
    | variable T_EQUAL mixed_expression                             { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL expression                                   { updateSymbolFloat($1, $3); $$ = ""; }
    | variable T_EQUAL boolean_expression                           { updateSymbolBool($1, $3); $$ = ""; }
    | T_RETURN variable                                             { returnSymbol($2); $$ = ""; }
    | variable left_right_bracket                                   { if ($1[0] != '\0' && is_interactive) printSymbolValueEndWithNewLine(getComplexElementBySymbolId($1, $2)); $$ = ""; free($1); }
    | variable left_right_bracket T_EQUAL T_TRUE                    { updateComplexElementBool($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL T_FALSE                   { updateComplexElementBool($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL T_INT                     { updateComplexElementInt($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL T_FLOAT                   { updateComplexElementFloat($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL T_STRING                  { updateComplexElementString($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL T_VAR                     { updateComplexElementSymbol($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL mixed_expression          { updateComplexElementFloat($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL expression                { updateComplexElementFloat($1, $2, $4); $$ = ""; }
    | variable left_right_bracket T_EQUAL boolean_expression        { updateComplexElementBool($1, $2, $4); $$ = ""; }
;

variable: T_VAR_BOOL                                                { }
    | T_VAR_BOOL T_VAR T_EQUAL T_TRUE                               { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_FALSE                              { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL boolean_expression                   { addSymbolBool($2, $4); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, K_BOOL, $4, K_ANY); $$ = ""; }
    | T_VAR_BOOL T_VAR T_EQUAL T_VAR left_right_bracket             { createCloneFromComplexElement($2, K_BOOL, $4, $5, K_ANY); $$ = ""; }
    | T_VAR_BOOL T_VAR_ARRAY T_VAR T_EQUAL T_VAR                    { createCloneFromSymbolByName($3, K_ARRAY, $5, K_BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL T_VAR                     { createCloneFromSymbolByName($3, K_DICT, $5, K_BOOL); $$ = ""; }
    | T_VAR_BOOL T_VAR_ARRAY T_VAR T_EQUAL arraystart               { finishComplexMode($3, K_BOOL); $$ = ""; free($3); }
    | T_VAR_BOOL T_VAR_DICT T_VAR T_EQUAL dictionarystart           { finishComplexMode($3, K_BOOL); $$ = ""; free($3); }
;

variable: T_VAR_NUMBER                                              { }
    | T_VAR_NUMBER T_VAR T_EQUAL T_INT                              { addSymbolInt($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_FLOAT                            { addSymbolFloat($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR                              { createCloneFromSymbolByName($2, K_NUMBER, $4, K_ANY); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL T_VAR left_right_bracket           { createCloneFromComplexElement($2, K_NUMBER, $4, $5, K_ANY); $$ = ""; }
    | T_VAR_NUMBER T_VAR_ARRAY T_VAR T_EQUAL T_VAR                  { createCloneFromSymbolByName($3, K_ARRAY, $5, K_NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, K_DICT, $5, K_NUMBER); $$ = ""; }
    | T_VAR_NUMBER T_VAR_ARRAY T_VAR T_EQUAL arraystart             { finishComplexMode($3, K_NUMBER); $$ = ""; free($3); }
    | T_VAR_NUMBER T_VAR_DICT T_VAR T_EQUAL dictionarystart         { finishComplexMode($3, K_NUMBER); $$ = ""; free($3); }
    | T_VAR_NUMBER T_VAR T_EQUAL mixed_expression                   { addSymbolFloat($2, $4); $$ = ""; }
    | T_VAR_NUMBER T_VAR T_EQUAL expression                         { addSymbolFloat($2, $4); $$ = ""; }
;

variable: T_VAR_STRING                                              { }
    | T_VAR_STRING T_VAR T_EQUAL T_STRING                           { addSymbolString($2, $4); $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR                              { createCloneFromSymbolByName($2, K_STRING, $4, K_ANY); $$ = ""; }
    | T_VAR_STRING T_VAR T_EQUAL T_VAR left_right_bracket           { createCloneFromComplexElement($2, K_STRING, $4, $5, K_ANY); $$ = ""; }
    | T_VAR_STRING T_VAR_ARRAY T_VAR T_EQUAL T_VAR                  { createCloneFromSymbolByName($3, K_ARRAY, $5, K_STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL T_VAR                   { createCloneFromSymbolByName($3, K_DICT, $5, K_STRING); $$ = ""; }
    | T_VAR_STRING T_VAR_ARRAY T_VAR T_EQUAL arraystart             { finishComplexMode($3, K_STRING); $$ = ""; free($3); }
    | T_VAR_STRING T_VAR_DICT T_VAR T_EQUAL dictionarystart         { finishComplexMode($3, K_STRING); $$ = ""; free($3); }
;

variable: T_VAR_ANY                                                 { }
    | T_VAR_ANY T_VAR T_EQUAL T_STRING                              { addSymbolAnyString($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_INT                                 { addSymbolAnyInt($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_FLOAT                               { addSymbolAnyFloat($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_TRUE                                { addSymbolAnyBool($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_FALSE                               { addSymbolAnyBool($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_VAR                                 { createCloneFromSymbolByName($2, K_ANY, $4, K_ANY); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL T_VAR left_right_bracket              { createCloneFromComplexElement($2, K_ANY, $4, $5, K_ANY); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL boolean_expression                    { addSymbolAnyBool($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL mixed_expression                      { addSymbolAnyFloat($2, $4); $$ = ""; }
    | T_VAR_ANY T_VAR T_EQUAL expression                            { addSymbolAnyFloat($2, $4); $$ = ""; }
;

variable: T_VAR_ARRAY                                               { }
    | T_VAR_ARRAY T_VAR T_EQUAL T_VAR                               { createCloneFromSymbolByName($2, K_ARRAY, $4, K_ANY); $$ = "";}
    | T_VAR_ARRAY T_VAR T_EQUAL arraystart                          { finishComplexMode($2, K_ANY); $$ = ""; free($2); }
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
    | T_VAR_DICT T_VAR T_EQUAL T_VAR                                { createCloneFromSymbolByName($2, K_DICT, $4, K_ANY); $$ = "";}
    | T_VAR_DICT T_VAR T_EQUAL dictionarystart                      { finishComplexMode($2, K_ANY); $$ = ""; free($2); }
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
;

decisionstart:                                                                      { decision_mode = function_mode; handle_end_keyword(); }
    | decisionstart T_LEFT_CURLY_BRACKET decision T_RIGHT_CURLY_BRACKET             { decision_mode = NULL; }
    | decisionstart T_NEWLINE                                                       { decision_mode = NULL; }
;

decision:                                                                           { }
    | T_NEWLINE decision                                                            { }
;

decision: boolean_expression T_COLON T_VAR T_LEFT function_call_parameters_start    { addBooleanDecision(); free($3); }
    | decision T_COMMA decision                                                     { }
    | decision T_NEWLINE                                                            { }
;

decision: T_DEFAULT T_COLON T_VAR T_LEFT function_call_parameters_start             { addDefaultDecision(); free($3); }
    | decision T_COMMA decision                                                     { }
    | decision T_NEWLINE                                                            { }
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

function_parameters_start: error T_NEWLINE parser                   { if (is_interactive) { yyerrok; yyclearin; } }
function_call_parameters_start: error T_NEWLINE parser              { if (is_interactive) { yyerrok; yyclearin; } }
function_parameters: error T_NEWLINE parser                         { if (is_interactive) { yyerrok; yyclearin; } }
print: error T_NEWLINE parser                                       { if (is_interactive) { yyerrok; yyclearin; } }
mixed_expression: error T_NEWLINE parser                            { if (is_interactive) { yyerrok; yyclearin; } }
expression: error T_NEWLINE parser                                  { if (is_interactive) { yyerrok; yyclearin; } }
variable: error T_NEWLINE parser                                    { if (is_interactive) { yyerrok; yyclearin; } }
arraystart: error T_NEWLINE parser                                  { if (is_interactive) { yyerrok; yyclearin; } }
dictionarystart: error T_NEWLINE parser                             { if (is_interactive) { yyerrok; yyclearin; } }
dictionary: error T_NEWLINE parser                                  { if (is_interactive) { yyerrok; yyclearin; } }
loop: error T_NEWLINE parser                                        { if (is_interactive) { yyerrok; yyclearin; } }

%%

int main(int argc, char** argv) {
    fp = argc > 1 ? fopen (argv[1], "r") : stdin;

    if (argc > 1) {
        program_file_path = malloc(strlen(argv[1]) + 1);
        strcpy(program_file_path, argv[1]);
        program_file_path = relative_path_to_absolute(program_file_path);

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
    }

    initMainFunction();

    do {
        if (is_interactive) {
            if (setjmp(InteractiveShellErrorAbsorber)) {
                phase = INIT_PROGRAM;

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
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
        printf("%s ", __KAOS_SHELL_INDICATOR__);
        #endif
        flushLexer();
        phase = INIT_PROGRAM;
        yyrestart_interactive();
        freeModulePathStack();
        initMainFunction();
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

    yylex_destroy();

    if (!is_interactive) {
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
