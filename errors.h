#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>

enum ExitCode {
    E_SUCCESS,
    E_SYNTAX_ERROR,
    E_UNKNOWN_VARIABLE_TYPE,
    E_VARIABLE_ALREADY_DEFINED,
    E_UNDEFINED_VARIABLE,
    E_MEMORY_ALLOCATION_FOR_ARRAY_FAILED,
    E_ILLEGAL_ELEMENT_TYPE_FOR_TYPED_ARRAY,
    E_VARIABLE_IS_NOT_AN_ARRAY,
    E_UNDEFINED_INDEX,
    E_ILLEGAL_VARIABLE_TYPE_FOR_VARIABLE,
    E_ARRAYS_ARE_NOT_MASS_ASSIGNABLE,
    E_VARIABLE_IS_NOT_A_DICTIONARY,
    E_UNDEFINED_KEY,
    E_UNRECOGNIZED_COMPLEX_DATA_TYPE,
    E_ILLEGAL_VARIABLE_TYPE_FOR_FUNCTION,
    E_UNDEFINED_FUNCTION,
    E_MEMORY_ALLOCATION_FOR_FUNCTION_FAILED,
    E_MAXIMUM_RECURSION_DEPTH_EXCEEDED,
    E_UNEXPECTED_VALUE_TYPE,
    E_FUNCTION_DID_NOT_RETURN_ANYTHING,
    E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH,
    E_NO_VARIABLE_WITH_ID
};

extern void freeEverything();
extern bool is_interactive;
extern jmp_buf InteractiveShellErrorAbsorber;

void throw_error_base(int code, char *str1, char *str2, int int1, unsigned long long int llu1);

typedef struct {
    int code;
    char *str1;
    char *str2;
    int int1;
    unsigned long long int llu1;
} throw_error_args;

void throw_error_var(throw_error_args in);

#define throw_error(...) throw_error_var((throw_error_args){__VA_ARGS__});

#endif
