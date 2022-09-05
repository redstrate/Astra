#include "encryptedarg.h"

#if defined(Q_OS_MAC)
    #include <mach/mach_time.h>
    #include <sys/sysctl.h>
#endif

#if defined(Q_OS_WIN)
    #include <windows.h>
#endif

#if defined(Q_OS_MAC)
// taken from XIV-on-Mac, apparently Wine changed this?
uint32_t TickCount() {
    struct mach_timebase_info timebase;
    mach_timebase_info(&timebase);

    auto machtime = mach_continuous_time();
    auto numer = uint64_t(timebase.numer);
    auto denom = uint64_t(timebase.denom);
    auto monotonic_time = machtime * numer / denom / 100;
    return monotonic_time / 10000;
}
#endif

#if defined(Q_OS_LINUX)
uint32_t TickCount() {
    struct timespec ts{};

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if defined(Q_OS_WIN)
uint32_t TickCount() {
    return GetTickCount();
}
#endif