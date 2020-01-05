#include <stdio.h>
#include <string.h>
#include "loop.h"
//#include "../utilities/injector.h"

extern Loop* loop_mode;
//extern void recordToken(char *token, int length);
char *repeatStr(char *str, size_t count);

void startTimesDo(int iter) {
    if (loop_mode != NULL) {
        loop_mode->nested_counter++;
        return;
    }

    loop_mode = (struct Loop*)malloc(sizeof(Loop));
    loop_mode->type = TIMESDO;
    loop_mode->iter = iter;
    loop_mode->nested_counter = 0;
}

void endLoop() {
    if (loop_mode->nested_counter > 0) {
        loop_mode->nested_counter--;
        return;
    }

    char *body = malloc(strlen(loop_mode->body));
    strcpy(body, loop_mode->body);
    int iter = loop_mode->iter;
    loop_mode = NULL;

    for (int i = 0; i < iter; i++) {
        injectCode(body);
    }
}
