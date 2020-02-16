#include "language.h"

void greet() {
    int msg_width = 80;
    char lang[msg_width];
    char compiler[msg_width];
    char motto[msg_width];

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
    printf("%-*s", msg_width, lang);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[1;45m");
    #endif
    printf("%-*s", msg_width, compiler);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n");
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[5;45m");
    #endif
    printf("%-*s", msg_width, motto);
    #if defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        printf("\033[0m");
    #endif
    printf("\n\n");
}
