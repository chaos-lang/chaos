/*
 * Description: Helpers module of the Chaos Programming Language's source
 *
 * Copyright (c) 2019-2020 Chaos Language Development Authority <info@chaos-lang.org>
 *
 * License: GNU General Public License v3.0
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>
 *
 * Authors: M. Mert Yildiran <me@mertyildiran.com>
 */

#ifndef KAOS_HELPERS_H
#define KAOS_HELPERS_H

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#define __KAOS_ITOA_BUFFER_LENGTH__ 64

#include <ctype.h>
#include <string.h>
#include <assert.h>

typedef struct string_array {
    char **arr;
    unsigned capacity, size;
} string_array;

string_array free_string_stack;

#include "../interpreter/function.h"

char *longlong_to_string(long long value, char *result, unsigned short base);
char *trim_string(char *str);
char *fileGetContents(char *file_path);
char *strcat_ext(char *s1, const char *s2);
char *snprintf_concat_int(char *s1, char *format, long long i);
char *snprintf_concat_float(char *s1, char *format, long double f);
char *snprintf_concat_string(char *s1, char *format, char *s2);
void prepend_to_array(string_array *array, char *str);
void append_to_array(string_array *array, char *str);
void append_to_array_without_malloc(string_array *array, char *str);
bool is_in_array(string_array *array, char *str);
char *capitalize(const char *str);
int largest(int arr[], int n);
char *relative_path_to_absolute(char *path);
char *remove_ext(char* myStr, char extSep, char pathSep);
char* str_replace(char *target, const char *needle, const char *replacement);
bool is_file_exists(char* file_path);
const char *get_filename_ext(const char *filename);
void freeFreeStringStack();
char* escape_the_sequences_in_string_literal(char* string);
char* escape_string_literal_for_transpiler(char* string);
char* insert_nth_char(char* string, char c, long long n);
void remove_nth_char(char* string, long long n);
int string_ends_with(const char *str, const char *suffix);
void string_uppercase(char *s);
char* replace_char(char* str, char find, char replace);
void copy_binary_file(char* source_path, char* target_path);
bool in(char *s, char **x, int len);
string_array str_split(char* a_str, const char a_delim);
char *get_nth_line( FILE *f, int line_no );
char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   include <direct.h>
#   define GetCurrentDir _getcwd
#else
#   include <unistd.h>
#   define GetCurrentDir getcwd
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   define strdup _strdup
#endif

#include "cwalk.h"

#endif
