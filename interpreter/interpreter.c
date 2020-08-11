#include <math.h>

#include "interpreter.h"

void interpret(char *module, enum Phase phase_arg, bool is_interactive) {
    ASTNode* ast_node = ast_root_node;

    if (is_interactive) {
        if (ast_interactive_cursor != NULL) {
            ast_node = ast_interactive_cursor->next;
        } else {
            ast_interactive_cursor = ast_node;
        }
    }

    switch (phase_arg)
    {
        case PREPARSE:
            phase = PREPARSE;
            register_functions(ast_node, module);
            phase = PROGRAM;
            break;
        case PROGRAM:
            phase = PROGRAM;
            eval_node(ast_node, module);
            break;
        default:
            break;
    }

    if (phase_arg == INIT_PREPARSE) {
        phase = PREPARSE;
        register_functions(ast_node, module);
        phase = PROGRAM;
        eval_node(ast_node, module);
    }

    ast_interactive_cursor = ast_node_cursor;
}

bool is_node_function_related(ASTNode* ast_node) {
    return (ast_node->node_type >= AST_DEFINE_FUNCTION_BOOL && ast_node->node_type <= AST_DEFINE_FUNCTION_VOID) ||
        (ast_node->node_type >= AST_FUNCTION_PARAMETERS_START && ast_node->node_type <= AST_OPTIONAL_FUNCTION_PARAMETER_STRING_DICT) ||
        (ast_node->node_type >= AST_ADD_FUNCTION_NAME && ast_node->node_type <= AST_MODULE_IMPORT_PARTIAL);
}

