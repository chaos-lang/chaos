#include <stdio.h>
#include <string.h>

#include "loop.h"
#include "../symbol.h"
#include "../utilities/injector.h"

bool interactive_shell_loop_error_absorbed = false;
extern int yyparse();

void endLoop() {
    if (loop_mode == NULL) return;

    if (loop_mode->nested_counter > 0) {
        loop_mode->nested_counter--;
        return;
    }

    if (function_mode != NULL) {
        free(loop_mode);
        loop_mode = NULL;
        return;
    }

    char *body = loop_mode->body;
    unsigned long long iter = loop_mode->iter;
    bool is_infinite = loop_mode->is_infinite;
    enum LoopType loop_type = loop_mode->type;
    char *list_name = loop_mode->list;
    char *element_name = loop_mode->element.name;
    char *element_key = loop_mode->element.key;
    char *element_value = loop_mode->element.value;
    Loop* _loop_mode = loop_mode;
    loop_mode = NULL;

    Symbol* list;

    loop_execution_mode = true;

    if (is_interactive) {
        if (setjmp(InteractiveShellLoopErrorAbsorber)) {
            interactive_shell_loop_error_absorbed = true;
        }
    }

    if (!interactive_shell_loop_error_absorbed) {
        switch (loop_type)
        {
            case TIMESDO:
                if (is_infinite) {
                    while (true) {
                        injectCode(body, INIT_PROGRAM);
                    }
                } else {
                    for (unsigned long long i = 0; i < iter; i++) {
                        injectCode(body, INIT_PROGRAM);
                    }
                }
                break;
            case FOREACH:
                list = getSymbol(list_name);
                for (unsigned long long i = 0; i < list->children_count; i++) {
                    Symbol* child = list->children[i];
                    child->name = element_name;
                    injectCode(body, INIT_PROGRAM);
                    child->name = NULL;
                }
                break;
            case FOREACH_DICT:
                list = getSymbol(list_name);
                for (unsigned long long i = 0; i < list->children_count; i++) {
                    Symbol* child = list->children[i];
                    char *key = malloc(1 + strlen(child->key));
                    char *element_key_copy = malloc(1 + strlen(element_key));
                    strcpy(key, child->key);
                    strcpy(element_key_copy, element_key);

                    addSymbolString(element_key_copy, key);
                    child->name = element_value;
                    injectCode(body, INIT_PROGRAM);
                    child->name = NULL;
                    removeSymbolByName(element_key);
                }
                break;
        }
    }

    loop_execution_mode = false;

    switch (loop_type)
    {
        case TIMESDO:
            break;
        case FOREACH:
            free(_loop_mode->list);
            free(_loop_mode->element.name);
            break;
        case FOREACH_DICT:
            free(_loop_mode->list);
            free(_loop_mode->element.key);
            free(_loop_mode->element.value);
            break;
    }

    free(_loop_mode->body);
    free(_loop_mode);

    if (is_interactive && interactive_shell_loop_error_absorbed) {
        interactive_shell_loop_error_absorbed = false;
        yyrestart_interactive();
        yyparse();
    }
}
