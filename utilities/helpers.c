#include "helpers.h"

char *itoa(int value, char *result, int base) {
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

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

void handle_end_keyword() {
    if (loop_mode != NULL) {
        endLoop();
        return;
    }

    endFunction();
}

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define MAXCHAR 1000
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

int replace_char(char *str, char orig, char rep) {
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;
}

void prepend_to_array(string_array *array, char *str) {
    array->arr = (char **)realloc(array->arr, (array->capacity += 1) * sizeof(char *));

    for (int k = array->size; k > 0; k--) {
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

void relative_path_to_absolute(char *path)
{
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
