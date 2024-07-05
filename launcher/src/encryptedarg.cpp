// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptedarg.h"

#include <physis.hpp>

#if defined(Q_OS_MAC)
#include <mach/mach_time.h>
#include <sys/sysctl.h>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#if defined(Q_OS_MAC)
// taken from XIV-on-Mac, apparently Wine changed this?
uint32_t TickCount()
{
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
uint32_t TickCount()
{
    struct timespec ts {
    };

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#endif

#if defined(Q_OS_WIN)
uint32_t TickCount()
{
    return GetTickCount();
}
#endif

// from xivdev
static char ChecksumTable[] = {'f', 'X', '1', 'p', 'G', 't', 'd', 'S', '5', 'C', 'A', 'P', '4', '_', 'V', 'L'};

inline char GetChecksum(const unsigned int key)
{
    const auto value = key & 0x000F0000;
    return ChecksumTable[value >> 16];
}

QString encryptGameArg(const QString &arg)
{
    const uint32_t rawTicks = TickCount();
    const uint32_t ticks = rawTicks & 0xFFFFFFFFu;
    const uint32_t key = ticks & 0xFFFF0000u;

    char buffer[9]{};
    sprintf(buffer, "%08x", key);

    Blowfish const *blowfish = physis_blowfish_initialize(reinterpret_cast<uint8_t *>(buffer), 9);

    uint8_t *out_data = nullptr;
    uint32_t out_size = 0;

    QByteArray toEncrypt = (QStringLiteral(" /T =%1").arg(ticks) + arg).toUtf8();

    physis_blowfish_encrypt(blowfish, reinterpret_cast<uint8_t *>(toEncrypt.data()), toEncrypt.size(), &out_data, &out_size);

    const QByteArray encryptedArg = QByteArray::fromRawData(reinterpret_cast<const char *>(out_data), static_cast<int>(out_size));

    const QString base64 = QString::fromUtf8(encryptedArg.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::KeepTrailingEquals));
    const char checksum = GetChecksum(key);

    return QStringLiteral("//**sqex0003%1%2**//").arg(base64, QString(QLatin1Char(checksum)));
}