#ifndef LANGUAGE_H
#define LANGUAGE_H

#define __LANGUAGE_NAME__ "Chaos"
#define __LANGUAGE_VERSION__ "0.0.1-alpha"
#define __LANGUAGE_FILE_EXTENSION__ "kaos"
#define __LANGUAGE_MOTTO__ "Turn chaos into magic!"
#define __LANGUAGE_KEYWORD_COUNT__ 34
#define __INTERACTIVE_MODULE_NAME__ "__interactive__."__LANGUAGE_FILE_EXTENSION__
#define __MAX_RECURSION_DEPTH__ 1000
#define __MSG_LINE_LENGTH__ 1000
#define __BYE_BYE__ "You have chosen the order! "
#define __SPELLS__ "spells"
#define __SHARED_OBJECT_EXTENSION__ "so"
#define __EXTENSION_REGISTER_FUNCTION__ "KaosRegister"
#define __EXTENSION_FUNCTION_PREFIX__ "Kaos_"

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    #define __SHELL_INDICATOR__ "\033[0;90m###\033[0m "
    #define __SHELL_INDICATOR_BLOCK__ "\033[0;90m...\033[0m "
#else
    #define __SHELL_INDICATOR__ "### "
    #define __SHELL_INDICATOR_BLOCK__ "... "
#endif

#include <stdio.h>

#include "platform.h"
#include "helpers.h"

void greet();
void yyerror_msg(char* error_name, char* current_module, char* cause);
void print_bye_bye();

#endif