ASTNode* register_functions(ASTNode* ast_node, char *module) {
    if (ast_node == NULL) {
        return ast_node;
    }

    if (strcmp(ast_node->module, module) != 0) return register_functions(ast_node->next, module);

    if (is_node_function_related(ast_node)){
        if (ast_node->depend != NULL) {
            eval_node(ast_node->depend, module);
            register_functions(ast_node->depend, module);
        }

        if (ast_node->right != NULL) {
            eval_node(ast_node->right, module);
            register_functions(ast_node->right, module);
        }

        if (ast_node->left != NULL) {
            eval_node(ast_node->left, module);
            register_functions(ast_node->left, module);
        }
    }

    if (ast_debug_enabled)
        printf(
            "(Register)ASTNode: {id: %llu, node_type: %u, module: %s, string_size: %ld}\n",
            ast_node->id,
            ast_node->node_type,
            ast_node->module,
            ast_node->strings_size
        );

    switch (ast_node->node_type)
    {
        case AST_DEFINE_FUNCTION_BOOL:
            startFunction(ast_node->strings[0], K_BOOL, K_ANY);
            break;
        case AST_DEFINE_FUNCTION_NUMBER:
            startFunction(ast_node->strings[0], K_NUMBER, K_ANY);
            break;
        case AST_DEFINE_FUNCTION_STRING:
            startFunction(ast_node->strings[0], K_STRING, K_ANY);
            break;
        case AST_DEFINE_FUNCTION_ANY:
            startFunction(ast_node->strings[0], K_ANY, K_ANY);
            break;
        case AST_DEFINE_FUNCTION_LIST:
            startFunction(ast_node->strings[0], K_LIST, K_ANY);
            break;
        case AST_DEFINE_FUNCTION_DICT:
            startFunction(ast_node->strings[0], K_DICT, K_ANY);
            break;
        case AST_DEFINE_FUNCTION_BOOL_LIST:
            startFunction(ast_node->strings[0], K_LIST, K_BOOL);
            break;
        case AST_DEFINE_FUNCTION_BOOL_DICT:
            startFunction(ast_node->strings[0], K_DICT, K_BOOL);
            break;
        case AST_DEFINE_FUNCTION_NUMBER_LIST:
            startFunction(ast_node->strings[0], K_LIST, K_NUMBER);
            break;
        case AST_DEFINE_FUNCTION_NUMBER_DICT:
            startFunction(ast_node->strings[0], K_DICT, K_NUMBER);
            break;
        case AST_DEFINE_FUNCTION_STRING_LIST:
            startFunction(ast_node->strings[0], K_LIST, K_STRING);
            break;
        case AST_DEFINE_FUNCTION_STRING_DICT:
            startFunction(ast_node->strings[0], K_DICT, K_STRING);
            break;
        case AST_DEFINE_FUNCTION_VOID:
            startFunction(ast_node->strings[0], K_VOID, K_ANY);
            break;
        case AST_FUNCTION_PARAMETERS_START:
            if (function_parameters_mode == NULL)
                startFunctionParameters();
            break;
        case AST_FUNCTION_PARAMETER_BOOL:
            addFunctionParameter(ast_node->strings[0], K_BOOL, K_ANY);
            break;
        case AST_FUNCTION_PARAMETER_NUMBER:
            addFunctionParameter(ast_node->strings[0], K_NUMBER, K_ANY);
            break;
        case AST_FUNCTION_PARAMETER_STRING:
            addFunctionParameter(ast_node->strings[0], K_STRING, K_ANY);
            break;
        case AST_FUNCTION_PARAMETER_LIST:
            addFunctionParameter(ast_node->strings[0], K_LIST, K_ANY);
            break;
        case AST_FUNCTION_PARAMETER_BOOL_LIST:
            addFunctionParameter(ast_node->strings[0], K_LIST, K_BOOL);
            break;
        case AST_FUNCTION_PARAMETER_NUMBER_LIST:
            addFunctionParameter(ast_node->strings[0], K_LIST, K_NUMBER);
            break;
        case AST_FUNCTION_PARAMETER_STRING_LIST:
            addFunctionParameter(ast_node->strings[0], K_LIST, K_STRING);
            break;
        case AST_FUNCTION_PARAMETER_DICT:
            addFunctionParameter(ast_node->strings[0], K_DICT, K_ANY);
            break;
        case AST_FUNCTION_PARAMETER_BOOL_DICT:
            addFunctionParameter(ast_node->strings[0], K_DICT, K_BOOL);
            break;
        case AST_FUNCTION_PARAMETER_NUMBER_DICT:
            addFunctionParameter(ast_node->strings[0], K_DICT, K_NUMBER);
            break;
        case AST_FUNCTION_PARAMETER_STRING_DICT:
            addFunctionParameter(ast_node->strings[0], K_DICT, K_STRING);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_BOOL:
            addFunctionOptionalParameterBool(ast_node->strings[0], ast_node->right->value.b);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER:
            if (ast_node->right->value_type == V_INT) {
                addFunctionOptionalParameterInt(ast_node->strings[0], ast_node->right->value.i);
            } else {
                addFunctionOptionalParameterFloat(ast_node->strings[0], ast_node->right->value.f);
            }
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_STRING:
            addFunctionOptionalParameterString(ast_node->strings[0], ast_node->value.s);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_LIST:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_ANY);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_BOOL_LIST:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_BOOL);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER_LIST:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_NUMBER);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_STRING_LIST:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_STRING);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_DICT:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_ANY);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_BOOL_DICT:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_BOOL);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_NUMBER_DICT:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_NUMBER);
            break;
        case AST_OPTIONAL_FUNCTION_PARAMETER_STRING_DICT:
            reverseComplexMode();
            addFunctionOptionalParameterComplex(ast_node->strings[0], K_STRING);
            break;
        case AST_ADD_FUNCTION_NAME:
            addFunctionNameToFunctionNamesBuffer(ast_node->strings[0]);
            break;
        case AST_APPEND_MODULE:
            appendModuleToModuleBuffer(ast_node->strings[0]);
            break;
        case AST_PREPEND_MODULE:
            prependModuleToModuleBuffer(ast_node->strings[0]);
            break;
        case AST_MODULE_IMPORT:
            handleModuleImport(NULL, false);
            break;
        case AST_MODULE_IMPORT_AS:
            handleModuleImport(ast_node->strings[0], false);
            break;
        case AST_MODULE_IMPORT_PARTIAL:
            handleModuleImport(NULL, true);
            break;
        case AST_DECISION_DEFINE:
            decision_mode->decision_node = ast_node->right;
            decision_mode = NULL;
            break;
        default:
            break;
    }

    if (ast_node->node_type >= AST_DEFINE_FUNCTION_BOOL && ast_node->node_type <= AST_DEFINE_FUNCTION_VOID && function_mode != NULL) {
        function_mode->node = ast_node;
        decision_mode = function_mode;
        function_mode = NULL;
    }

    return register_functions(ast_node->next, module);
}

