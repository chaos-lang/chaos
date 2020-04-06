#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../../../chaos.h"

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

int EXPORT Kaos_hello()
{
    printf("Hello from example extension!\n");
    return 0;
}

int EXPORT KaosRegister(struct Kaos kaos)
{
    kaos.defineFunction("hello", K_VOID);
    return 0;
}
