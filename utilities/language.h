/*
 * Description: Language module of the Chaos Programming Language's source
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
 */

#ifndef KAOS_LANGUAGE_H
#define KAOS_LANGUAGE_H

#define __KAOS_LANGUAGE_NAME__ "Chaos"
#define __KAOS_VERSION_MAJOR__ 0
#define __KAOS_VERSION_MINOR__ 2
#define __KAOS_VERSION_PATCHLEVEL__ 0
#define __KAOS_LANGUAGE_FILE_EXTENSION__ "kaos"
#define __KAOS_LANGUAGE_MOTTO__ "Turn chaos into magic!"
#define __KAOS_LANGUAGE_KEYWORD_COUNT__ 33
#define __KAOS_INTERACTIVE_MODULE_NAME__ "__interactive__."__KAOS_LANGUAGE_FILE_EXTENSION__
#define __KAOS_MSG_LINE_LENGTH__ 1000
#define __KAOS_BYE_BYE__ "Bye bye!"
#define __KAOS_SPELLS__ "spells"
#define __KAOS_SYNTAX_ERROR__ "Syntax error"
#define __KAOS_INDENT_LENGTH__ 4
#define __KAOS_INDENT_CHAR__ ' '

#if defined(__APPLE__) || defined(__MACH__)
#   define __KAOS_DYNAMIC_LIBRARY_EXTENSION__ "dylib"
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   define __KAOS_DYNAMIC_LIBRARY_EXTENSION__ "dll"
#else
#   define __KAOS_DYNAMIC_LIBRARY_EXTENSION__ "so"
#endif

#define __KAOS_EXTENSION_REGISTER_FUNCTION__ "KaosRegister"
#define __KAOS_EXTENSION_FUNCTION_PREFIX__ "Kaos_"

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
#   define __KAOS_SHELL_INDICATOR__ "\001\033[0;90m\002kaos>\001\033[0m\002 "
#   define __KAOS_SHELL_INDICATOR_BLOCK__ "\001\033[0;90m\002....\001\033[0m\002  "
#else
#   define __KAOS_SHELL_INDICATOR__ "kaos>"
#   define __KAOS_SHELL_INDICATOR_BLOCK__ ".... "
#endif

#define __KAOS_TAB__ "    "

#define __KAOS_BUILD_DIRECTORY__ "build"
#define __KAOS_WINDOWS_EXE_EXT__ ".exe"

#include <stdio.h>

#ifdef CHAOS_DEBUG
#   include <stdlib.h>

#   define free(p) do{ printf( "%s(%d):freeing %p &%p\n", __FILE__, __LINE__, (void*)p, (void*)&p ); free(p);  }while(0)
#endif

#include "platform.h"

void yyerror_msg(char* error_name, char* current_module, char* cause);

#endif
