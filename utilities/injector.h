#ifndef INJECTOR_H
#define INJECTOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "loop.h"
#include "function.h"

extern Loop* loop_mode;
extern bool inject_mode;

void injectCode(char *code, enum Phase phase_arg);
void yyrestart_interactive();
void flushLexer();
void parseTheModuleContent(char *module_path);

char *last_token;

void recordToken(char *token, int length);
bool isStreamOpen();

#endif
