#include "../../../../Chaos.h"

char *hello_params_name[] = {};
unsigned hello_params_type[] = {};
unsigned short hello_params_length = 0;
int KAOS_EXPORT Kaos_hello()
{
    printf("Hello from example extension!\n");
    return 0;
}

char *add_params_name[] = {
    "x",
    "y"
};
unsigned add_params_type[] = {
    K_NUMBER,
    K_NUMBER
};
unsigned short add_params_length = (unsigned short) sizeof(add_params_type) / sizeof(unsigned);
int KAOS_EXPORT Kaos_add()
{
    long long x = kaos.getVariableInt(add_params_name[0]);
    long long y = kaos.getVariableInt(add_params_name[1]);
    long long z = x + y;
    kaos.returnVariableInt(z);
    return 0;
}

char *log_params_name[] = {
    "message"
};
unsigned log_params_type[] = {
    K_STRING
};
unsigned short log_params_length = (unsigned short) sizeof(log_params_type) / sizeof(unsigned);
int KAOS_EXPORT Kaos_log()
{
    char* var = kaos.getVariableString(log_params_name[0]);
    printf("%s\n", var);
}

char *complex_params_name[] = {
    "arr1",
    "dict1"
};
unsigned complex_params_type[] = {
    K_ARRAY,
    K_DICT
};
unsigned short complex_params_length = (unsigned short) sizeof(complex_params_type) / sizeof(unsigned);
int KAOS_EXPORT Kaos_complex()
{
    long long var1 = (long long) kaos.getArrayElementFloat(complex_params_name[0], 0);
    printf("%lld\n", var1);
    char* var2 = kaos.getDictElementString(complex_params_name[1], "a");
    printf("%s\n", var2);
}

char *mayhem_params_name[] = {};
unsigned mayhem_params_type[] = {};
unsigned short mayhem_params_length = 0;
int KAOS_EXPORT Kaos_mayhem()
{
    kaos.startBuildingDict();
    kaos.createVariableBool("b", true);
    kaos.createVariableInt("i", 1);
    kaos.createVariableFloat("f", 3.14);
    kaos.createVariableString("s", "bar");
    kaos.returnDict(K_ANY);
    return 0;
}

int KAOS_EXPORT KaosRegister(struct Kaos _kaos)
{
    kaos = _kaos;
    kaos.defineFunction("hello", K_VOID, hello_params_name, hello_params_type, hello_params_length);
    kaos.defineFunction("add", K_NUMBER, add_params_name, add_params_type, add_params_length);
    kaos.defineFunction("log", K_VOID, log_params_name, log_params_type, log_params_length);
    kaos.defineFunction("complex", K_VOID, complex_params_name, complex_params_type, complex_params_length);
    kaos.defineFunction("mayhem", K_DICT, mayhem_params_name, mayhem_params_type, mayhem_params_length);

    return 0;
}
