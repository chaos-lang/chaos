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
int hello_params_length = 0;
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
    kaos.returnVariableInt(z);
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

char *complex_params_name[] = {
    "arr1",
    "dict1"
};
int complex_params_type[] = {
    K_ARRAY,
    K_DICT
};
int complex_params_length = (int) sizeof(complex_params_type)/sizeof(int);
int EXPORT Kaos_complex()
{
    int var1 = (int) kaos.getArrayElementFloat(complex_params_name[0], 0);
    printf("%i\n", var1);
    char* var2 = kaos.getDictElementString(complex_params_name[1], "a");
    printf("%s\n", var2);
}

char *mayhem_params_name[] = {};
int mayhem_params_type[] = {};
int mayhem_params_length = 0;
int EXPORT Kaos_mayhem()
{
    kaos.startBuildingDict();
    kaos.createVariableBool("b", true);
    kaos.createVariableInt("i", 1);
    kaos.createVariableFloat("f", 3.14);
    kaos.createVariableString("s", "bar");
    kaos.returnDict(K_ANY);
    return 0;
}

int EXPORT KaosRegister(struct Kaos _kaos)
{
    kaos = _kaos;
    kaos.defineFunction("hello", K_VOID, hello_params_name, hello_params_type, hello_params_length);
    kaos.defineFunction("add", K_NUMBER, add_params_name, add_params_type, add_params_length);
    kaos.defineFunction("log", K_VOID, log_params_name, log_params_type, log_params_length);
    kaos.defineFunction("complex", K_VOID, complex_params_name, complex_params_type, complex_params_length);
    kaos.defineFunction("mayhem", K_DICT, mayhem_params_name, mayhem_params_type, mayhem_params_length);

    return 0;
}
