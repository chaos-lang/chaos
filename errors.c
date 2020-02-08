#include "errors.h"
#include "utilities/language.h"

void throw_error(int code, char *subject) {
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;41m");
    #endif
    printf(" %s Error - ", __LANGUAGE_NAME__);
    switch (code)
    {
        case 2:
            printf("Unkown variable type: %s", subject);
            break;
        case 3:
            printf("The variable name '%s' is already defined!", subject);
            break;
        case 4:
            printf("Undefined variable: %s", subject);
            break;
        case 5:
            printf("Memory allocation for array '%s' is failed!", subject);
            break;
        case 6:
            printf("Illegal element type for the typed array: %s", subject);
            break;
        case 7:
            printf("Variable '%s' is not an array!", subject);
            break;
        case 8:
            printf("Undefined index: %s", subject);
            break;
        case 9:
            printf("Illegal variable type for variable: %s", subject);
            break;
        case 10:
            printf("Arrays are not mass assignable!");
            break;
        case 11:
            printf("Variable '%s' is not a dictionary!", subject);
            break;
        case 12:
            printf("Undefined key: %s", subject);
            break;
        case 13:
            printf("Unrecognized complex data type for: %s", subject);
            break;
        case 14:
            printf("Illegal variable type for function: %s", subject);
            break;
        case 15:
            printf("Undefined function: %s", subject);
            break;
        case 16:
            printf("Memory allocation for the function is failed!");
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
