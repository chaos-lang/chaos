#ifndef HELPERS_H
#define HELPERS_H

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#define __ITOA_BUFFER_LENGTH__ 64

char *itoa(int value, char *result, int base);
char *trim_string(char *str);

#endif
