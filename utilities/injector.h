#ifndef INJECTOR_H
#define INJECTOR_H

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "../loops/loop.h"

extern Loop* loop_mode;
extern bool inject_mode;

void injectCode(char *code);

void recordToken(char *token, int length) {
    if (loop_mode != NULL) {
        if (loop_mode->nested_counter == 0 && strcmp(token, "end") == 0) {
            return;
        }
        strcat(loop_mode->body, token);
    }
}

#endif
