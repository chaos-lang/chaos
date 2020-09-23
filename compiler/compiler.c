#include "compiler.h"

struct stat dir_stat = {0};

unsigned short indent_length = 4;

void compile(char *module, enum Phase phase_arg, bool is_interactive) {
    printf("Starting compiling...\n");
    ASTNode* ast_node = ast_root_node;

    if (stat(__KAOS_BUILD_DIRECTORY__, &dir_stat) == -1) {
        printf("Creating build directory...\n");
        #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
            _mkdir(__KAOS_BUILD_DIRECTORY__);
        #else
            mkdir(__KAOS_BUILD_DIRECTORY__, 0700);
        #endif
    }

    char c_file_path[PATH_MAX];
    sprintf(c_file_path, "%s/main.c", __KAOS_BUILD_DIRECTORY__);

    printf("Compiling C code into %s\n", c_file_path);

    FILE *c_fp = fopen(c_file_path, "w");
    if (c_fp == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    const char *c_file_base =
        "#include <stdio.h>\n\n"
        "int main() {\n";

    unsigned short indent = indent_length;

    fprintf(c_fp, "%s", c_file_base);

    transpile_node(ast_node, module, c_fp, indent);

    fprintf(c_fp, "}\n");

    fclose(c_fp);
    printf("Finished compiling.\n");
}

ASTNode* transpile_node(ASTNode* ast_node, char *module, FILE *c_fp, unsigned short indent) {
    if (ast_node == NULL) {
        return ast_node;
    }

    if (ast_node->depend != NULL) {
        eval_node(ast_node->depend, module);
    }

    if (ast_node->right != NULL) {
        eval_node(ast_node->right, module);
    }

    if (ast_node->left != NULL) {
        eval_node(ast_node->left, module);
    }

    fprintf(c_fp, "%*c", indent, ' ');
    switch (ast_node->node_type)
    {
        case AST_PRINT_STRING:
            fprintf(c_fp, "printf(\"%s\\n\");", ast_node->value.s);
            break;
        default:
            break;
    }
    fprintf(c_fp, "\n");

    return transpile_node(ast_node, module, c_fp, indent);
}
