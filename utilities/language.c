#include "language.h"

void greet() {
    int msg_width = 80;
    char lang[msg_width];
    char gcc[msg_width];
    char motto[msg_width];

    sprintf(lang, "    %s Language %s (%s %s) ", __LANGUAGE_NAME__, __LANGUAGE_VERSION__, __DATE__, __TIME__);
    sprintf(gcc, "    GCC version: %d.%d.%d on %s ",__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__, __PLATFORM_NAME__);
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
    printf("%-*s", msg_width, gcc);
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
