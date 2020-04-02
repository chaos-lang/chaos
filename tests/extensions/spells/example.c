#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Type { BOOL, NUMBER, STRING, ANY, ARRAY, DICT, VOID };
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
    startFunction(name, VOID);
    endFunction();
    free(name);
}
