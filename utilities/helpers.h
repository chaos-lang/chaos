#ifndef HELPERS_H
#define HELPERS_H

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#define __KAOS_ITOA_BUFFER_LENGTH__ 64

#include <ctype.h>
#include <string.h>

typedef struct string_array {
    char **arr;
    unsigned capacity, size;
} string_array;

string_array free_string_stack;

#include "loop.h"
#include "function.h"

char *longlong_to_string(long long value, char *result, unsigned short base);
char *trim_string(char *str);
void handle_end_keyword();
char *fileGetContents(char *file_path);
char *strcat_ext(char *s1, const char *s2);
void prepend_to_array(string_array *array, char *str);
void append_to_array(string_array *array, char *str);
void append_to_array_without_malloc(string_array *array, char *str);
char *capitalize(const char *str);
int largest(int arr[], int n);
char *relative_path_to_absolute(char *path);
char *remove_ext(char* myStr, char extSep, char pathSep);
void str_replace(char *target, const char *needle, const char *replacement);
bool is_file_exists(char* file_path);
const char *get_filename_ext(const char *filename);
void freeFreeStringStack();

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define strdup _strdup
#endif

#endif
