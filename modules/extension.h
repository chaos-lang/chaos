#ifndef EXTENSION_H
#define EXTENSION_H

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <windows.h>
#define LIBTYPE HINSTANCE
#define OPENLIB(libname) LoadLibraryW(L"libname")
#define LIBFUNC(lib, fn) GetProcAddress((lib), (fn))
#else
#include <dlfcn.h>
#define LIBTYPE void*
#define OPENLIB(libname) dlopen((libname), RTLD_NOW | RTLD_GLOBAL)
#define LIBFUNC(lib, fn) dlsym((lib), (fn))
#endif

#include "../functions/function.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
typedef long long int (*lib_func)();
#else
typedef void (*lib_func)();
#endif

void callRegisterInSharedObject(char* so_path);
void callFunctionFromSharedObject(_Function* function);
lib_func getFunctionFromSharedObject(char* so_path, char* function_name);

#endif
