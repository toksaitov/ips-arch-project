#include "utils.h"

#ifdef _WIN32
    #include <windows.h>
#elif MACOS
    #include <sys/param.h>
    #include <sys/sysctl.h>
#else
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>

static inline int utils_get_number_of_cpu_cores()
{
    int result;

#ifdef WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    result = info.dwNumberOfProcessors;
#elif MACOS
    int nm[2];

    size_t length = 4;
    uint32_t count;

    nm[0] = CTL_HW;
    nm[1] = HW_AVAILCPU;

    sysctl(nm, 2, &count, &length, NULL, 0);

    if (count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &length, NULL, 0);
    }

    result = count;
#else
    result = sysconf(_SC_NPROCESSORS_ONLN);
#endif

    if (result < 1) {
        result = 1;
    }

    return result;
}

