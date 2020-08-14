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
