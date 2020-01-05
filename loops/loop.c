#include <stdio.h>
#include <string.h>

#include "loop.h"

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
