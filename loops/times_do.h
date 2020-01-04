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
    injectCode(repeatStr(loop_mode->body, loop_mode->iter - 1));
}

char *repeatStr(char *str, size_t count) {
    if (count == 0) return NULL;
    char *ret = malloc (strlen (str) * count + count);
    if (ret == NULL) return NULL;
    strcpy (ret, str);
    while (--count > 0) {
        strcat (ret, " ");
        strcat (ret, str);
    }
    return ret;
}
