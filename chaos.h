#ifndef CHAOS_H
#define CHAOS_H

#include "enums.h"

int defineFunction(char *name, enum Type type, char *params_name[], int params_type[], int params_length);
bool getVariableBool(char *name);
int getVariableInt(char *name);
float getVariableFloat(char *name);
char* getVariableString(char *name);
bool getArrayElementBool(char *name, int i);
int getArrayElementInt(char *name, int i);
float getArrayElementFloat(char *name, int i);
char* getArrayElementString(char *name, int i);
bool getDictElementBool(char *name, char *key);
int getDictElementInt(char *name, char *key);
float getDictElementFloat(char *name, char *key);
char* getDictElementString(char *name, char *key);

struct Kaos {
    int (*defineFunction)(char *name, enum Type type, char *params_name[], int params_type[], int params_length);
    bool (*getVariableBool)(char *name);
    int (*getVariableInt)(char *name);
    float (*getVariableFloat)(char *name);
    char* (*getVariableString)(char *name);
    bool (*getArrayElementBool)(char *name, int i);
    int (*getArrayElementInt)(char *name, int i);
    float (*getArrayElementFloat)(char *name, int i);
    char* (*getArrayElementString)(char *name, int i);
    bool (*getDictElementBool)(char *name, char *key);
    int (*getDictElementInt)(char *name, char *key);
    float (*getDictElementFloat)(char *name, char *key);
    char* (*getDictElementString)(char *name, char *key);
};

struct Kaos kaos;

#endif
