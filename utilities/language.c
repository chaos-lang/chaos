#include "language.h"

extern int yylineno;
extern char *last_token;

void greet() {
    struct winsize terminal;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);

    char lang[terminal.ws_col];
    char compiler[terminal.ws_col];
    char motto[terminal.ws_col];

    sprintf(lang, "    %s Language %s (%s %s) ", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
    #if defined(__clang__)
        sprintf(compiler, "    Clang version: %d.%d.%d on %s ", __clang_major__, __clang_minor__, __clang_patchlevel__, __PLATFORM_NAME__);
    #elif defined(__GNUC__) || defined(__GNUG__)
        sprintf(compiler, "    GCC version: %d.%d.%d on %s ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);
    #endif
    sprintf(motto, "    %s", __LANGUAGE_MOTTO__);

    int cols[3];
    cols[0] = (int) strlen(lang);
    cols[1] = (int) strlen(compiler);
    cols[2] = (int) strlen(motto);
    int ws_col = largest(cols, 3) + 4;

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;45m");
    #endif
    printf("%-*s", ws_col, lang);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;45m");
    #endif
    printf("%-*s", ws_col, compiler);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[5;45m");
    #endif
    printf("%-*s", ws_col, motto);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n\n");
}

void yyerror_msg(char* error_name, char* current_module, char* cause) {
    struct winsize terminal;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);

    char error_name_msg[terminal.ws_col];
    char current_module_msg[terminal.ws_col];
    char line_no_msg[terminal.ws_col];
    char cause_msg[terminal.ws_col];

    if (strcmp(last_token, "\n") != 0) yylineno++;

    sprintf(error_name_msg, "  %s:", error_name);
    sprintf(current_module_msg, "    Module: %s", current_module);
    sprintf(line_no_msg, "    Line: %d", yylineno);
    sprintf(cause_msg, "    Cause: %s", cause);

    str_replace(cause_msg, "\n", "\\n");

    int cols[4];
    cols[0] = (int) strlen(error_name_msg);
    cols[1] = (int) strlen(current_module_msg);
    cols[2] = (int) strlen(line_no_msg);
    cols[3] = (int) strlen(cause_msg);
    int ws_col = largest(cols, 3) + 4;

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;46m");
    #endif
    printf("%-*s", ws_col, error_name_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;46m");
    #endif
    printf("%-*s", ws_col, current_module_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;46m");
    #endif
    printf("%-*s", ws_col, line_no_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0;46m");
    #endif
    printf("%-*s", ws_col, cause_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
}
