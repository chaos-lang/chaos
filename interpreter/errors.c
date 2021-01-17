/*
 * Description: Errors module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#include "errors.h"

extern int kaos_lineno;
extern FILE* tmp_stdin;

void throw_error_var(throw_error_args in) {
    unsigned short code = in.code ? in.code : 0;
    char *str1 = in.str1 ? in.str1 : "";
    char *str2 = in.str2 ? in.str2 : "";
    long long lld1 = in.lld1 ? in.lld1 : 0;
    unsigned long long llu1 = in.llu1 ? in.llu1 : 0;
    bool is_preemptive = in.is_preemptive ? in.is_preemptive : false;
    _Function* function = in.function ? in.function : NULL;
    throw_error_base(code, str1, str2, lld1, llu1, is_preemptive, function);
}

void throw_error_base(
    unsigned short code,
    char *str1, char *str2,
    long long lld1,
    unsigned long long llu1,
    bool is_preemptive,
    _Function* function
) {
    char title_msg[__KAOS_MSG_LINE_LENGTH__];
    char error_msg[__KAOS_MSG_LINE_LENGTH__];
    char error_msg_out[__KAOS_MSG_LINE_LENGTH__ + 4];
    char bg_color[3];
    int indent = 2;

    if (is_preemptive) {
        sprintf(bg_color, "43");
        sprintf(title_msg, "%*cPreemptive Error:", indent, ' ');
    } else {
        sprintf(bg_color, "41");
        sprintf(title_msg, "%*c%s Error (most recent call last):", indent, ' ', __KAOS_LANGUAGE_NAME__);
    }

    switch (code)
    {
        case E_UNKNOWN_VARIABLE_TYPE:
            sprintf(error_msg, "Unknown variable type: %s for variable: %s", str1, str2);
            break;
        case E_VARIABLE_ALREADY_DEFINED:
            sprintf(error_msg, "The variable name '%s' is already defined!", str1);
            break;
        case E_UNDEFINED_VARIABLE:
            sprintf(error_msg, "Undefined variable: %s", str1);
            break;
        case E_MEMORY_ALLOCATION_FOR_LIST_FAILED:
            sprintf(error_msg, "Memory allocation for list '%s' is failed!", str1);
            break;
        case E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_LIST:
            sprintf(error_msg, "Illegal element type: %s for the typed list: %s", str1, str2);
            break;
        case E_VARIABLE_IS_NOT_AN_LIST:
            sprintf(error_msg, "Variable '%s' is not an list!", str1);
            break;
        case E_INDEX_OUT_OF_RANGE:
            sprintf(error_msg, "Index out of range: %lld for list: %s", lld1, str1);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE:
            sprintf(error_msg, "Illegal variable type: %s, for variable: %s", str1, str2);
            break;
        case E_VARIABLE_IS_NOT_A_DICTIONARY:
            sprintf(error_msg, "Variable '%s' is not a dictionary!", str1);
            break;
        case E_UNDEFINED_KEY:
            sprintf(error_msg, "Undefined key: %s for dictionary: %s", str1, str2);
            break;
        case E_UNRECOGNIZED_COMPLEX_DATA_TYPE:
            sprintf(error_msg, "Unrecognized complex data type: %s for variable: %s", str1, str2);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION:
            sprintf(error_msg, "Illegal variable type: %s for function: %s", str1, str2);
            break;
        case E_UNDEFINED_FUNCTION:
            sprintf(error_msg, "Undefined function: %s", str1);
            break;
        case E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED:
            sprintf(error_msg, "Memory allocation for the function is failed!");
            break;
        case E_UNEXPECTED_VALUE_TYPE:
            sprintf(error_msg, "Unexpected value type: %s for variable: %s", str1, str2);
            break;
        case E_FUNCTION_DID_NOT_RETURN_ANYTHING:
            sprintf(error_msg, "The function '%s' did not return anything!", str1);
            break;
        case E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH:
            sprintf(error_msg, "The module %s is either empty or not exists on the path!", str1);
            break;
        case E_NO_VARIABLE_WITH_ID:
            sprintf(error_msg, "No variable with given id: %llu is found!", llu1);
            break;
        case E_INDEX_OUT_OF_RANGE_STRING:
            sprintf(error_msg, "Index out of range: %lld for string: %s", lld1, str1);
            break;
        case E_ILLEGAL_CHARACTER_ASSIGNMENT_FOR_STRING:
            sprintf(error_msg, "Illegal character assignment for string: %s", str1);
            break;
        case E_NOT_A_CHARACTER:
            sprintf(error_msg, "Right hand side of the assignment is not a character for string: %s", str1);
            break;
        case E_PROGRAM_FILE_DOES_NOT_EXISTS_ON_PATH:
            sprintf(error_msg, "Program file does not exists on the given path: %s", str1);
            break;
        case E_INCORRECT_FUNCTION_ARGUMENT_COUNT:
            sprintf(error_msg, "Incorrect argument count for function: %s", str1);
            break;
        case E_NOT_A_LIST:
            sprintf(error_msg, "'%s' is not a list!", str1);
            break;
        case E_NOT_A_DICT:
            sprintf(error_msg, "'%s' is not a dictionary!", str1);
            break;
        case E_RAISED_FROM_AN_EXTENSION:
            sprintf(error_msg, "%s", str1);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION_PARAMETER:
            sprintf(error_msg, "Illegal variable type for function parameter: %s of function: %s", str1, str2);
            break;
        case E_UNEXPECTED_ACCESSOR_DATA_TYPE:
            sprintf(error_msg, "Unexpected accessor data type: %s for variable: %s", str1, str2);
            break;
        case E_NEGATIVE_ITERATION_COUNT:
            sprintf(error_msg, "Negative iteration count: %lld", lld1);
            break;
        case E_BREAK_CALL_OUTSIDE_LOOP:
            sprintf(error_msg, "Call to a function with `break` from outside a loop: %s", str1);
            break;
        case E_CONTINUE_CALL_OUTSIDE_LOOP:
            sprintf(error_msg, "Call to a function with `continue` from outside a loop: %s", str1);
            break;
        case E_BREAK_CALL_MULTILINE_LOOP:
            sprintf(error_msg, "Call to a function with `break` from a multiline loop: %s", str1);
            break;
        case E_CONTINUE_CALL_MULTILINE_LOOP:
            sprintf(error_msg, "Call to a function with `continue` from a multiline loop: %s", str1);
            break;
        default:
            sprintf(error_msg, "Unkown error.");
            break;
    }

    sprintf(error_msg_out, "  %s", error_msg);

    char* new_error_msg_out = str_replace(error_msg_out, "\n", "\\n");

    unsigned traceback_size = 0;
    if (is_preemptive) {
        traceback_size = 1;
    } else {
        traceback_size = function_call_stack.size + 1;
    }
    int cols[2 + traceback_size * 2];
    char traceback_line_msg[traceback_size][__KAOS_MSG_LINE_LENGTH__];
    char traceback_line[traceback_size][__KAOS_MSG_LINE_LENGTH__];

    FILE* fp_module = NULL;

    if (!is_preemptive) {
        for (unsigned i = 0; i < function_call_stack.size; i++) {
            sprintf(
                traceback_line_msg[i],
                "%*cFile: \"%s\", line %d, in %s",
                indent * 2,
                ' ',
                i != 0 ?
                    function_call_stack.arr[i - 1]->function->module_context
                    :
                    function_call_stack.arr[i]->function->module_context,
                function_call_stack.arr[i]->lineno,
                i != 0 ?
                    function_call_stack.arr[i - 1]->function->name
                    :
                    "<module>"
            );

#ifndef CHAOS_COMPILER
            if (is_interactive &&
                strcmp(
                    module_path_stack.arr[0],
                    i != 0 ?
                        function_call_stack.arr[i - 1]->function->module_context
                        :
                        function_call_stack.arr[i]->function->module_context
                ) == 0
            ) {
                fseek(tmp_stdin, 0, SEEK_SET);
                fp_module = tmp_stdin;
            } else {
#endif
                fp_module = fopen(i != 0 ? function_call_stack.arr[i - 1]->function->module_context : function_call_stack.arr[i]->function->module_context, "r");
#ifndef CHAOS_COMPILER
            }
#endif
            char *line = NULL;
            if (fp_module == NULL) {
                line = malloc(4);
                strcpy(line, "???");
            } else {
                line = get_nth_line(fp_module, function_call_stack.arr[i]->lineno);
#ifndef CHAOS_COMPILER
                if (fp_module != tmp_stdin)
#endif
                    fclose(fp_module);
                if (line == NULL) {
                    line = malloc(4);
                    strcpy(line, "???");
                }
            }
            sprintf(
                traceback_line[i],
                "%*c%s",
                indent * 3,
                ' ',
                trim(line)
            );
            free(line);
        }
    }

    char *function_name = NULL;
    if (is_preemptive) {
        function_name = function->name;
    } else {
        if (function_call_stack.size > 0) {
            FunctionCall* function_call = function_call_stack.arr[function_call_stack.size - 1];
            function_name = function_call->function->name;
        }
    }
    char *module_path = getCurrentModule();
    sprintf(
        traceback_line_msg[traceback_size - 1],
        "%*cFile: \"%s\", line %d, in %s",
        indent * 2,
        ' ',
        module_path,
        kaos_lineno,
        function_name != NULL ? function_name : "<module>"
    );

#ifndef CHAOS_COMPILER
    if (is_interactive && module_path_stack.size < 2) {
        fseek(tmp_stdin, 0, SEEK_SET);
        fp_module = tmp_stdin;
    } else {
#endif
        fp_module = fopen(module_path, "r");
#ifndef CHAOS_COMPILER
    }
#endif
    char *line = NULL;
    if (fp_module == NULL) {
        line = malloc(4);
        strcpy(line, "???");
    } else {
        line = get_nth_line(fp_module, kaos_lineno);
#ifndef CHAOS_COMPILER
        if (fp_module != tmp_stdin)
#endif
            fclose(fp_module);
        if (line == NULL) {
            line = malloc(4);
            strcpy(line, "???");
        }
    }
    sprintf(
        traceback_line[traceback_size - 1],
        "%*c%s",
        indent * 3,
        ' ',
        trim(line)
    );
    free(line);

    cols[0] = (int) strlen(title_msg) + 1;
    unsigned j = 0;
    for (unsigned i = 0; i < traceback_size; i++) {
        cols[j + 1] = (int) strlen(traceback_line_msg[i]) + 1;
        cols[j + 2] = (int) strlen(traceback_line[i]) + 1;
        j++; j++;
    }
    cols[1 + traceback_size * 2] = (int) strlen(new_error_msg_out) + 1;
    int ws_col = largest(cols, (int) (2 + traceback_size * 2)) + 4;
    InteractiveShellErrorAbsorber_ws_col = ws_col;

    fflush(stdout);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;%sm", bg_color);
#endif
    fprintf(stderr, "%-*s", ws_col, title_msg);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");

    for (unsigned i = 0; i < traceback_size; i++) {
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        fprintf(stderr, "\033[0;%sm", bg_color);
#endif
        fprintf(stderr, "%-*s", ws_col, traceback_line_msg[i]);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        fprintf(stderr, "\033[0m");
#endif
        fprintf(stderr, "\n");

#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        fprintf(stderr, "\033[0;%sm", bg_color);
#   endif
        fprintf(stderr, "%-*s", ws_col, traceback_line[i]);
#   if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        fprintf(stderr, "\033[0m");
#   endif
        fprintf(stderr, "\n");
    }

#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[1;%sm", bg_color);
#endif
    fprintf(stderr, "%-*s", ws_col, new_error_msg_out);
#if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
    fprintf(stderr, "\033[0m");
#endif
    fprintf(stderr, "\n");
    fflush(stderr);
    free(new_error_msg_out);

#ifndef CHAOS_COMPILER
    if (is_preemptive && is_interactive)
        removeFunction(end_function);

    if (!is_interactive) {
#endif
        freeEverything();
        exit(code);
#ifndef CHAOS_COMPILER
    } else {
        absorbError();
    }
#endif
}
