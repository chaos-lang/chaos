#ifndef CHAOS_H
#define CHAOS_H

#include "enums.h"

int defineFunction(char *name, enum Type type);

struct Kaos {
    int (*defineFunction)(char *name, enum Type type);
};

struct Kaos kaos;

#endif
