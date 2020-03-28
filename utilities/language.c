#include "language.h"

extern int yylineno;

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

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;45m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, lang);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;45m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, compiler);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[5;45m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, motto);
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

    sprintf(error_name_msg, "  %s:", error_name);
    sprintf(current_module_msg, "    Module: %s", current_module);
    sprintf(line_no_msg, "    Line: %d", yylineno);
    sprintf(cause_msg, "    Cause: %s", cause);

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;46m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, error_name_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;46m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, current_module_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;46m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, line_no_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");

    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;46m");
    #endif
    printf("%-*s", __MSG_LINE_LENGTH__, cause_msg);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
}
