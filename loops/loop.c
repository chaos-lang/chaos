#include <stdio.h>
#include <string.h>

#include "loop.h"
#include "../symbol.h"
#include "../utilities/injector.h"
#include "../interpreter/interpreter.h"

bool interactive_shell_loop_error_absorbed = false;
extern int yyparse();

ASTNode* startTimesDo(unsigned long long iter, bool is_infinite, ASTNode* ast_node) {
    ASTNode* next_node;

    if (is_infinite) {
        while (true) {
            next_node = eval_node(ast_node->next, ast_node->module);
        }
    } else {
        for (unsigned long long i = 0; i < iter; i++) {
            next_node = eval_node(ast_node->next, ast_node->module);
        }
    }

    return eval_node(next_node->next, ast_node->module);
}

ASTNode* startForeach(char *list_name, char *element_name, ASTNode* ast_node) {
    ASTNode* next_node;

    Symbol* list = getSymbol(list_name);
    for (unsigned long long i = 0; i < list->children_count; i++) {
        Symbol* child = list->children[i];
        Symbol* clone_symbol = createCloneFromSymbol(element_name, child->type, child, child->secondary_type);
        next_node = eval_node(ast_node->next, ast_node->module);
        removeSymbol(clone_symbol);
    }

    return eval_node(next_node->next, ast_node->module);
}

ASTNode* startForeachDict(char *list_name, char *element_key, char *element_value, ASTNode* ast_node) {
    ASTNode* next_node;

    Symbol* list = getSymbol(list_name);
    for (unsigned long long i = 0; i < list->children_count; i++) {
        Symbol* child = list->children[i];

        addSymbolString(element_key, child->key);
        Symbol* clone_symbol = createCloneFromSymbol(element_value, child->type, child, child->secondary_type);
        next_node = eval_node(ast_node->next, ast_node->module);
        removeSymbol(clone_symbol);
        removeSymbolByName(element_key);
    }

    return eval_node(next_node->next, ast_node->module);
}

bool endLoop() {
    if (loop_mode == NULL) return false;

    if (loop_mode->nested_counter > 0) {
        loop_mode->nested_counter--;
        return false;
    }

    if (function_mode != NULL) {
        free(loop_mode);
        loop_mode = NULL;
        return false;
    }

    printf("RUN LOOP!\n");

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
                        //eval_node(_loop_mode->ast_node->next);
                    }
                } else {
                    for (unsigned long long i = 0; i < iter - 1; i++) {
                        //eval_node(_loop_mode->ast_node->next);
                    }
                }
                break;
            case FOREACH:
                list = getSymbol(list_name);
                for (unsigned long long i = 0; i < list->children_count; i++) {
                    Symbol* child = list->children[i];
                    child->name = element_name;
                    //eval_node(_loop_mode->ast_node->next);
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

    return true;
}
