#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

#include "symbol.h"
#include "../ast/ast.h"

enum LoopType { TIMESDO, FOREACH, FOREACH_DICT };

typedef struct {
    char *name;
    char *key;
    char *value;
} LoopElement;

typedef struct {
    enum LoopType type;
    unsigned long long iter;
    bool is_infinite;
    unsigned nested_counter;
    char *list;
    LoopElement element;
    struct ASTNode* ast_node;
} Loop;

Loop* loop_mode;
bool loop_execution_mode;
unsigned long long nested_loop_counter;

jmp_buf InteractiveShellLoopErrorAbsorber;
bool interactive_shell_loop_error_absorbed;

jmp_buf LoopBreak;
jmp_buf LoopContinue;

ASTNode* startTimesDo(unsigned long long iter, bool is_infinite, ASTNode* ast_node);
ASTNode* startForeach(char *list_name, char *element_name, ASTNode* ast_node);
ASTNode* startForeachDict(char *dict_name, char *element_key, char *element_value, ASTNode* ast_node);
void breakLoop();
void continueLoop();

#endif
