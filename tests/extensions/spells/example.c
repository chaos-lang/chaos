#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Type { K_BOOL, K_NUMBER, K_STRING, K_ANY, K_ARRAY, K_DICT, K_VOID };
void startFunction(char *name, enum Type type);
void endFunction();
void startFunctionParameters();

int Kaos_hello()
{
    printf("Hello from example extension!\n");
    return 0;
}

void KaosRegister()
{
    char *name = malloc(6);
    strcpy(name, "hello");
    startFunctionParameters();
    startFunction(name, K_VOID);
    endFunction();
}
