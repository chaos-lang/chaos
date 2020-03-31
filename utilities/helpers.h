#ifndef HELPERS_H
#define HELPERS_H

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#define __ITOA_BUFFER_LENGTH__ 64

#include <ctype.h>
#include <string.h>

typedef struct string_array {
    char **arr;
    unsigned capacity, size;
} string_array;

#include "loop.h"
#include "function.h"

char *itoa(int value, char *result, int base);
char *trim_string(char *str);
void handle_end_keyword();
char *fileGetContents(char *file_path);
char *strcat_ext(char *s1, const char *s2);
int replace_char(char *str, char orig, char rep);
void prepend_to_array(string_array *array, char *str);
void append_to_array(string_array *array, char *str);
char *capitalize(const char *str);
int largest(int arr[], int n);
void relative_path_to_absolute(char *path);
char *remove_ext(char* myStr, char extSep, char pathSep);
void str_replace(char *target, const char *needle, const char *replacement);

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#endif
