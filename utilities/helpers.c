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
        throw_error(20, file_path);
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
        throw_error(20, file_path);
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
