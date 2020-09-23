#ifndef LANGUAGE_H
#define LANGUAGE_H

#define __KAOS_LANGUAGE_NAME__ "Chaos"
#define __KAOS_LANGUAGE_VERSION__ "0.0.1-alpha"
#define __KAOS_LANGUAGE_FILE_EXTENSION__ "kaos"
#define __KAOS_LANGUAGE_MOTTO__ "Turn chaos into magic!"
#define __KAOS_LANGUAGE_KEYWORD_COUNT__ 33
#define __KAOS_INTERACTIVE_MODULE_NAME__ "__interactive__."__KAOS_LANGUAGE_FILE_EXTENSION__
#define __KAOS_MAX_RECURSION_DEPTH__ 1000
#define __KAOS_MSG_LINE_LENGTH__ 1000
#define __KAOS_BYE_BYE__ "You have chosen the order! "
#define __KAOS_SPELLS__ "spells"
#define __KAOS_SYNTAX_ERROR__ "Syntax error"
#if defined(__APPLE__) || defined(__MACH__)
    #define __KAOS_DYNAMIC_LIBRARY_EXTENSION__ "dylib"
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define __KAOS_DYNAMIC_LIBRARY_EXTENSION__ "dll"
#else
    #define __KAOS_DYNAMIC_LIBRARY_EXTENSION__ "so"
#endif
#define __KAOS_EXTENSION_REGISTER_FUNCTION__ "KaosRegister"
#define __KAOS_EXTENSION_FUNCTION_PREFIX__ "Kaos_"

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    #define __KAOS_SHELL_INDICATOR__ "\001\033[0;90m\002kaos>\001\033[0m\002 "
    #define __KAOS_SHELL_INDICATOR_BLOCK__ "\001\033[0;90m\002....\001\033[0m\002  "
#else
    #define __KAOS_SHELL_INDICATOR__ "kaos>"
    #define __KAOS_SHELL_INDICATOR_BLOCK__ ".... "
#endif

#define __KAOS_TAB__ "    "

#define __KAOS_BUILD_DIRECTORY__ "build"

#include <stdio.h>
#if !defined(__clang__) || !(defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
#include <getopt.h>
#endif

#include "platform.h"

void greet();
void yyerror_msg(char* error_name, char* current_module, char* cause);
void print_bye_bye();
void print_help();

#if !defined(__clang__) || !(defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
static struct option long_options[] =
{
    {"help", no_argument, NULL, 'h'},
    {"version", no_argument, NULL, 'v'},
    {"debug", no_argument, NULL, 'd'},
    {"compile", no_argument, NULL, 'c'},
    {"output", no_argument, NULL, 'o'},
    {NULL, 0, NULL, 0}
};
#endif

#endif
