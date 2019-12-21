%{

#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yyparse();
extern FILE* yyin;

void yyerror(const char* s);

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

/**
 * Determination a platform of an operation system
 * Fully supported supported only GNU GCC/G++, partially on Clang/LLVM
 */

#if defined(_WIN32)
    #define __PLATFORM_NAME__ "windows" // Windows
#elif defined(_WIN64)
    #define __PLATFORM_NAME__ "windows" // Windows
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #define __PLATFORM_NAME__ "windows" // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
    #define __PLATFORM_NAME__ "android" // Android (implies Linux, so it must come first)
#elif defined(__linux__)
    #define __PLATFORM_NAME__ "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
    #include <sys/param.h>
    #if defined(BSD)
        #define __PLATFORM_NAME__ "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
    #endif
#elif defined(__hpux)
    #define __PLATFORM_NAME__ "hp-ux" // HP-UX
#elif defined(_AIX)
    #define __PLATFORM_NAME__ "aix" // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
        #define __PLATFORM_NAME__ "ios" // Apple iOS
    #elif TARGET_OS_IPHONE == 1
        #define __PLATFORM_NAME__ "ios" // Apple iOS
    #elif TARGET_OS_MAC == 1
        #define __PLATFORM_NAME__ "osx" // Apple OSX
    #endif
#elif defined(__sun) && defined(__SVR4)
    #define __PLATFORM_NAME__ "solaris" // Oracle Solaris, Open Indiana
#else
    #define __PLATFORM_NAME__ NULL
#endif

char __language[] = "Chaos Language";
char __version[] = "0.0.1-alpha";
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
	| calculation line
;

line: T_NEWLINE
    | mixed_expression T_NEWLINE		{ printf("%f\n", $1);}
    | expression T_NEWLINE				{ printf("%i\n", $1); }
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

int main() {
	printf("%s %s (%s %s)\n", __language, __version, __DATE__, __TIME__);
	printf("GCC version: %d.%d.%d on %s\n",__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);

	yyin = stdin;

	do {
		printf("%s", "### ");
		yyparse();
	} while(!feof(yyin));

	return 0;
}

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s\n", s);
	exit(1);
}
