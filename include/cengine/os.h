#ifndef _CENGINE_OS_H_
#define _CENGINE_OS_H_

    #ifdef          __linux__
    #define     OS_LINUX
    #elif defined   __APPLE__ && __MACH__
    #define     OS_MACOS
    #elif defined   _WIN32 || _WIN64 || __WINDOWS__
    #define     OS_WINDOWS
    #else
    #define     OS_UNKNOWN
    #endif

#endif