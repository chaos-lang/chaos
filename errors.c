#include "errors.h"

extern int yylineno;
extern char *last_token;

void throw_error_var(throw_error_args in) {
    int code = in.code ? in.code : 0;
    char *str1 = in.str1 ? in.str1 : "";
    char *str2 = in.str2 ? in.str2 : "";
    int int1 = in.int1 ? in.int1 : 0;
    unsigned long long int llu = in.llu1 ? in.llu1 : 0;
    throw_error_base(code, str1, str2, int1, llu);
}

void throw_error_base(int code, char *str1, char *str2, int int1, unsigned long long int llu1) {
    struct winsize terminal;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);

    char title_msg[terminal.ws_col];
    char current_module_msg[terminal.ws_col];
    char line_no_msg[terminal.ws_col];
    char error_msg[terminal.ws_col];
    char error_msg_out[terminal.ws_col];

    if (strcmp(last_token, "\n") != 0) yylineno++;

    sprintf(title_msg, "  %s Error:", __LANGUAGE_NAME__);
    sprintf(current_module_msg, "    Module: %s", getCurrentModule());
    sprintf(line_no_msg, "    Line: %d", yylineno);

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
        case E_MEMORY_ALLOCATION_FOR_ARRAY_FAILED:
            sprintf(error_msg, "Memory allocation for array '%s' is failed!", str1);
            break;
        case E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY:
            sprintf(error_msg, "Illegal element type: %s for the typed array: %s", str1, str2);
            break;
        case E_VARIABLE_IS_NOT_AN_ARRAY:
            sprintf(error_msg, "Variable '%s' is not an array!", str1);
            break;
        case E_UNDEFINED_INDEX:
            sprintf(error_msg, "Undefined index: %i for array: %s", int1, str1);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE:
            sprintf(error_msg, "Illegal variable type: %s, for variable: %s", str1, str2);
            break;
        case E_ARRAYS_ARE_NOT_MASS_ASSIGNABLE:
            sprintf(error_msg, "Arrays are not mass assignable! Target variable: %s", str1);
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
        case E_MAXIMUM_RECURSION_DEPTH_EXCEEDED:
            sprintf(error_msg, "Maximum recursion depth %i exceeded!", __MAX_RECURSION_DEPTH__);
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
        default:
            sprintf(error_msg, "Unkown error.");
            break;
    }

    sprintf(error_msg_out, "    %s", error_msg);

    int cols[4];
    cols[0] = (int) strlen(title_msg);
    cols[1] = (int) strlen(current_module_msg);
    cols[2] = (int) strlen(line_no_msg);
    cols[3] = (int) strlen(error_msg_out);
    int ws_col = largest(cols, 4) + 4;
    InteractiveShellErrorAbsorber_ws_col = ws_col;

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;41m");
    #endif
    printf("%-*s", ws_col, title_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;41m");
    #endif
    printf("%-*s", ws_col, current_module_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;41m");
    #endif
    printf("%-*s", ws_col, line_no_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;41m");
    #endif
    printf("%-*s", ws_col, error_msg_out);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    if (strlen(str1) > 0) free(str1);
    if (strlen(str2) > 0) free(str2);
    if (!is_interactive) {
        freeEverything();
        exit(code);
    } else {
        longjmp(InteractiveShellErrorAbsorber, 1);
    }
}
