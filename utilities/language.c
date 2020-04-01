#include "language.h"

extern int yylineno;
extern char *last_token;

void greet() {
    char lang[__MSG_LINE_LENGTH__];
    char compiler[__MSG_LINE_LENGTH__];
    char motto[__MSG_LINE_LENGTH__];

    sprintf(lang, "    %s Language %s (%s %s) ", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
    #if defined(__clang__)
        sprintf(compiler, "    Clang version: %d.%d.%d on %s ", __clang_major__, __clang_minor__, __clang_patchlevel__, __PLATFORM_NAME__);
    #elif defined(__GNUC__) || defined(__GNUG__)
        sprintf(compiler, "    GCC version: %d.%d.%d on %s ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);
    #endif
    sprintf(motto, "    %s", __LANGUAGE_MOTTO__);

    int cols[3];
    cols[0] = (int) strlen(lang) + 1;
    cols[1] = (int) strlen(compiler) + 1;
    cols[2] = (int) strlen(motto) + 1;
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
    char error_name_msg[__MSG_LINE_LENGTH__];
    char current_module_msg[__MSG_LINE_LENGTH__];
    char line_no_msg[__MSG_LINE_LENGTH__];
    char cause_msg[__MSG_LINE_LENGTH__];

    if (strcmp(last_token, "\n") != 0) yylineno++;

    sprintf(error_name_msg, "  %s:", error_name);
    sprintf(current_module_msg, "    Module: %s", current_module);
    sprintf(line_no_msg, "    Line: %d", yylineno);
    sprintf(cause_msg, "    Cause: %s", cause);

    str_replace(cause_msg, "\n", "\\n");

    int cols[4];
    cols[0] = (int) strlen(error_name_msg) + 1;
    cols[1] = (int) strlen(current_module_msg) + 1;
    cols[2] = (int) strlen(line_no_msg) + 1;
    cols[3] = (int) strlen(cause_msg) + 1;
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

void print_bye_bye() {
    int ws_col = (int) strlen(__BYE_BYE__) + 8 + 1;
    char bye_bye_msg[ws_col];
    sprintf(bye_bye_msg, "    %s    ", __BYE_BYE__);

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[5;42m");
    #endif
    printf("%-*s", ws_col, bye_bye_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
}
