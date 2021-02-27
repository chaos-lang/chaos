/*
 * Description: Extension module of the Chaos Programming Language's source
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
 */

#ifndef KAOS_EXTENSION_H
#define KAOS_EXTENSION_H

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   include <windows.h>
#   define LIBTYPE HINSTANCE
#   define OPENLIB(libname) LoadLibrary(libname)
#   define CLOSELIB(handle) FreeLibrary(handle)
#   define LIBFUNC(lib, fn) GetProcAddress((lib), (fn))
typedef FARPROC lib_func;
#else
#   include <dlfcn.h>
#   define LIBTYPE void*
#   define OPENLIB(libname) dlopen((libname), RTLD_NOW | RTLD_NODELETE)
#   define CLOSELIB(handle) dlclose(handle)
#   define LIBFUNC(lib, fn) dlsym((lib), (fn))
typedef void (*lib_func)();
#endif

#include "function.h"
#include "../Chaos.h"

typedef struct dynamic_library {
    void     *handle;
    lib_func  func;
} dynamic_library;

void initKaosApi();
void callRegisterInDynamicLibrary(char* dynamic_library_path);
void callFunctionFromDynamicLibrary(_Function* function);
dynamic_library getFunctionFromDynamicLibrary(char* dynamic_library_path, char* function_name);
void returnVariable(Symbol* symbol);

#endif
