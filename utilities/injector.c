#include "injector.h"

void recordToken(char *token, int length) {
    if (strcmp(token, " ") != 0) {
        last_token = (char *) realloc(last_token, strlen(token) + 1);
        strcpy(last_token, token);
    }

    if (loop_mode != NULL && function_mode == NULL) {
        if (loop_mode->nested_counter == 0 && strcmp(token, "end") == 0) {
            free(token);
            return;
        }
        loop_mode->body = strcat_ext(loop_mode->body, token);
    }

    if (function_mode != NULL) {
        if (loop_mode == NULL && strcmp(token, "end") == 0) {
            free(token);
            return;
        }
        function_mode->body = strcat_ext(function_mode->body, token);
    }

    if (decision_mode != NULL) {
        if (strcmp(token, "{") == 0 || strcmp(token, "}") == 0 || strcmp(token, ",") == 0) {
            free(token);
            return;
        }
        if (strcmp(token, ":") == 0) {
            free(token);
            if (strlen(trim_string(decision_buffer)) == 0) {
                return;
            }
            append_to_array(
                &decision_mode->decision_expressions,
                trim_string(decision_buffer)
            );
            free(decision_buffer);
            decision_buffer = "";
            return;
        }
        decision_buffer = strcat_ext(decision_buffer, token);
    }

    free(token);
}

bool isForeach() {
    if (last_token == NULL) return false;
    if (strcmp(last_token, "foreach") == 0 || strcmp(last_token, "as") == 0 || strcmp(last_token, ":") == 0) return true;
    return false;
}
