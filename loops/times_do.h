#include <stdio.h>
#include <string.h>
#include "loop.h"
//#include "../utilities/injector.h"

extern Loop* loop_mode;
//extern void recordToken(char *token, int length);
char *repeatStr(char *str, size_t count);

void startTimesDo(int iter) {
    loop_mode = (struct Loop*)malloc(sizeof(Loop));
    loop_mode->type = TIMESDO;
    loop_mode->iter = iter;
}

void endLoop() {
    char *body = malloc(strlen(loop_mode->body));
    strcpy(body, loop_mode->body);
    int iter = loop_mode->iter;
    loop_mode = NULL;
    for (int i = 1; i < iter; i++) {
        injectCode(body);
    }
}
