#ifndef INJECTOR_H
#define INJECTOR_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "loop.h"

extern Loop* loop_mode;
extern bool inject_mode;

void injectCode(char *code);

char *last_token;

void recordToken(char *token, int length);

bool isForeach();

#endif
