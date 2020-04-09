#ifndef PLATFORM_H
#define PLATFORM_H

/**
 * Determination a platform of an operation system
 * Fully supported supported only GNU GCC/G++, partially on Clang/LLVM
 */

#if defined(_WIN32)
    #define __KAOS_PLATFORM_NAME__ "windows" // Windows
#elif defined(_WIN64)
    #define __KAOS_PLATFORM_NAME__ "windows" // Windows
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #define __KAOS_PLATFORM_NAME__ "windows" // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
    #define __KAOS_PLATFORM_NAME__ "android" // Android (implies Linux, so it must come first)
#elif defined(__linux__)
    #define __KAOS_PLATFORM_NAME__ "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
    #include <sys/param.h>
    #if defined(BSD)
        #define __KAOS_PLATFORM_NAME__ "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
    #endif
#elif defined(__hpux)
    #define __KAOS_PLATFORM_NAME__ "hp-ux" // HP-UX
#elif defined(_AIX)
    #define __KAOS_PLATFORM_NAME__ "aix" // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1
        #define __KAOS_PLATFORM_NAME__ "ios" // Apple iOS
    #elif TARGET_OS_IPHONE == 1
        #define __KAOS_PLATFORM_NAME__ "ios" // Apple iOS
    #elif TARGET_OS_MAC == 1
        #define __KAOS_PLATFORM_NAME__ "osx" // Apple OSX
    #endif
#elif defined(__sun) && defined(__SVR4)
    #define __KAOS_PLATFORM_NAME__ "solaris" // Oracle Solaris, Open Indiana
#else
    #define __KAOS_PLATFORM_NAME__ NULL
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define __KAOS_PATH_SEPARATOR__ "\\"
    #define __KAOS_PATH_SEPARATOR_ASCII__ '\\'
#else
    #define __KAOS_PATH_SEPARATOR__ "/"
    #define __KAOS_PATH_SEPARATOR_ASCII__ '/'
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #if !defined(PATH_MAX)
        #define PATH_MAX _MAX_PATH
    #endif
#endif

#endif
