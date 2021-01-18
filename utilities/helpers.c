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

#include "helpers.h"

char *longlong_to_string(long long value, char *result, unsigned short base) {
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    long long tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

char *trim_string(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
    return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
char *fileGetContents(char *file_path) {
    char *file_content = "";
    char str[MAXCHAR];
    FILE * fp = fopen(file_path, "r");
    if (fp) {
        while (fgets(str, MAXCHAR, fp) != NULL) {
            char *line = malloc(1 + strlen(str));
            strcpy(line, str);
            file_content = strcat_ext(file_content, line);
            free(line);
        }
        fclose(fp);
    } else {
        append_to_array_without_malloc(&free_string_stack, file_path);
        throw_error(E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH, file_path);
    }
    return file_content;
}
#else
char *fileGetContents(char *file_path) {
    char *file_buffer = NULL;
    long length;
    FILE * f = fopen(file_path, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        file_buffer = malloc(length + 1);
        file_buffer[length] = '\0';
        if (file_buffer) {
            fread(file_buffer, 1, length, f);
        }
        fclose(f);
    } else {
        append_to_array_without_malloc(&free_string_stack, file_path);
        throw_error(E_MODULE_IS_EMPTY_OR_NOT_EXISTS_ON_PATH, file_path);
    }
    return file_buffer;
}
#endif

char *strcat_ext(char *s1, const char *s2)
{
    size_t n = 0;
    if (s1 != NULL)
        n = strlen(s1);

    char *p = (char *)malloc(n + strlen(s2) + 1);

    if (p) {
        if (s1 != NULL)
            strcpy(p, s1);
        strcpy(p + n, s2);
    }

    if ((s1 != NULL) && (s1[0] != '\0')) {
        free(s1);
    }
    return p;
}

char *snprintf_concat_int(char *s1, char *format, long long i)
{
    char *tmp;
    size_t len;

    // 1st pass gets needed size
    len = (size_t) snprintf(NULL, 0, format, i) + 1;
    tmp = malloc(len);

    // Safely write to tmp with known length 'len'
    snprintf(tmp, len, format, i);

    s1 = strcat_ext(s1, tmp);
    free(tmp);
    return s1;
}

char *snprintf_concat_float(char *s1, char *format, long double f)
{
    char *tmp;
    size_t len;

    // 1st pass gets needed size
    len = (size_t) snprintf(NULL, 0, format, f) + 1;
    tmp = malloc(len);

    // Safely write to tmp with known length 'len'
    snprintf(tmp, len, format, f);

    s1 = strcat_ext(s1, tmp);
    free(tmp);
    return s1;
}

char *snprintf_concat_string(char *s1, char *format, char *s2)
{
    char *tmp;
    size_t len;

    // 1st pass gets needed size
    len = (size_t) snprintf(NULL, 0, format, s2) + 1;
    tmp = malloc(len);

    // Safely write to tmp with known length 'len'
    snprintf(tmp, len, format, s2);

    s1 = strcat_ext(s1, tmp);
    free(tmp);
    return s1;
}

void prepend_to_array(string_array *array, char *str) {
    array->arr = (char **)realloc(array->arr, (array->capacity += 1) * sizeof(char *));

    for (unsigned k = array->size; k > 0; k--) {
        array->arr[k] = array->arr[k - 1];
    }

    array->arr[0] = malloc(1 + strlen(str));
    strcpy(array->arr[0], str);
    array->size++;
}

void append_to_array(string_array *array, char *str) {
    if (array->capacity == 0) {
        array->arr = (char **)malloc((array->capacity = 2) * sizeof(char *));
    } else if (array->capacity == array->size) {
        array->arr = (char **)realloc(array->arr, (array->capacity *= 2) * sizeof(char *));
    }

    array->arr[array->size] = malloc(1 + strlen(str));
    strcpy(array->arr[array->size], str);
    array->size++;
}

void append_to_array_without_malloc(string_array *array, char *str) {
    if (array->capacity == 0) {
        array->arr = (char **)malloc((array->capacity = 2) * sizeof(char *));
    } else if (array->capacity == array->size) {
        array->arr = (char **)realloc(array->arr, (array->capacity *= 2) * sizeof(char *));
    }

    array->arr[array->size] = str;
    array->size++;
}

bool is_in_array(string_array *array, char *str) {
    for (unsigned i = 0; i < array->size; i++) {
        if (strcmp(array->arr[i], str) == 0)
            return true;
    }
    return false;
}

char *capitalize(const char *str) {
    char *_str = malloc(1 + strlen(str));
    strcpy(_str, str);
    _str[0] = toupper(_str[0]);
    return _str;
}

int largest(int arr[], int n) {
    int i;

    // Initialize maximum element
    int max = arr[0];

    // Traverse array elements from second and
    // compare every element with current max
    for (i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];

    return max;
}

char *relative_path_to_absolute(char *path)
{
    char actual_path[PATH_MAX];
    char *absolute_path;
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    realpath(path, actual_path);
#else
    char *filename;
    GetFullPathName(path, PATH_MAX, actual_path, &filename);
#endif
    free(path);
    absolute_path = (char *) malloc(strlen(actual_path) + 1);
    strcpy(absolute_path, actual_path);
    return absolute_path;
}

char *remove_ext(char* myStr, char extSep, char pathSep) {
    char *retStr, *lastExt, *lastPath;

    // Error checks and allocate string.

    if (myStr == NULL) return NULL;
    if ((retStr = malloc (strlen (myStr) + 1)) == NULL) return NULL;

    // Make a copy and find the relevant characters.

    strcpy (retStr, myStr);
    lastExt = strrchr (retStr, extSep);
    lastPath = (pathSep == 0) ? NULL : strrchr (retStr, pathSep);

    // If it has an extension separator.

    if (lastExt != NULL) {
        // and it's to the right of the path separator.

        if (lastPath != NULL) {
            if (lastPath < lastExt) {
                // then remove it.

                *lastExt = '\0';
            }
        } else {
            // Has extension separator with no path separator.

            *lastExt = '\0';
        }
    }

    // Return the modified string.

    return retStr;
}

char* str_replace(char *target, const char *needle, const char *replacement) {
    char buffer[4096] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    size_t counter = 0;

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }
        counter++;

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    char* new_target = (char*) malloc((1 + strlen(target) + counter * (repl_len - needle_len)) * sizeof(char));
    strcpy(new_target, buffer);
    return new_target;
}

bool is_file_exists(char* file_path) {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    WIN32_FIND_DATA FindFileData;
    HANDLE handle = FindFirstFile(file_path, &FindFileData) ;
    int found = handle != INVALID_HANDLE_VALUE;
    FindClose(handle);
    if (found) {
        return true;
    } else {
        return false;
    }
#else
    return access(file_path, F_OK) != -1;
#endif
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

void freeFreeStringStack() {
    for (unsigned i = 0; i < free_string_stack.size; i++) {
        free(free_string_stack.arr[i]);
    }
    free_string_stack.capacity = 0;
    free_string_stack.size = 0;
    free(free_string_stack.arr);
}

char* escape_the_sequences_in_string_literal(char* string) {
    char* new_string = malloc(strlen(string) + 1);
    strcpy(new_string, string);

    for (long long i = 0; i < (long long) strlen(new_string); i++){
        if (new_string[i] == '\\') {
            switch (new_string[i+1])
            {
                case '\\':
                    remove_nth_char(new_string, i);
                    break;
                case 'a':
                    new_string[i+1] = '\a';
                    remove_nth_char(new_string, i);
                    break;
                case 'b':
                    new_string[i+1] = '\b';
                    remove_nth_char(new_string, i);
                    break;
                case 'f':
                    new_string[i+1] = '\f';
                    remove_nth_char(new_string, i);
                    break;
                case 'n':
                    new_string[i+1] = '\n';
                    remove_nth_char(new_string, i);
                    break;
                case 'r':
                    new_string[i+1] = '\r';
                    remove_nth_char(new_string, i);
                    break;
                case 't':
                    new_string[i+1] = '\t';
                    remove_nth_char(new_string, i);
                    break;
                case 'v':
                    new_string[i+1] = '\v';
                    remove_nth_char(new_string, i);
                    break;
                case '"':
                    remove_nth_char(new_string, i);
                    break;
                case '\'':
                    remove_nth_char(new_string, i);
                    break;
                default:
                    break;
            }
        }
    }

    return new_string;
}

char* escape_string_literal_for_transpiler(char* string) {
    char* new_string = malloc(strlen(string) + 1);
    strcpy(new_string, string);
    bool pass_next = false;
    char prev_char = '\0';

    for (long long i = 0; i < (long long) strlen(new_string); i++){
        if (pass_next) {
            pass_next = false;
            continue;
        }

        switch (new_string[i])
        {
            case '\a':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 'a';
                break;
            case '\b':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 'b';
                break;
            case '\f':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 'f';
                break;
            case '\n':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 'n';
                break;
            case '\r':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 'r';
                break;
            case '\t':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 't';
                break;
            case '\v':
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = 'v';
                break;
            case '"':
                if (prev_char == '\\')
                    break;
                new_string = insert_nth_char(new_string, '\\', i);
                new_string[i+1] = '"';
                pass_next = true;
                break;
            case '\'':
                if (prev_char == '\\')
                    remove_nth_char(new_string, i-1);
                break;
            default:
                break;
        }

        prev_char = new_string[i];
    }

    return new_string;
}

char* insert_nth_char(char* string, char c, long long n) {
    string = realloc(string, 2 + strlen(string));
    memmove(string + n + 1, string + n, strlen(string) + 1 - n);
    string[n] = c;
    return string;
}

void remove_nth_char(char* string, long long n) {
    memmove(&string[n], &string[n + 1], strlen(string) - n);
}

int string_ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void string_uppercase(char *s) {
    while (*s) {
        *s = toupper((unsigned char) *s);
        s++;
    }
}

char* replace_char(char* str, char find, char replace) {
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

void copy_binary_file(char* source_path, char* target_path) {
    FILE *source_fp, *target_fp;
    source_fp = fopen(source_path, "rb");
    target_fp = fopen(target_path, "wb");

    size_t n, m;
    unsigned char buff[8192];
    do {
        n = fread(buff, 1, sizeof buff, source_fp);
        if (n) m = fwrite(buff, 1, n, target_fp);
        else   m = 0;
    } while ((n > 0) && (n == m));

    fclose(source_fp);
    fclose(target_fp);
}

bool in(char *s, char **x, int len) {
    int i;
    for (i = 0; i < len; ++i) {
        if (!strcmp(x[i], s)) {
            return true;
        }
    }
    return false;
}

string_array str_split(char* a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_delim = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_delim = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_delim < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    string_array result_string_stack;
    result_string_stack.arr = result;
    result_string_stack.size = count;
    return result_string_stack;
}

char *get_nth_line( FILE *f, int line_no ) {
    int BUF_SIZE = 256;
    char   buf[ BUF_SIZE ];
    size_t curr_alloc = BUF_SIZE, curr_ofs = 0;
    char   *line      = malloc( BUF_SIZE );
    int    in_line    = line_no == 1;
    size_t bytes_read;

    /* Illegal to ask for a line before the first one. */
    if ( line_no < 1 )
        return NULL;

    /* Handle out-of-memory by returning NULL */
    if ( !line )
        return NULL;

    /* Scan the file looking for newlines */
    while ( line_no && ( bytes_read = fread( buf, 1, BUF_SIZE, f ) ) > 0 ) {
        int i;

        for ( i = 0 ; i < bytes_read ; i++ ) {
            if ( in_line ) {
                if ( curr_ofs >= curr_alloc ) {
                    curr_alloc <<= 1;
                    line = realloc( line, curr_alloc );

                    if ( !line )    /* out of memory? */
                        return NULL;
                }
                line[ curr_ofs++ ] = buf[i];
            }

            if ( buf[i] == '\n' ) {
                line_no--;

                if ( line_no == 1 )
                    in_line = 1;

                if ( line_no == 0 )
                    break;
            }
        }
    }

    /* Didn't find the line? */
    if ( line_no != 0 ) {
        free( line );
        return NULL;
    }

    /*  Resize allocated buffer to what's exactly needed by the string
        and the terminating NUL character.  Note that this code *keeps*
        the terminating newline as part of the string.
     */
    line = realloc( line, curr_ofs + 1 );

    if ( !line ) /* out of memory? */
        return NULL;

    /* Add the terminating NUL. */
    line[ curr_ofs ] = '\0';

    /* Return the line.  Caller is responsible for freeing it. */
    return line;
}

char *ltrim(char *s) {
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s));
}
