/*
 * Description: Modularity feature of the Chaos Programming Language's source
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

#ifndef KAOS_MODULE_H
#define KAOS_MODULE_H

#include <stdio.h>

#include "symbol.h"
#include "errors.h"
#include "../lexer/lexer.h"
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
void handleModuleImport(char *module_name, bool directly_import, char *parent_context);
void moduleImportParse(char *module_path);
char* resolveModulePath(char *module_name, bool directly_import, char *parent_context);
void moduleImportCleanUp(char *module_path);
void freeModulesBuffer();
void pushModuleStack(char *module_path, char *module);
void popModuleStack();
void freeModulePathStack();
void freeModuleStack();
char* getCurrentModule();
char* getMainModuleDir();
char* getParentDir(char* path);
void stripLastPathSegment(char* path);
char* searchSpellsIfNotExits(char* module_path, char* relative_path);

#endif
