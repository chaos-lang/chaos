#include <stdlib.h>
#include <string.h>

#include "Chaos.h"

char *hello_params_name[] = {};
unsigned hello_params_type[] = {};
unsigned hello_params_secondary_type[] = {};
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
unsigned add_params_secondary_type[] = {
    K_ANY,
    K_ANY
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
unsigned log_params_secondary_type[] = {
    K_ANY
};
unsigned short log_params_length = (unsigned short) sizeof(log_params_type) / sizeof(unsigned);
int KAOS_EXPORT Kaos_log()
{
    char* var = kaos.getVariableString(log_params_name[0]);
    printf("%s\n", var);
    free(var);
    return 0;
}

char *complex_params_name[] = {
    "arr1",
    "dict1"
};
unsigned complex_params_type[] = {
    K_LIST,
    K_DICT
};
unsigned complex_params_secondary_type[] = {
    K_ANY,
    K_ANY
};
unsigned short complex_params_length = (unsigned short) sizeof(complex_params_type) / sizeof(unsigned);
int KAOS_EXPORT Kaos_complex()
{
    long long var1 = (long long) kaos.getListElementFloat(complex_params_name[0], 0);
    printf("%lld\n", var1);
    char* var2 = kaos.getDictElementString(complex_params_name[1], "a");
    printf("%s\n", var2);
    free(var2);
    return 0;
}

char *array_params_name[] = {};
unsigned array_params_type[] = {};
unsigned array_params_secondary_type[] = {};
unsigned short array_params_length = 0;
int KAOS_EXPORT Kaos_array()
{
    kaos.startBuildingList();
    kaos.createVariableBool(NULL, true);
    kaos.createVariableInt(NULL, 1);
    kaos.createVariableFloat(NULL, 3.14);
    kaos.createVariableString(NULL, "bar");
    kaos.returnList(K_ANY);
    return 0;
}

char *dictionary_params_name[] = {};
unsigned dictionary_params_type[] = {};
unsigned dictionary_params_secondary_type[] = {};
unsigned short dictionary_params_length = 0;
int KAOS_EXPORT Kaos_dictionary()
{
    kaos.startBuildingDict();
    kaos.createVariableBool("b", true);
    kaos.createVariableInt("i", 1);
    kaos.createVariableFloat("f", 3.14);
    kaos.createVariableString("s", "bar");
    kaos.returnDict(K_ANY);
    return 0;
}

char *optional_test_params_name[] = {
    "param1",
    "param2"
};
unsigned optional_test_params_type[] = {
    K_STRING,
    K_STRING
};
unsigned optional_test_params_secondary_type[] = {
    K_ANY,
    K_ANY
};
unsigned short optional_test_params_length = (unsigned short) sizeof(optional_test_params_type) / sizeof(unsigned);
int KAOS_EXPORT Kaos_optional_test()
{
    char* param1 = kaos.getVariableString(optional_test_params_name[0]);
    printf("%s\n", param1);
    free(param1);
    char* param2 = kaos.getVariableString(optional_test_params_name[1]);
    printf("%s\n", param2);
    free(param2);
    return 0;
}

int KAOS_EXPORT KaosRegister(struct Kaos _kaos)
{
    kaos = _kaos;
    kaos.defineFunction("hello", K_VOID, K_ANY, hello_params_name, hello_params_type, hello_params_secondary_type, hello_params_length, NULL, 0);
    kaos.defineFunction("add", K_NUMBER, K_ANY, add_params_name, add_params_type, add_params_secondary_type, add_params_length, NULL, 0);
    kaos.defineFunction("log", K_VOID, K_ANY, log_params_name, log_params_type, log_params_secondary_type, log_params_length, NULL, 0);
    kaos.defineFunction("complex", K_VOID, K_ANY, complex_params_name, complex_params_type, complex_params_secondary_type, complex_params_length, NULL, 0);
    kaos.defineFunction("array", K_LIST, K_ANY, array_params_name, array_params_type, array_params_secondary_type, array_params_length, NULL, 0);
    kaos.defineFunction("dictionary", K_DICT, K_ANY, dictionary_params_name, dictionary_params_type, dictionary_params_secondary_type, dictionary_params_length, NULL, 0);


    // Functions with optional parameters

    struct KaosValue optional_test_optional_param1;
    char *default_value = "default value";
    optional_test_optional_param1.s = malloc(1 + strlen(default_value));
    strcpy(optional_test_optional_param1.s, default_value);

    struct KaosValue optional_test_optional_params[] = {
        optional_test_optional_param1
    };

    kaos.defineFunction(
        "optional_test",
        K_VOID,
        K_ANY,
        optional_test_params_name,
        optional_test_params_type,
        optional_test_params_secondary_type,
        optional_test_params_length,
        optional_test_optional_params,
        1
    );

    return 0;
}
