%{

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utilities/platform.h"
#include "utilities/language.h"
#include "utilities/helpers.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

bool is_interactive = true;
%}

%union {
	int ival;
	float fval;
}

%token<ival> T_INT
%token<fval> T_FLOAT
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT
%token T_NEWLINE T_QUIT
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%type<ival> expression
%type<fval> mixed_expression

%start calculation

%%

calculation:
	| calculation line					{ is_interactive ? printf("\n%s ", __SHELL_INDICATOR__) : printf("\n"); }
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE		{ printf("%f", $1); }
    | expression T_NEWLINE				{ printf("%i", $1); }
    | T_QUIT T_NEWLINE					{ printf("bye!\n"); exit(0); }
;

mixed_expression: T_FLOAT                 		 		{ $$ = $1; }
	| mixed_expression T_PLUS mixed_expression	 		{ $$ = $1 + $3; }
	| mixed_expression T_MINUS mixed_expression	 		{ $$ = $1 - $3; }
	| mixed_expression T_MULTIPLY mixed_expression		{ $$ = $1 * $3; }
	| mixed_expression T_DIVIDE mixed_expression	 	{ $$ = $1 / $3; }
	| T_LEFT mixed_expression T_RIGHT		 			{ $$ = $2; }
	| expression T_PLUS mixed_expression	 	 		{ $$ = $1 + $3; }
	| expression T_MINUS mixed_expression	 	 		{ $$ = $1 - $3; }
	| expression T_MULTIPLY mixed_expression 	 		{ $$ = $1 * $3; }
	| expression T_DIVIDE mixed_expression	 			{ $$ = $1 / $3; }
	| mixed_expression T_PLUS expression	 	 		{ $$ = $1 + $3; }
	| mixed_expression T_MINUS expression	 	 		{ $$ = $1 - $3; }
	| mixed_expression T_MULTIPLY expression 	 		{ $$ = $1 * $3; }
	| mixed_expression T_DIVIDE expression	 			{ $$ = $1 / $3; }
	| expression T_DIVIDE expression		 			{ $$ = $1 / (float)$3; }
;

expression: T_INT						{ $$ = $1; }
	| expression T_PLUS expression		{ $$ = $1 + $3; }
	| expression T_MINUS expression		{ $$ = $1 - $3; }
	| expression T_MULTIPLY expression	{ $$ = $1 * $3; }
	| T_LEFT expression T_RIGHT			{ $$ = $2; }
;

%%

int main(int argc, char** argv) {
	printf("%s %s (%s %s)\n", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
	printf("GCC version: %d.%d.%d on %s\n",__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);

	FILE *fp = argc > 1 ? fopen (argv[1], "r") : stdin;

	is_interactive = (fp != stdin) ? false : true;

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
