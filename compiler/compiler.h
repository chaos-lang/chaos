#ifndef COMPILER_H
#define COMPILER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/syslimits.h>
#elif !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
#include <linux/limits.h>
#endif

#include "../ast/ast.h"

bool compiler_mode;
unsigned short indent_length;

void compile(char *module, enum Phase phase_arg, char *bin_file);
ASTNode* transpile_node(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent);

#endif
