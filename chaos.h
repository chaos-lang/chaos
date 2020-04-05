#ifndef CHAOS_H
#define CHAOS_H

#include "enums.h"

struct Kaos {
    void (*startFunction)(char *name, enum Type type);
    void (*endFunction)();
    void (*startFunctionParameters)();
};

struct Kaos kaos;

#endif