ASTNode* eval_node(ASTNode* ast_node, char *module) {
    if (ast_node == NULL || stop_ast_evaluation) {
        return ast_node;
    }

    if (strcmp(ast_node->module, module) != 0) return eval_node(ast_node->next, module);

    if (ast_node->node_type == AST_DECISION_DEFINE) return eval_node(ast_node->next, module);

    if (ast_node->node_type != AST_FUNCTION_STEP)
        if (is_node_function_related(ast_node)) return eval_node(ast_node->next, module);

    if (ast_node->depend != NULL) {
        eval_node(ast_node->depend, module);
    }

    if (ast_node->right != NULL) {
        eval_node(ast_node->right, module);
    }

    if (ast_node->left != NULL) {
        eval_node(ast_node->left, module);
    }

    if (ast_debug_enabled)
        printf(
            "ASTNode: {id: %llu, node_type: %u, module: %s, string_size: %ld}\n",
            ast_node->id,
            ast_node->node_type,
            ast_node->module,
            ast_node->strings_size
        );

    if (ast_node->node_type == AST_END) {
        return ast_node;
    }

    switch (ast_node->node_type)
    {
        case AST_START_TIMES_DO:
            return startTimesDo(ast_node->right->value.i, false, ast_node);
            break;
        case AST_START_TIMES_DO_INFINITE:
            return startTimesDo(0, true, ast_node);
            break;
        case AST_START_TIMES_DO_VAR:
            return startTimesDo((unsigned) getSymbolValueInt(ast_node->strings[0]), false, ast_node);
            break;
        case AST_START_FOREACH:
            return startForeach(ast_node->strings[0], ast_node->strings[1], ast_node);
            break;
        case AST_START_FOREACH_DICT:
            return startForeachDict(ast_node->strings[0], ast_node->strings[1], ast_node->strings[2], ast_node);
            break;
        default:
            break;
    }


    Symbol* symbol;
    long double l_value;
    long double r_value;
    long long exit_code;

    switch (ast_node->node_type)
    {
        case AST_VAR_CREATE_BOOL:
            addSymbolBool(ast_node->strings[0], ast_node->right->value.b);
            break;
        case AST_VAR_CREATE_BOOL_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_BOOL, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_BOOL_VAR_EL:
            createCloneFromComplexElement(ast_node->strings[0], K_BOOL, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_BOOL_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_BOOL, ast_node->strings[1], NULL, K_ANY);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_BOOL, ast_node->strings[2], ast_node->strings[1], K_ANY);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_NUMBER:
            if (ast_node->right->value_type == V_INT) {
                addSymbolInt(ast_node->strings[0], ast_node->right->value.i);
            } else {
                addSymbolFloat(ast_node->strings[0], ast_node->right->value.f);
            }
            break;
        case AST_VAR_CREATE_NUMBER_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_NUMBER, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_NUMBER_VAR_EL:
            createCloneFromComplexElement(ast_node->strings[0], K_NUMBER, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_NUMBER_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_NUMBER, ast_node->strings[1], NULL, K_ANY);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_NUMBER, ast_node->strings[2], ast_node->strings[1], K_ANY);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_STRING:
            addSymbolString(ast_node->strings[0], ast_node->value.s);
            break;
        case AST_VAR_CREATE_STRING_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_STRING, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_STRING_VAR_EL:
            createCloneFromComplexElement(ast_node->strings[0], K_STRING, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_STRING_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_STRING, ast_node->strings[1], NULL, K_ANY);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_STRING, ast_node->strings[2], ast_node->strings[1], K_ANY);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_ANY_BOOL:
            addSymbolAnyBool(ast_node->strings[0], ast_node->right->value.b);
            break;
        case AST_VAR_CREATE_ANY_NUMBER:
            if (ast_node->right->value_type == V_INT) {
                addSymbolAnyInt(ast_node->strings[0], ast_node->right->value.i);
            } else {
                addSymbolAnyFloat(ast_node->strings[0], ast_node->right->value.f);
            }
            break;
        case AST_VAR_CREATE_ANY_STRING:
            addSymbolAnyString(ast_node->strings[0], ast_node->value.s);
            break;
        case AST_VAR_CREATE_ANY_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_ANY, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_ANY_VAR_EL:
            createCloneFromComplexElement(ast_node->strings[0], K_ANY, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_ANY_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_ANY, ast_node->strings[1], NULL, K_ANY);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_ANY, ast_node->strings[2], ast_node->strings[1], K_ANY);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_LIST:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_ANY);
            break;
        case AST_VAR_CREATE_LIST_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_LIST, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_LIST_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[1], NULL, K_ANY);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[2], ast_node->strings[1], K_ANY);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_DICT:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_ANY);
            break;
        case AST_VAR_CREATE_DICT_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_DICT, ast_node->strings[1], K_ANY);
            break;
        case AST_VAR_CREATE_DICT_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[1], NULL, K_ANY);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[2], ast_node->strings[1], K_ANY);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_BOOL_LIST:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_LIST_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_LIST, ast_node->strings[1], K_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_LIST_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[1], NULL, K_BOOL);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[2], ast_node->strings[1], K_BOOL);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_BOOL_DICT:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_DICT_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_DICT, ast_node->strings[1], K_BOOL);
            break;
        case AST_VAR_CREATE_BOOL_DICT_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[1], NULL, K_BOOL);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[2], ast_node->strings[1], K_BOOL);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_NUMBER_LIST:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_NUMBER);
            break;
        case AST_VAR_CREATE_NUMBER_LIST_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_LIST, ast_node->strings[1], K_NUMBER);
            break;
        case AST_VAR_CREATE_NUMBER_LIST_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[1], NULL, K_NUMBER);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[2], ast_node->strings[1], K_NUMBER);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_NUMBER_DICT:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_NUMBER);
            break;
        case AST_VAR_CREATE_NUMBER_DICT_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_DICT, ast_node->strings[1], K_NUMBER);
            break;
        case AST_VAR_CREATE_NUMBER_DICT_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[1], NULL, K_NUMBER);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[2], ast_node->strings[1], K_NUMBER);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_STRING_LIST:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_STRING);
            break;
        case AST_VAR_CREATE_STRING_LIST_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_LIST, ast_node->strings[1], K_STRING);
            break;
        case AST_VAR_CREATE_STRING_LIST_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[1], NULL, K_STRING);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_LIST, ast_node->strings[2], ast_node->strings[1], K_STRING);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_CREATE_STRING_DICT:
            reverseComplexMode();
            finishComplexMode(ast_node->strings[0], K_STRING);
            break;
        case AST_VAR_CREATE_STRING_DICT_VAR:
            createCloneFromSymbolByName(ast_node->strings[0], K_DICT, ast_node->strings[1], K_STRING);
            break;
        case AST_VAR_CREATE_STRING_DICT_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[1], NULL, K_STRING);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); createCloneFromFunctionReturn(ast_node->strings[0], K_DICT, ast_node->strings[2], ast_node->strings[1], K_STRING);
                    break;
                default:
                    break;
            }
            break;
        case AST_VAR_UPDATE_BOOL:
            updateSymbolBool(ast_node->strings[0], ast_node->right->value.b);
            break;
        case AST_VAR_UPDATE_NUMBER:
            if (ast_node->right->value_type == V_INT) {
                updateSymbolInt(ast_node->strings[0], ast_node->right->value.i);
            } else {
                updateSymbolFloat(ast_node->strings[0], ast_node->right->value.f);
            }
            break;
        case AST_VAR_UPDATE_STRING:
            updateSymbolString(ast_node->strings[0], ast_node->value.s);
            break;
        case AST_VAR_UPDATE_LIST:
            reverseComplexMode();
            finishComplexModeWithUpdate(ast_node->strings[0]);
            break;
        case AST_VAR_UPDATE_DICT:
            reverseComplexMode();
            finishComplexModeWithUpdate(ast_node->strings[0]);
            break;
        case AST_VAR_UPDATE_VAR:
            updateSymbolByClonningName(ast_node->strings[0], ast_node->strings[1]);
            break;
        case AST_VAR_UPDATE_VAR_EL:
            updateSymbolByClonningComplexElement(ast_node->strings[0], ast_node->strings[1]);
            break;
        case AST_VAR_UPDATE_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 2:
                    callFunction(ast_node->strings[1], NULL); updateSymbolByClonningFunctionReturn(ast_node->strings[0], ast_node->strings[1], NULL);
                    break;
                case 3:
                    callFunction(ast_node->strings[2], ast_node->strings[1]); updateSymbolByClonningFunctionReturn(ast_node->strings[0], ast_node->strings[2], ast_node->strings[1]);
                    break;
                default:
                    break;
            }
            break;
        case AST_RETURN_VAR:
            returnSymbol(ast_node->strings[0]);
            break;
        case AST_PRINT_COMPLEX_EL:
            printSymbolValueEndWithNewLine(getComplexElementBySymbolId(variable_complex_element, variable_complex_element_symbol_id), false, false);
            break;
        case AST_COMPLEX_EL_UPDATE_BOOL:
            updateComplexElementBool(ast_node->right->value.b);
            break;
        case AST_COMPLEX_EL_UPDATE_NUMBER:
            if (ast_node->right->value_type == V_INT) {
                updateComplexElementInt(ast_node->right->value.i);
            } else {
                updateComplexElementFloat(ast_node->right->value.f);
            }
            break;
        case AST_COMPLEX_EL_UPDATE_STRING:
            updateComplexElementString(ast_node->value.s);
            break;
        case AST_COMPLEX_EL_UPDATE_LIST:
            reverseComplexMode();
            updateComplexElementComplex();
            break;
        case AST_COMPLEX_EL_UPDATE_DICT:
            reverseComplexMode();
            updateComplexElementComplex();
            break;
        case AST_COMPLEX_EL_UPDATE_VAR:
            updateComplexElementSymbol(getSymbol(ast_node->strings[0]));
            break;
        case AST_COMPLEX_EL_UPDATE_VAR_EL:
            updateComplexElementSymbol(getComplexElementThroughLeftRightBracketStack(ast_node->strings[0], 0));
            break;
        case AST_COMPLEX_EL_UPDATE_FUNC_RETURN:
            switch (ast_node->strings_size)
            {
                case 1:
                    callFunction(ast_node->strings[0], NULL); updateComplexSymbolByClonningFunctionReturn(ast_node->strings[0], NULL);
                    break;
                case 2:
                    callFunction(ast_node->strings[1], ast_node->strings[0]); updateComplexSymbolByClonningFunctionReturn(ast_node->strings[1], ast_node->strings[0]);
                    break;
                default:
                    break;
            }
            break;
        case AST_PRINT_VAR:
            printSymbolValueEndWithNewLine(getSymbol(ast_node->strings[0]), false, true);
            break;
        case AST_PRINT_VAR_EL:
            printSymbolValueEndWithNewLine(getComplexElementThroughLeftRightBracketStack(ast_node->strings[0], 0), false, true);
            break;
        case AST_PRINT_EXPRESSION:
            printf("%lld\n", ast_node->right->value.i);
            break;
        case AST_PRINT_MIXED_EXPRESSION:
            printf("%Lg\n", ast_node->right->value.f);
            break;
        case AST_PRINT_STRING:
            printf("%s\n", ast_node->value.s);
            break;
        case AST_PRINT_INTERACTIVE_VAR:
            if (ast_node->strings[0][0] != '\0' && is_interactive)
                printSymbolValueEndWithNewLine(getSymbol(ast_node->strings[0]), false, false);
            break;
        case AST_PRINT_INTERACTIVE_EXPRESSION:
            if (is_interactive)
                printf("%lld\n", ast_node->right->value.i);
            break;
        case AST_PRINT_INTERACTIVE_MIXED_EXPRESSION:
            if (is_interactive)
                printf("%Lg\n", ast_node->right->value.f);
            break;
        case AST_ECHO_VAR:
            printSymbolValueEndWith(getSymbol(ast_node->strings[0]), "", false, true);
            break;
        case AST_ECHO_VAR_EL:
            printSymbolValueEndWith(getComplexElementThroughLeftRightBracketStack(ast_node->strings[0], 0), "", false, true);
            break;
        case AST_ECHO_EXPRESSION:
            printf("%lld", ast_node->right->value.i);
            break;
        case AST_ECHO_MIXED_EXPRESSION:
            printf("%Lg", ast_node->right->value.f);
            break;
        case AST_ECHO_STRING:
            printf("%s", ast_node->value.s);
            break;
        case AST_PRETTY_PRINT_VAR:
            printSymbolValueEndWithNewLine(getSymbol(ast_node->strings[0]), true, true);
            break;
        case AST_PRETTY_PRINT_VAR_EL:
            printSymbolValueEndWithNewLine(getComplexElementThroughLeftRightBracketStack(ast_node->strings[0], 0), true, true);
            break;
        case AST_PRETTY_ECHO_VAR:
            printSymbolValueEndWith(getSymbol(ast_node->strings[0]), "", true, true);
            break;
        case AST_PRETTY_ECHO_VAR_EL:
            printSymbolValueEndWith(getComplexElementThroughLeftRightBracketStack(ast_node->strings[0], 0), "", true, true);
            break;
        case AST_EXPRESSION_PLUS:
            ast_node->value.i = ast_node->left->value.i + ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_MINUS:
            ast_node->value.i = ast_node->left->value.i  - ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_MULTIPLY:
            ast_node->value.i = ast_node->left->value.i * ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_BITWISE_AND:
            ast_node->value.i = ast_node->left->value.i & ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_BITWISE_OR:
            ast_node->value.i = ast_node->left->value.i | ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_BITWISE_XOR:
            ast_node->value.i = ast_node->left->value.i ^ ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_BITWISE_NOT:
            ast_node->value.i = ~ ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_BITWISE_LEFT_SHIFT:
            ast_node->value.i = ast_node->left->value.i << ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_EXPRESSION_BITWISE_RIGHT_SHIFT:
            ast_node->value.i = ast_node->left->value.i >> ast_node->right->value.i;
            ast_node->value_type = V_INT;
            break;
        case AST_VAR_EXPRESSION_VALUE:
            ast_node->value.i = getSymbolValueInt(ast_node->strings[0]);
            ast_node->value_type = V_INT;
            break;
        case AST_VAR_EXPRESSION_INCREMENT:
            ast_node->value.i = ast_node->right->value.i + 1;
            ast_node->value_type = V_INT;
            break;
        case AST_VAR_EXPRESSION_DECREMENT:
            ast_node->value.i = ast_node->right->value.i - 1;
            ast_node->value_type = V_INT;
            break;
        case AST_VAR_EXPRESSION_INCREMENT_ASSIGN:
            ast_node->value.i = incrementThenAssign(ast_node->strings[0], ast_node->value.i);
            ast_node->value_type = V_INT;
            break;
        case AST_VAR_EXPRESSION_ASSIGN_INCREMENT:
            ast_node->value.i = assignThenIncrement(ast_node->strings[0], ast_node->value.i);
            ast_node->value_type = V_INT;
            break;
        case AST_MIXED_EXPRESSION_PLUS:
            if (ast_node->left->value_type == V_INT) {
                l_value = (long double) ast_node->left->value.i;
            } else {
                l_value = ast_node->left->value.f;
            }
            if (ast_node->right->value_type == V_INT) {
                r_value = (long double) ast_node->right->value.i;
            } else {
                r_value = ast_node->right->value.f;
            }
            ast_node->value.f = l_value + r_value;
            ast_node->value_type = V_FLOAT;
            break;
        case AST_MIXED_EXPRESSION_MINUS:
            if (ast_node->left->value_type == V_INT) {
                l_value = (long double) ast_node->left->value.i;
            } else {
                l_value = ast_node->left->value.f;
            }
            if (ast_node->right->value_type == V_INT) {
                r_value = (long double) ast_node->right->value.i;
            } else {
                r_value = ast_node->right->value.f;
            }
            ast_node->value.f = l_value - r_value;
            ast_node->value_type = V_FLOAT;
            break;
        case AST_MIXED_EXPRESSION_MULTIPLY:
            if (ast_node->left->value_type == V_INT) {
                l_value = (long double) ast_node->left->value.i;
            } else {
                l_value = ast_node->left->value.f;
            }
            if (ast_node->right->value_type == V_INT) {
                r_value = (long double) ast_node->right->value.i;
            } else {
                r_value = ast_node->right->value.f;
            }
            ast_node->value.f = l_value * r_value;
            ast_node->value_type = V_FLOAT;
            break;
        case AST_MIXED_EXPRESSION_DIVIDE:
            if (ast_node->left->value_type == V_INT) {
                l_value = (long double) ast_node->left->value.i;
            } else {
                l_value = ast_node->left->value.f;
            }
            if (ast_node->right->value_type == V_INT) {
                r_value = (long double) ast_node->right->value.i;
            } else {
                r_value = ast_node->right->value.f;
            }
            ast_node->value.f = l_value / r_value;
            ast_node->value_type = V_FLOAT;
            break;
        case AST_VAR_MIXED_EXPRESSION_VALUE:
            ast_node->value.f = getSymbolValueFloat(ast_node->strings[0]);
            ast_node->value_type = V_FLOAT;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL:
            ast_node->value.b = ast_node->left->value.b == ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL:
            ast_node->value.b = ast_node->left->value.b != ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT:
            ast_node->value.b = ast_node->left->value.b > ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL:
            ast_node->value.b = ast_node->left->value.b < ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL:
            ast_node->value.b = ast_node->left->value.b >= ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL:
            ast_node->value.b = ast_node->left->value.b < ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND:
            ast_node->value.b = ast_node->left->value.b && ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR:
            ast_node->value.b = ast_node->left->value.b || ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_NOT:
            ast_node->value.b = ! ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED:
            ast_node->value.b = ast_node->left->value.f == ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED:
            ast_node->value.b = ast_node->left->value.f != ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED:
            ast_node->value.b = ast_node->left->value.f > ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED:
            ast_node->value.b = ast_node->left->value.f < ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED:
            ast_node->value.b = ast_node->left->value.f >= ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED:
            ast_node->value.b = ast_node->left->value.f < ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED:
            ast_node->value.b = ast_node->left->value.f && ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED:
            ast_node->value.b = ast_node->left->value.f || ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_NOT_MIXED:
            ast_node->value.b = ! ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f != ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f > ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f < ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f >= ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f < ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f && ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED_BOOLEAN:
            ast_node->value.b = ast_node->left->value.f || ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b == ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b != ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b > ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b < ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b >= ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b < ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b && ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_BOOLEAN_MIXED:
            ast_node->value.b = ast_node->left->value.b || ast_node->right->value.f;
            break;
        case AST_VAR_BOOLEAN_EXPRESSION_VALUE:
            ast_node->value.b = getSymbolValueBool(ast_node->strings[0]);
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP:
            ast_node->value.b = ast_node->left->value.i == ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP:
            ast_node->value.b = ast_node->left->value.i != ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP:
            ast_node->value.b = ast_node->left->value.i > ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP:
            ast_node->value.b = ast_node->left->value.i < ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP:
            ast_node->value.b = ast_node->left->value.i >= ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP:
            ast_node->value.b = ast_node->left->value.i < ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP:
            ast_node->value.b = ast_node->left->value.i && ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP:
            ast_node->value.b = ast_node->left->value.i || ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_NOT_EXP:
            ast_node->value.b = ! ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i == ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i != ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i > ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i < ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i >= ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i < ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i && ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP_BOOLEAN:
            ast_node->value.b = ast_node->left->value.i || ast_node->right->value.b;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b == ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b != ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b > ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b < ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b >= ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b < ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b && ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_BOOLEAN_EXP:
            ast_node->value.b = ast_node->left->value.b || ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f == ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f != ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f > ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f < ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f >= ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f < ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f && ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_MIXED_EXP:
            ast_node->value.b = ast_node->left->value.f || ast_node->right->value.i;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_EQUAL_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i == ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_NOT_EQUAL_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i != ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i > ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i < ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_GREAT_EQUAL_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i >= ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_REL_SMALL_EQUAL_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i < ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_AND_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i && ast_node->right->value.f;
            break;
        case AST_BOOLEAN_EXPRESSION_LOGIC_OR_EXP_MIXED:
            ast_node->value.b = ast_node->left->value.i || ast_node->right->value.f;
            break;
        case AST_DELETE_VAR:
            removeSymbolByName(ast_node->strings[0]);
            break;
        case AST_DELETE_VAR_EL:
            removeComplexElementByLeftRightBracketStack(ast_node->strings[0]);
            break;
        case AST_PRINT_SYMBOL_TABLE:
            printSymbolTable();
            break;
        case AST_LIST_START:
            addSymbolList(NULL);
            break;
        case AST_LIST_ADD_VAR:
            cloneSymbolToComplex(ast_node->strings[0], NULL);
            break;
        case AST_LIST_ADD_VAR_EL:
            buildVariableComplexElement(ast_node->strings[0], NULL);
            break;
        case AST_LIST_NESTED_FINISH:
            if (isNestedComplexMode()) {
                pushNestedComplexModeStack(getComplexMode());
                reverseComplexMode();
                finishComplexMode(NULL, K_ANY);
            }
            break;
        case AST_DICT_START:
            addSymbolDict(NULL);
            break;
        case AST_DICT_ADD_VAR:
            cloneSymbolToComplex(ast_node->strings[1], ast_node->strings[0]);
            break;
        case AST_DICT_ADD_VAR_EL:
            buildVariableComplexElement(ast_node->strings[1], ast_node->strings[0]);
            break;
        case AST_DICT_NESTED_FINISH:
            if (isNestedComplexMode()) {
                pushNestedComplexModeStack(getComplexMode());
                reverseComplexMode();
                finishComplexMode(NULL, K_ANY);
            }
            break;
        case AST_POP_NESTED_COMPLEX_STACK:
            popNestedComplexModeStack(ast_node->strings[0]);
            break;
        case AST_LEFT_RIGHT_BRACKET_EXPRESSION:
            disable_complex_mode = true;
            if (ast_node->right->value_type == V_INT) {
                symbol = addSymbolInt(NULL, ast_node->right->value.i);
            } else {
                symbol = addSymbolFloat(NULL, ast_node->right->value.f);
            }
            symbol->sign = 1;
            pushLeftRightBracketStack(symbol->id);
            disable_complex_mode = false;
            break;
        case AST_LEFT_RIGHT_BRACKET_MINUS_EXPRESSION:
            disable_complex_mode = true;
            if (ast_node->right->value_type == V_INT) {
                symbol = addSymbolInt(NULL, - ast_node->right->value.i);
            } else {
                symbol = addSymbolFloat(NULL, - ast_node->right->value.f);
            }
            symbol->sign = 1;
            pushLeftRightBracketStack(symbol->id);
            disable_complex_mode = false;
            break;
        case AST_LEFT_RIGHT_BRACKET_STRING:
            disable_complex_mode = true;
            symbol = addSymbolString(NULL, ast_node->value.s);
            symbol->sign = 1;
            pushLeftRightBracketStack(symbol->id);
            disable_complex_mode = false;
            break;
        case AST_LEFT_RIGHT_BRACKET_VAR:
            disable_complex_mode = true;
            symbol = createCloneFromSymbolByName(NULL, K_ANY, ast_node->strings[0], K_ANY);
            symbol->sign = 1;
            pushLeftRightBracketStack(symbol->id);
            disable_complex_mode = false;
            break;
        case AST_LEFT_RIGHT_BRACKET_VAR_MINUS:
            disable_complex_mode = true;
            symbol = createCloneFromSymbolByName(NULL, K_ANY, ast_node->strings[0], K_ANY);
            symbol->sign = -1;
            pushLeftRightBracketStack(symbol->id);
            disable_complex_mode = false;
            break;
        case AST_BUILD_COMPLEX_VARIABLE:
            disable_complex_mode = true;
            buildVariableComplexElement(ast_node->strings[0], NULL);
            disable_complex_mode = false;
            break;
        case AST_EXIT_SUCCESS:
            if (is_interactive) {
                print_bye_bye();
            }
            freeEverything();
            exit(E_SUCCESS);
            break;
        case AST_EXIT_EXPRESSION:
            if (is_interactive) {
                print_bye_bye();
            }
            exit_code = ast_node->right->value.i;
            freeEverything();
            exit(exit_code);
            break;
        case AST_EXIT_VAR:
            if (is_interactive) {
                print_bye_bye();
            }
            exit_code = getSymbolValueInt(ast_node->strings[0]);
            freeEverything();
            exit(exit_code);
            break;
        case AST_PRINT_FUNCTION_TABLE:
            printFunctionTable();
            break;
        case AST_FUNCTION_CALL_PARAMETERS_START:
            //startFunctionParameters();
            break;
        case AST_FUNCTION_CALL_PARAMETER_BOOL:
            addFunctionCallParameterBool(ast_node->right->value.b);
            break;
        case AST_FUNCTION_CALL_PARAMETER_NUMBER:
            if (ast_node->right->value_type == V_INT) {
                addFunctionCallParameterInt(ast_node->right->value.i);
            } else {
                addFunctionCallParameterFloat(ast_node->right->value.f);
            }
            break;
        case AST_FUNCTION_CALL_PARAMETER_STRING:
            addFunctionCallParameterString(ast_node->value.s);
            break;
        case AST_FUNCTION_CALL_PARAMETER_VAR:
            addFunctionCallParameterSymbol(ast_node->strings[0]);
            break;
        case AST_FUNCTION_CALL_PARAMETER_LIST:
            reverseComplexMode();
            addFunctionCallParameterList(K_ANY);
            break;
        case AST_FUNCTION_CALL_PARAMETER_DICT:
            reverseComplexMode();
            addFunctionCallParameterList(K_ANY);
            break;
        case AST_PRINT_FUNCTION_RETURN:
            callFunction(ast_node->strings[0], ast_node->strings[1]);
            printFunctionReturn(ast_node->strings[0], ast_node->strings[1], "\n", false, true);
            break;
        case AST_ECHO_FUNCTION_RETURN:
            callFunction(ast_node->strings[0], ast_node->strings[1]);
            printFunctionReturn(ast_node->strings[0], ast_node->strings[1], "", false, true);
            break;
        case AST_PRETTY_PRINT_FUNCTION_RETURN:
            callFunction(ast_node->strings[0], ast_node->strings[1]);
            printFunctionReturn(ast_node->strings[0], ast_node->strings[1], "\n", true, true);
            break;
        case AST_PRETTY_ECHO_FUNCTION_RETURN:
            callFunction(ast_node->strings[0], ast_node->strings[1]);
            printFunctionReturn(ast_node->strings[0], ast_node->strings[1], "", true, true);
            break;
        case AST_FUNCTION_RETURN:
            callFunction(ast_node->strings[0], ast_node->strings[1]);
            if (is_interactive && !isFunctionType(ast_node->strings[0], ast_node->strings[1], K_VOID) && !inject_mode && !decision_execution_mode)
                printFunctionReturn(ast_node->strings[0], ast_node->strings[1], "\n", false, false);
            break;
        case AST_NESTED_COMPLEX_TRANSITION:
            reverseComplexMode();
            break;
        case AST_DECISION_MAKE_BOOLEAN:
            if (ast_node->right->value.b) {
                callFunction(ast_node->strings[0], executed_function->module);
                stop_ast_evaluation = true;
            }
            break;
        case AST_DECISION_MAKE_DEFAULT:
            if (executed_function != NULL) {
                callFunction(ast_node->strings[0], executed_function->module);
                stop_ast_evaluation = true;
            }
            break;
        default:
            break;
    }

    if (phase == PREPARSE) {
        return ast_node;
    } else {
        return eval_node(ast_node->next, module);
    }
}
