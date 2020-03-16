#ifndef HELPERS_H
#define HELPERS_H

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#define __ITOA_BUFFER_LENGTH__ 64

#include <ctype.h>
#include <string.h>

#include "loop.h"
#include "function.h"

char *itoa(int value, char *result, int base);
char *trim_string(char *str);
void handle_end_keyword();
char *fileGetContents(char *file_path);
char *strcat_ext(char *s1, const char *s2);
int replace_char(char *str, char orig, char rep);

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#endif
