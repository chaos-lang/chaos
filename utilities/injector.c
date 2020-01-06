#include "injector.h"

void recordToken(char *token, int length) {
    if (strcmp(token, " ") != 0) {
        last_token = (char *) malloc(strlen(token) + 1);
        last_token = token;
    }

    if (loop_mode != NULL) {
        if (loop_mode->nested_counter == 0 && strcmp(token, "end") == 0) {
            return;
        }
        strcat(loop_mode->body, token);
    }
}

bool isForeach() {
    if (strcmp(last_token, "foreach") == 0 || strcmp(last_token, "as") == 0) return true;
    return false;
}
