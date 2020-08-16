#ifndef EXTENSION_H
#define EXTENSION_H

#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <windows.h>
#define LIBTYPE HINSTANCE
#define OPENLIB(libname) LoadLibrary(libname)
#define CLOSELIB(handle) FreeLibrary(handle)
#define LIBFUNC(lib, fn) GetProcAddress((lib), (fn))
#else
#include <dlfcn.h>
#define LIBTYPE void*
#define OPENLIB(libname) dlopen((libname), RTLD_NOW | RTLD_NODELETE)
#define CLOSELIB(handle) dlclose(handle)
#define LIBFUNC(lib, fn) dlsym((lib), (fn))
#endif

#include "function.h"
#include "../Chaos.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
typedef long long (*lib_func)();
#else
typedef void (*lib_func)();
#endif

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
