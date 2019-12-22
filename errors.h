#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void throw_error(int code, char *subject) {
    printf("%s Error - ", __LANGUAGE_NAME__);
    switch (code)
    {
        case 1:
            printf("Unkown variable type: '%s'\n", subject);
            break;
        case 2:
            printf("The variable name '%s' is already defined!\n", subject);
            break;
        case 3:
            printf("Undefined variable: '%s'\n", subject);
            break;
        default:
            printf("Unkown error.");
            break;
    }

    exit(code);
}
