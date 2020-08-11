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
    char *file_buffer;
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
    size_t n = strlen(s1);

    char *p = (char *)malloc(n + strlen(s2) + 1);

    if (p) {
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
#if !defined(_WIN32) && !defined(_WIN64) && !defined(__CYGWIN__)
    size_t i;
    size_t j;
    size_t k;

    //Move to the beginning of the string
    i = 0;
    k = 0;

    //Replace backslashes with forward slashes
    while (path[i] != '\0') {
        //Forward slash or backslash separator found?
        if (path[i] == '/' || path[i] == '\\') {
            path[k++] = '/';
            while (path[i] == '/' || path[i] == '\\')
                i++;
        } else {
            path[k++] = path[i++];
        }
    }

    //Properly terminate the string with a NULL character
    path[k] = '\0';

    //Move back to the beginning of the string
    i = 0;
    j = 0;
    k = 0;

    //Parse the entire string
    do {
        //Forward slash separator found?
        if (path[i] == '/' || path[i] == '\0') {
            //"." element found?
            if ((i - j) == 1 && !strncmp (path + j, ".", 1)) {
                //Check whether the pathname is empty?
                if (k == 0) {
                    if (path[i] == '\0') {
                        path[k++] = '.';
                    } else if (path[i] == '/' && path[i + 1] == '\0') {
                        path[k++] = '.';
                        path[k++] = '/';
                    }
                } else if (k > 1) {
                    //Remove the final slash if necessary
                    if (path[i] == '\0')
                        k--;
                }
            }
            //".." element found?
            else if ((i - j) == 2 && !strncmp (path + j, "..", 2)) {
                //Check whether the pathname is empty?
                if (k == 0) {
                    path[k++] = '.';
                    path[k++] = '.';

                    //Append a slash if necessary
                    if (path[i] == '/')
                        path[k++] = '/';
                } else if (k > 1) {
                    //Search the path for the previous slash
                    for (j = 1; j < k; j++) {
                        if (path[k - j - 1] == '/')
                            break;
                    }

                    //Slash separator found?
                    if (j < k) {
                        if (!strncmp (path + k - j, "..", 2)) {
                            path[k++] = '.';
                            path[k++] = '.';
                        } else {
                            k = k - j - 1;
                        }

                        //Append a slash if necessary
                        if (k == 0 && path[0] == '/')
                            path[k++] = '/';
                        else if (path[i] == '/')
                            path[k++] = '/';
                    }
                    //No slash separator found?
                    else {
                        if (k == 3 && !strncmp (path, "..", 2)) {
                            path[k++] = '.';
                            path[k++] = '.';

                            //Append a slash if necessary
                            if (path[i] == '/')
                                path[k++] = '/';
                        } else if (path[i] == '\0') {
                            k = 0;
                            path[k++] = '.';
                        } else if (path[i] == '/' && path[i + 1] == '\0') {
                            k = 0;
                            path[k++] = '.';
                            path[k++] = '/';
                        } else {
                            k = 0;
                        }
                    }
                }
            } else {
                //Copy directory name
                memmove (path + k, path + j, i - j);
                //Advance write pointer
                k += i - j;

                //Append a slash if necessary
                if (path[i] == '/')
                    path[k++] = '/';
            }

            //Move to the next token
            while (path[i] == '/')
                i++;
            j = i;
        }
        else if (k == 0) {
            while (path[i] == '.' || path[i] == '/') {
                j++,i++;
            }
        }
    } while (path[i++] != '\0');

    //Properly terminate the string with a NULL character
    path[k] = '\0';
    return path;
#else
    char actual_path[PATH_MAX];
    char *absolute_path;
    char *filename;
    GetFullPathName(path, PATH_MAX, actual_path, &filename);
    free(path);
    absolute_path = (char *) malloc(strlen(actual_path) + 1);
    strcpy(absolute_path, actual_path);
    return absolute_path;
#endif
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

void str_replace(char *target, const char *needle, const char *replacement) {
    char buffer[1024] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

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
    strcpy(target, buffer);
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

    for (long long i = 0; i < strlen(new_string); i++){
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
                case 'e':
                    new_string[i+1] = '\e';
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
                default:
                    break;
            }
        }
    }

    return new_string;
}

void remove_nth_char(char* string, long long n) {
    memmove(&string[n], &string[n + 1], strlen(string) - n);
}
