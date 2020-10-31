%{
/*
 * Description: Lexer of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 *          Melih Sahin <melihsahin24@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ast/ast.h"
#include "lexer/lexer.h"

#define YY_DECL int yylex()

extern void recordToken(char *token, int length);
extern int oerrno;
extern void yyerror(const char* s);
#define YY_FATAL_ERROR(msg) yy_fatal_error( msg )

#include "parser.tab.h"

enum Phase phase = INIT_PROGRAM;
unsigned short module_parsing = 0;

void injectCode(char *code, enum Phase phase_arg) {
    phase = phase_arg;
    inject_mode = true;

    YY_BUFFER_STATE old_buffer = YY_CURRENT_BUFFER;
    YY_BUFFER_STATE new_buffer = yy_scan_string(code);
    yy_switch_to_buffer(new_buffer);
    yyparse();

    if (phase_arg == INIT_JSON_PARSE) {
        phase_arg = PROGRAM;
    } else {
        phase_arg = PREPARSE;
    }

    char *interpreted_module = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - 1]));
    strcpy(interpreted_module, module_path_stack.arr[module_path_stack.size - 1]);
    interpret(interpreted_module, phase_arg, false);
    free(interpreted_module);

    yy_delete_buffer(new_buffer);
    yy_switch_to_buffer(old_buffer);

    inject_mode = false;
}

void switchBuffer(char *code, enum Phase phase_arg) {
    phase = phase_arg;

    YY_BUFFER_STATE old_buffer = YY_CURRENT_BUFFER;
    YY_BUFFER_STATE new_buffer = yy_scan_string(code);
    yy_switch_to_buffer(new_buffer);
    yy_delete_buffer(old_buffer);
}

void yyrestart_interactive() {
    if (!YY_CURRENT_BUFFER){
        yyensure_buffer_stack();
		YY_CURRENT_BUFFER_LVALUE = yy_create_buffer(yyin, YY_BUF_SIZE);
	}

	yy_init_buffer(YY_CURRENT_BUFFER, yyin);
    YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = true;
}

void flushLexer() {
    yy_flush_buffer(YY_CURRENT_BUFFER);
}

void parseTheModuleContent(char *module_path) {
    char *code = fileGetContents(module_path);
    size_t code_length = strlen(code);
    code = (char*)realloc(code, code_length + 2);
    code[code_length] = '\n';
    code[code_length + 1] = '\0';

    if (code != NULL) {
        module_parsing++;
        injectCode(code, INIT_PROGRAM);
        module_parsing--;
        free(code);

        if (is_interactive) {
            phase = PROGRAM;
        }
    } else {
        append_to_array_without_malloc(&free_string_stack, module_path);
        throw_error(E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH, module_path);
    }
}

#define YY_SKIP_YYWRAP 1
int yywrap() { if (phase == PREPARSE && module_parsing == 0) { switchBuffer(program_code, INIT_PROGRAM); yyparse(); } return 1; }

#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
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
        return result;
    }
}
#endif
%}

%x COMMENT

%%

%{

switch (phase)
{
    case INIT_PREPARSE:
        yylineno = reset_line_no_to;
        phase = PREPARSE;
        return START_PREPARSE;
        break;
    case INIT_PROGRAM:
        yylineno = reset_line_no_to;
        phase = PROGRAM;
        return START_PROGRAM;
        break;
    case INIT_JSON_PARSE:
        phase = JSON_PARSE;
        return START_JSON_PARSE;
    default:
        break;
}

%}

[ \t]                           {}; // ignore all whitespace
[0-9]+\.[0-9]+                  {yylval.fval = atof(yytext); return T_FLOAT;}
[0-9]+                          {yylval.ival = atoi(yytext); return T_INT;}
\n                              {yylineno++; return T_NEWLINE;}
"="                             {return T_EQUAL;}
"+"                             {return T_PLUS;}
"-"                             {return T_MINUS;}
"*"                             {return T_MULTIPLY;}
"/"                             {return T_DIVIDE;}
"\\"                            {return T_BACKSLASH;}
"("                             {return T_LEFT;}
")"                             {return T_RIGHT;}
"["                             {return T_LEFT_BRACKET;}
"]"                             {return T_RIGHT_BRACKET;}
"{"                             {return T_LEFT_CURLY_BRACKET;}
"}"                             {return T_RIGHT_CURLY_BRACKET;}
","                             {return T_COMMA;}
"."                             {return T_DOT;}
"=="                            {return T_REL_EQUAL;}
"!="                            {return T_REL_NOT_EQUAL;}
">"                             {return T_REL_GREAT;}
"<"                             {return T_REL_SMALL;}
">="                            {return T_REL_GREAT_EQUAL;}
"<="                            {return T_REL_SMALL_EQUAL;}
\&\&|and                        {return T_LOGIC_AND;}
\|\||or                         {return T_LOGIC_OR;}
\!|not                          {return T_LOGIC_NOT;}
"&"                             {return T_BITWISE_AND;}
"|"                             {return T_BITWISE_OR;}
"^"                             {return T_BITWISE_XOR;}
"~"                             {return T_BITWISE_NOT;}
"<<"                            {return T_BITWISE_LEFT_SHIFT;}
">>"                            {return T_BITWISE_RIGHT_SHIFT;}
"++"                            {return T_INCREMENT;}
"--"                            {return T_DECREMENT;}
"//".*"\n"                      {yylineno++; return T_NEWLINE;}
"#".*"\n"                       {yylineno++; return T_NEWLINE;}
":"                             {return T_COLON;}
"exit"                          {return T_QUIT;}
"quit"                          {return T_QUIT;}
"print"                         {return T_PRINT;}
"echo"                          {return T_ECHO;}
"pretty"                        {return T_PRETTY;}
"true"                          {yylval.bval = 1; return T_TRUE;}
"false"                         {yylval.bval = 0; return T_FALSE;}
"symbol_table"                  {return T_SYMBOL_TABLE;}
"function_table"                {return T_FUNCTION_TABLE;}
"del"                           {return T_DEL;}
"return"                        {return T_RETURN;}
"default"                       {return T_DEFAULT;}

"times do"                      {return T_TIMES_DO;}
"end"                           {return T_END;}
"foreach"                       {return T_FOREACH;}
"as"                            {return T_AS;}
"from"                          {return T_FROM;}
"INFINITE"                      {return T_INFINITE;}

"/*"                            {BEGIN(COMMENT);}
<COMMENT>"*/"                   {BEGIN(INITIAL);}
<COMMENT>.|"\n"                 {yylineno++;}

(\")([^\\(\")]|\\.)*(\") {
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    return T_STRING;
}

(\')([^\\(\')]|\\.)*(\') {
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    return T_STRING;
}

"bool"                          {return T_VAR_BOOL;}
"num"                           {return T_VAR_NUMBER;}
"str"                           {return T_VAR_STRING;}
"list"                          {return T_VAR_LIST;}
"dict"                          {return T_VAR_DICT;}
"any"                           {return T_VAR_ANY;}
"null"                          {return T_NULL;}
"void"                          {return T_VOID;}
"def"                           {return T_FUNCTION;}
"import"                        {return T_IMPORT;}
"break"                         {return T_BREAK;}
"continue"                      {return T_CONTINUE;}
[a-zA-Z_][a-zA-Z0-9_]*          {yylval.sval=strdup(yytext); return T_VAR;}
%%
