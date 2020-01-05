#ifndef INJECTOR_H
#define INJECTOR_H

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "../loops/loop.h"

extern Loop* loop_mode;

void injectCode(char *code);

void recordToken(char *token, int length) {
    if (loop_mode != NULL && strcmp(token, "end") != 0) {
        strcat(loop_mode->body, token);
    }
}

#endif
