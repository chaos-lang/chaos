#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "../ast/ast.h"

void interpret(char *module, enum Phase phase_arg, bool is_interactive);
ASTNode* startTimesDo(unsigned long long iter, bool is_infinite, ASTNode* ast_node);
ASTNode* startForeach(char *list_name, char *element_name, ASTNode* ast_node);
ASTNode* startForeachDict(char *list_name, char *element_key, char *element_value, ASTNode* ast_node);
bool is_node_function_related(ASTNode* ast_node);
ASTNode* register_functions(ASTNode* ast_node, char *module);
ASTNode* eval_node(ASTNode* ast_node, char *module);

#endif
