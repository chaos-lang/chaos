#include "ast.h"

unsigned long long ast_node_id_counter = 0;
bool enable_branch_out = false;
unsigned long long loops_inside_function_counter = 0;

ASTNode* addASTNodeBase(enum ASTNodeType node_type, char *strings[], size_t strings_size, union Value value, enum ValueType value_type) {
    ASTNode* ast_node;
    ast_node = (struct ASTNode*)calloc(1, sizeof(ASTNode) + strings_size * sizeof *ast_node->strings);
    ast_node_id_counter++;

    ast_node->id = ast_node_id_counter;
    ast_node->node_type = node_type;
    ast_node->value = value;
    ast_node->value_type = value_type;
    ast_node->module = malloc(1 + strlen(module_path_stack.arr[module_path_stack.size - 1]));
    strcpy(ast_node->module, module_path_stack.arr[module_path_stack.size - 1]);

    ast_node->strings_size = strings_size;
    for (size_t i = 0; i < ast_node->strings_size; ++i) {
        ast_node->strings[i] = strings[i];
    }

    if (ast_debug_enabled)
        printf(
            "(Create)ASTNode: {id: %llu, node_type: %u, module: %s, string_size: %ld}\n",
            ast_node->id,
            ast_node->node_type,
            ast_node->module,
            ast_node->strings_size
        );

    return ast_node;
}

ASTNode* addASTNode(enum ASTNodeType node_type, char *strings[], size_t strings_size) {
    union Value value;
    return addASTNodeBase(node_type, strings, strings_size, value, V_VOID);
}

ASTNode* addASTNodeBool(enum ASTNodeType node_type, char *strings[], size_t strings_size, bool b, ASTNode* node) {
    union Value value;
    value.b = b;
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, value, V_BOOL);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeInt(enum ASTNodeType node_type, char *strings[], size_t strings_size, long long i, ASTNode* node) {
    union Value value;
    value.i = i;
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, value, V_INT);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeFloat(enum ASTNodeType node_type, char *strings[], size_t strings_size, long double f, ASTNode* node) {
    union Value value;
    value.f = f;
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, value, V_FLOAT);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeString(enum ASTNodeType node_type, char *strings[], size_t strings_size, char *s, ASTNode* node) {
    union Value value;
    value.s = malloc(1 + strlen(s));
    strcpy(value.s, s);
    free(s);
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, value, V_STRING);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeBranch(enum ASTNodeType node_type, ASTNode* l_node, ASTNode* r_node) {
    char *strings[] = {};
    size_t strings_size = 0;
    union Value value;
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, value, V_VOID);
    ast_node->left = l_node;
    ast_node->right = r_node;
    return ast_node;
}

ASTNode* addASTNodeAssign(enum ASTNodeType node_type, char *strings[], size_t strings_size, ASTNode* node) {
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, node->value, V_VOID);
    ast_node->right = node;
    return ast_node;
}

ASTNode* addASTNodeFull(enum ASTNodeType node_type, char *strings[], size_t strings_size, ASTNode* l_node, ASTNode* r_node) {
    union Value value;
    ASTNode* ast_node = addASTNodeBase(node_type, strings, strings_size, value, V_VOID);
    ast_node->left = l_node;
    ast_node->right = r_node;
    return ast_node;
}

void ASTNodeNext(ASTNode* ast_node) {
    if (ast_node_cursor != NULL) {
        if (enable_branch_out) {
            ast_node_cursor->child = ast_node;
            enable_branch_out = false;
        } else {
            ast_node_cursor->next = ast_node;
        }
    } else {
        ast_root_node = ast_node;
    }
    ast_node_cursor = ast_node;
}

void ASTBranchOut() {
    ast_node_cursor_backup = ast_node_cursor;
    enable_branch_out = true;
}

void ASTMergeBack() {
    if (loops_inside_function_counter > 0)
        loops_inside_function_counter--;
    if (ast_node_cursor_backup != NULL && loops_inside_function_counter == 0) {
        ast_node_cursor = ast_node_cursor_backup;
        ast_node_cursor_backup = NULL;
    }
}

ASTNode* free_node(ASTNode* ast_node) {
    if (ast_node == NULL) {
        return ast_node;
    }

    free_node(ast_node->depend);
    free_node(ast_node->right);
    free_node(ast_node->left);
    free_node(ast_node->child);

    if (ast_debug_enabled)
        printf(
            "(Free)ASTNode: {id: %llu, node_type: %u, module: %s, string_size: %ld}\n",
            ast_node->id,
            ast_node->node_type,
            ast_node->module,
            ast_node->strings_size
        );

    ASTNode* next_node = ast_node->next;

    for (size_t i = 0; i < ast_node->strings_size; ++i) {
        free(ast_node->strings[i]);
    }
    if (ast_node->value_type == V_STRING) free(ast_node->value.s);
    free(ast_node->module);
    free(ast_node);
    return free_node(next_node);
}
