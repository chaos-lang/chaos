#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void throw_error(int code, char *subject) {
    switch (code)
    {
        case '1':
            printf("%s", strcat("Unkown variable type: ", subject));
            break;
        default:
            printf("Unkown error.");
            break;
    }

    exit(code);
}
