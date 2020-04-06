#ifndef CHAOS_H
#define CHAOS_H

#include "enums.h"

int defineFunction(char *name, enum Type type, char *params_name[], int params_type[], int params_length);
bool getVariableBool(char *name);
int getVariableInt(char *name);
float getVariableFloat(char *name);
char* getVariableString(char *name);

struct Kaos {
    int (*defineFunction)(char *name, enum Type type, char *params_name[], int params_type[], int params_length);
    bool (*getVariableBool)(char *name);
    int (*getVariableInt)(char *name);
    float (*getVariableFloat)(char *name);
    char* (*getVariableString)(char *name);
};

struct Kaos kaos;

#endif
