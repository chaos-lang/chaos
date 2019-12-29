#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void throw_error(int code, char *subject) {
    printf("%s Error - ", __LANGUAGE_NAME__);
    switch (code)
    {
        case 1:
            printf("Unkown variable type: '%s'", subject);
            break;
        case 2:
            printf("The variable name '%s' is already defined!", subject);
            break;
        case 3:
            printf("Undefined variable: '%s'", subject);
            break;
        case 4:
            printf("Memory allocation for array '%s' failed!", subject);
            break;
        case 5:
            printf("Illegal element type for the typed array: '%s'", subject);
            break;
        case 6:
            printf("Variable %s is not an array!", subject);
            break;
        case 7:
            printf("Undefined index: %s", subject);
            break;
        case 8:
            printf("Illegal variable type for: '%s'", subject);
            break;
        default:
            printf("Unkown error.");
            break;
    }
    printf("\n");

    exit(code);
}
