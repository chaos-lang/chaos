#ifndef LANGUAGE_H
#define LANGUAGE_H

#define __LANGUAGE_NAME__ "Chaos"
#define __LANGUAGE_VERSION__ "0.0.1-alpha"
#define __LANGUAGE_MOTTO__ "Turn chaos into magic!"
#define __BYE_BYE__ "\033[5;42m You have chosen the order! \033[0m"
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    #define __SHELL_INDICATOR__ "\033[0;90m###\033[0m "
    #define __SHELL_INDICATOR_BLOCK__ "\033[0;90m...\033[0m "
#else
    #define __SHELL_INDICATOR__ "### "
    #define __SHELL_INDICATOR_BLOCK__ "... "
#endif

#include <stdio.h>

#include "platform.h"

void greet();

#endif
