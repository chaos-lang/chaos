#include "errors.h"
#include "utilities/language.h"

void throw_error_var(throw_error_args in) {
    int code = in.code ? in.code : 0;
    char *str1 = in.str1 ? in.str1 : "";
    char *str2 = in.str2 ? in.str2 : "";
    int int1 = in.int1 ? in.int1 : 0;
    unsigned long long int llu = in.llu1 ? in.llu1 : 0;
    throw_error_base(code, str1, str2, int1, llu);
}

void throw_error_base(int code, char *str1, char *str2, int int1, unsigned long long int llu1) {
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;41m");
    #endif
    printf(" %s Error - ", __LANGUAGE_NAME__);
    switch (code)
    {
        case E_UNKNOWN_VARIABLE_TYPE:
            printf("Unknown variable type: %s for variable: %s", str1, str2);
            break;
        case E_VARIABLE_ALREADY_DEFINED:
            printf("The variable name '%s' is already defined!", str1);
            break;
        case E_UNDEFINED_VARIABLE:
            printf("Undefined variable: %s", str1);
            break;
        case E_MEMORY_ALLOCATION_FOR_ARRAY_FAILED:
            printf("Memory allocation for array '%s' is failed!", str1);
            break;
        case E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY:
            printf("Illegal element type: %s for the typed array: %s", str1, str2);
            break;
        case E_VARIABLE_IS_NOT_AN_ARRAY:
            printf("Variable '%s' is not an array!", str1);
            break;
        case E_UNDEFINED_INDEX:
            printf("Undefined index: %i for array: %s", int1, str1);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE:
            printf("Illegal variable type: %s, for variable: %s", str1, str2);
            break;
        case E_ARRAYS_ARE_NOT_MASS_ASSIGNABLE:
            printf("Arrays are not mass assignable! Target variable: %s", str1);
            break;
        case E_VARIABLE_IS_NOT_A_DICTIONARY:
            printf("Variable '%s' is not a dictionary!", str1);
            break;
        case E_UNDEFINED_KEY:
            printf("Undefined key: %s for dictionary: %s", str1, str2);
            break;
        case E_UNRECOGNIZED_COMPLEX_DATA_TYPE:
            printf("Unrecognized complex data type: %s for variable: %s", str1, str2);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION:
            printf("Illegal variable type: %s for function: %s", str1, str2);
            break;
        case E_UNDEFINED_FUNCTION:
            printf("Undefined function: %s", str1);
            break;
        case E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED:
            printf("Memory allocation for the function is failed!");
            break;
        case E_MAXIMUM_RECURSION_DEPTH_EXCEEDED:
            printf("Maximum recursion depth %i exceeded!", __MAX_RECURSION_DEPTH__);
            break;
        case E_UNEXPECTED_VALUE_TYPE:
            printf("Unexpected value type: %s for variable: %s", str1, str2);
            break;
        case E_FUNCTION_DID_NOT_RETURN_ANYTHING:
            printf("The function '%s' did not return anything!", str1);
            break;
        case E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH:
            printf("The module %s is either empty or not exists on the path!", str1);
            break;
        case E_NO_VARIABLE_WITH_ID:
            printf("No variable with given id: %llu is found!", llu1);
            break;
        default:
            printf("Unkown error.");
            break;
    }
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf(" \033[0m");
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
