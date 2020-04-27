#ifndef MODULE_H
#define MODULE_H

#include <stdio.h>

#include "../symbol.h"
#include "../errors.h"
#include "../utilities/injector.h"
#include "../utilities/helpers.h"
#include "extension.h"

string_array modules_buffer;

extern char *program_file_path;
extern char *program_file_dir;
extern char *program_code;
string_array module_path_stack;
string_array module_stack;

extern unsigned short module_parsing;

void initMainContext();
void appendModuleToModuleBuffer(char *name);
void prependModuleToModuleBuffer(char *name);
void handleModuleImport(char *module_name, bool directly_import);
void freeModulesBuffer();
void pushModuleStack(char *module_path, char *module);
void popModuleStack();
void freeModulePathStack();
void freeModuleStack();
char* getCurrentModule();
char* getMainModuleDir();
char* searchSpellsIfNotExits(char* module_path, char* relative_path);

#endif
