#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "function.h"
#include "chaos.h"

int defineFunction(char *name, enum Type type) {
    char *_name = malloc(strlen(name) + 1);
    strcpy(_name, name);
    startFunctionParameters();
    startFunction(_name, type);
    endFunction();
    return 0;
}
