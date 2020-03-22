#include "errors.h"
#include "utilities/language.h"

void throw_error(int code, char *subject) {
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;41m");
    #endif
    printf(" %s Error - ", __LANGUAGE_NAME__);
    switch (code)
    {
        case E_UNKNOWN_VARIABLE_TYPE:
            printf("Unknown variable type: %s", subject);
            break;
        case E_VARIABLE_ALREADY_DEFINED:
            printf("The variable name '%s' is already defined!", subject);
            break;
        case E_UNDEFINED_VARIABLE:
            printf("Undefined variable: %s", subject);
            break;
        case E_MEMORY_ALLOCATION_FOR_ARRAY_FAILED:
            printf("Memory allocation for array '%s' is failed!", subject);
            break;
        case E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY:
            printf("Illegal element type for the typed array: %s", subject);
            break;
        case E_VARIABLE_IS_NOT_AN_ARRAY:
            printf("Variable '%s' is not an array!", subject);
            break;
        case E_UNDEFINED_INDEX:
            printf("Undefined index: %s", subject);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE:
            printf("Illegal variable type for variable: %s", subject);
            break;
        case E_ARRAYS_ARE_NOT_MASS_ASSIGNABLE:
            printf("Arrays are not mass assignable!");
            break;
        case E_VARIABLE_IS_NOT_A_DICTIONARY:
            printf("Variable '%s' is not a dictionary!", subject);
            break;
        case E_UNDEFINED_KEY:
            printf("Undefined key: %s", subject);
            break;
        case E_UNRECOGNIZED_COMPLEX_DATA_TYPE:
            printf("Unrecognized complex data type for: %s", subject);
            break;
        case E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION:
            printf("Illegal variable type for function: %s", subject);
            break;
        case E_UNDEFINED_FUNCTION:
            printf("Undefined function: %s", subject);
            break;
        case E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED:
            printf("Memory allocation for the function is failed!");
            break;
        case E_MAXIMUM_RECURSION_DEPTH_EXCEEDED:
            printf("Maximum recursion depth %i exceeded!", __MAX_RECURSION_DEPTH__);
            break;
        case E_UNEXPECTED_VALUE_TYPE:
            printf("Unexpected value type: %s", subject);
            break;
        case E_FUNCTION_DID_NOT_RETURN_ANYTHING:
            printf("The function '%s' did not return anything!", subject);
            break;
        case E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH:
            printf("The module %s is either empty or not exists on the path!", subject);
            break;
        case E_NO_VARIABLE_WITH_ID:
            printf("No variable with given id is found!");
            break;
        default:
            printf("Unkown error.");
            break;
    }
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf(" \033[0m");
    #endif
    printf("\n");

    free(subject);
    if (!is_interactive) {
        freeEverything();
        exit(code);
    } else {
        longjmp(InteractiveShellErrorAbsorber, 1);
    }
}
