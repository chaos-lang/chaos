%{
/*
 * Description: Lexer of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2021 Chaos Language Development Authority <info@chaos-lang.org>
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

#undef free

#define YY_DECL int yylex()

extern void recordToken(char *token, int length);
extern int oerrno;
extern void yyerror(const char* s);
extern FILE* tmp_stdin;
extern unsigned long long shell_indicator_block_counter;
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

    char *interpreted_module = malloc(1 + strlen(_ast_root->files[_ast_root->file_count - 1]->module_path));
    strcpy(interpreted_module, _ast_root->files[_ast_root->file_count - 1]->module_path);
// #ifndef CHAOS_COMPILER
//     interpret(interpreted_module, phase_arg, false);
// #else
//     interpret(interpreted_module, phase_arg);
// #endif
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

#ifndef CHAOS_COMPILER
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
#endif

void parseTheModuleContent(char *module_path) {
    char *code = fileGetContents(module_path);
    size_t code_length = strlen(code);
    code = (char*)realloc(code, code_length + 2);
    code[code_length] = '\n';
    code[code_length + 1] = '\0';

    if (code != NULL) {
        module_parsing++;
        int yylineno_backup = yylineno;
        yylineno = 1;
        injectCode(code, INIT_PROGRAM);
        yylineno = yylineno_backup;
        module_parsing--;
        free(code);

#ifndef CHAOS_COMPILER
        if (is_interactive)
            phase = PROGRAM;
#endif
    } else {
        append_to_array_without_malloc(&free_string_stack, module_path);
        throw_error(E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH, module_path);
    }
}

#define YY_SKIP_YYWRAP 1
int yywrap() { if (phase == PREPARSE && module_parsing == 0) { switchBuffer(program_code, INIT_PROGRAM); yyparse(); } return 1; }

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) result = custom_input(buf, result, max_size);

static int custom_input(char *buf, int result, int max_size) {
    if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) {
#if !defined(CHAOS_COMPILER) && !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
        return shell_readline(buf);
#else
        int c = '*';
		int n;
		for ( n = 0; n < max_size && (c = getc( yyin )) != EOF && c != '\n'; ++n )
			buf[n] = (char) c;
		if ( c == '\n' )
			buf[n++] = (char) c;
		if ( c == EOF && ferror( yyin ) )
			YY_FATAL_ERROR( "input in flex scanner failed" );
		result = n;
        buf[result] = '\0';
#   ifndef CHAOS_COMPILER
        fprintf(tmp_stdin, "%s", buf);
#   endif
        return result;
#endif
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
        buf[result] = '\0';
#ifndef CHAOS_COMPILER
        fprintf(tmp_stdin, "%s", buf);
#endif
        return result;
    }
    return -1;
}
%}

%option noinput
%option nounput

%x COMMENT

%%

%{

switch (phase) {
case INIT_PREPARSE:
    phase = PREPARSE;
    return START_PREPARSE;
    break;
case INIT_PROGRAM:
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
"="                             {return T_ASSIGN;}
"+"                             {return T_ADD;}
"-"                             {return T_SUB;}
"*"                             {return T_MUL;}
"/"                             {return T_QUO;}
"%"                             {return T_REM;}
"\\"                            {return T_BACKSLASH;}
"("                             {return T_LPAREN;}
")"                             {return T_RPAREN;}
"["                             {shell_indicator_block_counter++; return T_LBRACK;}
"]"                             {shell_indicator_block_counter--; return T_RBRACK;}
"{"                             {shell_indicator_block_counter++; return T_LBRACE;}
"}"                             {shell_indicator_block_counter--; return T_RBRACE;}
","                             {return T_COMMA;}
"."                             {return T_PERIOD;}
"=="                            {return T_EQL;}
"!="                            {return T_NEQ;}
">"                             {return T_GTR;}
"<"                             {return T_LSS;}
">="                            {return T_GEQ;}
"<="                            {return T_LEQ;}
\&\&|and                        {return T_LAND;}
\|\||or                         {return T_LOR;}
\!|not                          {return T_NOT;}
"&"                             {return T_AND;}
"|"                             {return T_OR;}
"^"                             {return T_XOR;}
"~"                             {return T_TILDE;}
"<<"                            {return T_SHL;}
">>"                            {return T_SHR;}
"++"                            {return T_INC;}
"--"                            {return T_DEC;}
"//".*"\n"                      {yylineno++; return T_NEWLINE;}
"#".*"\n"                       {yylineno++; return T_NEWLINE;}
":"                             {return T_COLON;}
"exit"                          {return T_EXIT;}
"quit"                          {return T_EXIT;}
"print"                         {return T_PRINT;}
"echo"                          {return T_ECHO;}
"pretty"                        {return T_PRETTY;}
"true"                          {yylval.bval = 1; return T_TRUE;}
"false"                         {yylval.bval = 0; return T_FALSE;}
"function_table"                {return T_FUNCTION_TABLE;}
"del"                           {return T_DEL;}
"return"                        {return T_RETURN;}
"default"                       {return T_DEFAULT;}

"times do"                      {shell_indicator_block_counter++; return T_TIMES_DO;}
"end"                           {shell_indicator_block_counter--; return T_END;}
"foreach"                       {shell_indicator_block_counter++; return T_FOREACH;}
"as"                            {return T_AS;}
"from"                          {return T_FROM;}
"INFINITE"                      {return T_INFINITE;}

"/*"                            {BEGIN(COMMENT);}
<COMMENT>"*/"                   {BEGIN(INITIAL);}
<COMMENT>.|"\n"                 {yylineno++;}

\"(\$\{.*\}|\\.|[^\"\\])*\" {
    yylval.sval = (char*)calloc(strlen(yytext)-1, sizeof(char));
    strncpy(yylval.sval, &yytext[1], strlen(yytext)-2);
    return T_STRING;
}

\'(\$\{.*\}|\\.|[^\'\\])*\' {
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
"def"                           {shell_indicator_block_counter++; return T_DEF;}
"import"                        {return T_IMPORT;}
"break"                         {return T_BREAK;}
"continue"                      {return T_CONTINUE;}
[a-zA-Z_][a-zA-Z0-9_]*          {yylval.sval=strdup(yytext); return T_VAR;}
%%
