#ifndef INJECTOR_H
#define INJECTOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../enums.h"

extern bool inject_mode;

void injectCode(char *code, enum Phase phase_arg);
void switchBuffer(char *code, enum Phase phase_arg);
void yyrestart_interactive();
void flushLexer();
void parseTheModuleContent(char *module_path);

#endif
