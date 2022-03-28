#include "encryptedarg.h"

#if defined(Q_OS_MAC)
#include <sys/sysctl.h>
#include <mach/mach_time.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#if defined(Q_OS_MAC)
// this is pretty much what wine does :-0
uint32_t TickCount() {
    struct mach_timebase_info convfact;
    mach_timebase_info(&convfact);

    return (mach_absolute_time() * convfact.numer) / (convfact.denom * 1000000);
}
#endif

#if defined(Q_OS_LINUX)
uint32_t TickCount() {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if defined(Q_OS_WIN)
uint32_t TickCount() {
    return GetTickCount();
}
#endif