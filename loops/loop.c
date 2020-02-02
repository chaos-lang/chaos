#include <stdio.h>
#include <string.h>

#include "loop.h"
#include "../symbol.h"
#include "../utilities/injector.h"

void endLoop() {
    if (loop_mode == NULL) return;

    if (loop_mode->nested_counter > 0) {
        loop_mode->nested_counter--;
        return;
    }

    if (function_mode != NULL) {
        loop_mode = NULL;
        return;
    }

    char *body = malloc(strlen(loop_mode->body));
    strcpy(body, loop_mode->body);
    int iter = loop_mode->iter;
    enum LoopType loop_type = loop_mode->type;
    char *array_name = loop_mode->array;
    char *element_name = loop_mode->element.name;
    char *element_key = loop_mode->element.key;
    char *element_value = loop_mode->element.value;
    loop_mode = NULL;

    Symbol* array;

    loop_execution_mode = true;

    switch (loop_type)
    {
        case TIMESDO:
            for (int i = 0; i < iter; i++) {
                injectCode(body);
            }
            break;
        case FOREACH:
            array = getSymbol(array_name);
            for (int i = 0; i < array->children_count; i++) {
                Symbol* child = array->children[i];
                child->name = element_name;
                injectCode(body);
                child->name = NULL;
            }
            break;
        case FOREACH_DICT:
            array = getSymbol(array_name);
            for (int i = 0; i < array->children_count; i++) {
                Symbol* child = array->children[i];
                char *key = malloc(1 + strlen(child->key));
                strcpy(key, child->key);

                addSymbolString(element_key, key);
                child->name = element_value;
                injectCode(body);
                child->name = NULL;
                removeSymbolByName(element_key);
            }
            break;
    }

    loop_execution_mode = false;
}
