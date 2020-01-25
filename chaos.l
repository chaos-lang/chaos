%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "utilities/language.h"
#include "utilities/injector.h"
#include "utilities/shell.h"
#include "loop.h"

#define YY_DECL int yylex()

extern void recordToken(char *token, int length);
extern int oerrno;
#define YY_FATAL_ERROR(msg) yy_fatal_error( msg )

#include "chaos.tab.h"

enum Phase { INIT_PREPARSE, PREPARSE, INIT_PROGRAM, PROGRAM };
enum Phase phase = INIT_PREPARSE;

void injectCode(char *code) {
    inject_mode = true;
    phase = INIT_PROGRAM;

    YY_BUFFER_STATE old_buffer = YY_CURRENT_BUFFER;
    YY_BUFFER_STATE new_buffer = yy_scan_string(code);
    yy_switch_to_buffer(new_buffer);
    int ret = yyparse();
    yy_delete_buffer(new_buffer);
    yy_switch_to_buffer(old_buffer);

    inject_mode = false;
}

int yywrap() { if (phase == PREPARSE) { phase = INIT_PROGRAM; rewind(yyin); return 1; } }

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = custom_input(buf, result, max_size);

static int custom_input(char *buf, int result, int max_size) {
    if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) {
        return shell_readline(buf);
    } else {
        errno = 0;
        while ( (result = (int) fread(buf, 1, (yy_size_t) max_size, yyin)) == 0 && ferror(yyin)) {
            if( errno != EINTR) {
                YY_FATAL_ERROR( "input in flex scanner failed" );
                break;
            }
            errno = 0;
            clearerr(yyin);
        }
    }
}
%}

%%

%{

switch (phase)
{
    case INIT_PREPARSE:
        phase = PREPARSE;
        return START_PREPARSE;
        break;
    case INIT_PROGRAM:
        phase = PROGRAM;
        return START_PROGRAM;
        break;
    default:
        break;
}

%}

[ \t]                           {recordToken(strdup(yytext), yyleng);}; // ignore all whitespace
[0-9]+\.[0-9]+                  {recordToken(strdup(yytext), yyleng); yylval.fval = atof(yytext); if (loop_mode == NULL && function_mode == NULL) return T_FLOAT;}
[0-9]+                          {recordToken(strdup(yytext), yyleng); yylval.ival = atoi(yytext); return T_INT;}
\n                              {recordToken(strdup(yytext), yyleng); yylineno = yylineno + 1; return T_NEWLINE;}
"="                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_EQUAL;}
"+"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_PLUS;}
"-"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_MINUS;}
"*"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_MULTIPLY;}
"/"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_DIVIDE;}
"("                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_LEFT;}
")"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_RIGHT;}
"["                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_LEFT_BRACKET;}
"]"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_RIGHT_BRACKET;}
"{"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_LEFT_CURLY_BRACKET;}
"}"                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_RIGHT_CURLY_BRACKET;}
","                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_COMMA;}
"//".*"\n"                      {recordToken(strdup(yytext), yyleng); yylineno = yylineno + 1; return T_NEWLINE;}
"#".*"\n"                       {recordToken(strdup(yytext), yyleng); yylineno = yylineno + 1; return T_NEWLINE;}
":"                             {recordToken(strdup(yytext), yyleng); return T_COLON;}
"exit"                          {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_QUIT;}
"quit"                          {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_QUIT;}
"print"                         {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_PRINT;}
"true"                          {recordToken(strdup(yytext), yyleng); yylval.bval = 1; if (loop_mode == NULL && function_mode == NULL) return T_TRUE;}
"false"                         {recordToken(strdup(yytext), yyleng); yylval.bval = 0; if (loop_mode == NULL && function_mode == NULL) return T_FALSE;}
"symbol_table"                  {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_SYMBOL_TABLE;}
"del"                           {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_DEL;}
"return"                        {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_RETURN;}

"times do"                      {recordToken(strdup(yytext), yyleng); return T_TIMES_DO;}
"end"                           {recordToken(strdup(yytext), yyleng); return T_END;}
"foreach"                       {recordToken(strdup(yytext), yyleng); return T_FOREACH;}
"as"                            {recordToken(strdup(yytext), yyleng); return T_AS;}

(\"|\')([^\\(\"|\')]|\\.)*(\"|\') {
    recordToken(strdup(yytext), yyleng);
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    if (loop_mode == NULL && function_mode == NULL) return T_STRING;
}

bool|boolean                    {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_VAR_BOOL;}
num|number                      {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_VAR_NUMBER;}
str|string                      {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_VAR_STRING;}
list|array                      {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_VAR_ARRAY;}
json|dict                       {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_VAR_DICT;}
any                             {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL) return T_VAR_ANY;}
void|null|nil                   {recordToken(strdup(yytext), yyleng); if (loop_mode == NULL && function_mode == NULL) return T_VOID;}
func|function|def               {recordToken(strdup(yytext), yyleng); return T_FUNCTION;}
[a-zA-Z][a-zA-Z0-9]*            {bool was_foreach = isForeach(); recordToken(strdup(yytext), yyleng); yylval.sval=strdup(yytext); if (loop_mode == NULL && function_mode == NULL || was_foreach) return T_VAR;}
%%
