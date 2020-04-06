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

char *hello_params_name[] = {};
int hello_params_type[] = {};
int hello_parasm_length = 0;
int EXPORT Kaos_hello()
{
    printf("Hello from example extension!\n");
    return 0;
}

char *add_params_name[] = {
    "x",
    "y"
};
int add_params_type[] = {
    K_NUMBER,
    K_NUMBER
};
int add_params_length = (int) sizeof(add_params_type)/sizeof(int);
int EXPORT Kaos_add()
{
    int x = kaos.getVariableInt(add_params_name[0]);
    int y = kaos.getVariableInt(add_params_name[1]);
    int z = x + y;
    printf("%i\n", z);
    return 0;
}

char *log_params_name[] = {
    "message"
};
int log_params_type[] = {
    K_STRING
};
int log_params_length = (int) sizeof(log_params_type)/sizeof(int);
int EXPORT Kaos_log()
{
    char* var = kaos.getVariableString(log_params_name[0]);
    printf("%s\n", var);
}

int EXPORT KaosRegister(struct Kaos _kaos)
{
    kaos = _kaos;
    kaos.defineFunction("hello", K_VOID, hello_params_name, hello_params_type, hello_parasm_length);
    kaos.defineFunction("add", K_VOID, add_params_name, add_params_type, add_params_length);
    kaos.defineFunction("log", K_VOID, log_params_name, log_params_type, log_params_length);

    return 0;
}
