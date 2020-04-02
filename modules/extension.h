#ifndef EXTENSION_H
#define EXTENSION_H

#include <stdio.h>
#include <dlfcn.h>

#include "../functions/function.h"

typedef void (*lib_func)();

void callRegisterInSharedObject(char* so_path);
void callFunctionFromSharedObject(_Function* function);
lib_func getFunctionFromSharedObject(char* so_path, char* function_name);

#endif
