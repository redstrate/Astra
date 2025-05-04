// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptedarg.h"
#include "crtrand.h"

#include <QDebug>
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

    Blowfish *blowfish = physis_blowfish_initialize(reinterpret_cast<uint8_t *>(buffer), 9);

    uint8_t *out_data = nullptr;
    uint32_t out_size = 0;

    QByteArray toEncrypt = (QStringLiteral(" /T =%1").arg(ticks) + arg).toUtf8();

    physis_blowfish_encrypt(blowfish, reinterpret_cast<uint8_t *>(toEncrypt.data()), toEncrypt.size(), &out_data, &out_size);

    const QByteArray encryptedArg = QByteArray::fromRawData(reinterpret_cast<const char *>(out_data), static_cast<int>(out_size));

    const QString base64 = QString::fromUtf8(encryptedArg.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::KeepTrailingEquals));
    const char checksum = GetChecksum(key);

    physis_blowfish_free(blowfish);

    return QStringLiteral("//**sqex0003%1%2**//").arg(base64, QString(QLatin1Char(checksum)));
}

// Based off of the XIVQuickLauncher implementation
constexpr auto SQEX_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
constexpr int SPLIT_SIZE = 300;

QStringList intoChunks(const QString &str, const int maxChunkSize)
{
    QStringList chunks;
    for (int i = 0; i < str.length(); i += maxChunkSize) {
        chunks.push_back(str.mid(i, std::min(static_cast<qlonglong>(maxChunkSize), str.length() - i)));
    }

    return chunks;
}

QString encryptSteamTicket(const QByteArray &ticket, uint32_t time)
{
    // Round the time down
    time -= 5;
    time -= time % 60;

    auto ticketString = QString::fromLatin1(ticket.toHex()).remove(QLatin1Char('-')).toLower();
    auto rawTicketBytes = ticketString.toLatin1();
    rawTicketBytes.append('\0');

    ushort ticketSum = 0;
    for (const auto b : rawTicketBytes) {
        ticketSum += b;
    }

    QByteArray binaryWriter;
    binaryWriter.append(reinterpret_cast<const char *>(&ticketSum), sizeof(ushort));
    binaryWriter.append(rawTicketBytes);

    const int castTicketSum = static_cast<short>(ticketSum);
    const auto seed = time ^ castTicketSum;
    auto rand = CrtRand(seed);

    const auto numRandomBytes = (static_cast<ulong>(rawTicketBytes.length() + 9) & 0xFFFFFFFFFFFFFFF8) - 2 - static_cast<ulong>(rawTicketBytes.length());
    auto garbage = QByteArray();
    garbage.resize(numRandomBytes);

    uint badSum = *reinterpret_cast<uint32_t *>(binaryWriter.data());

    for (auto i = 0u; i < numRandomBytes; i++) {
        const auto randChar = SQEX_ALPHABET[static_cast<int>(badSum + rand.next()) & 0x3F];
        garbage[i] = static_cast<char>(randChar);
        badSum += randChar;
    }

    binaryWriter.append(garbage);

    char blowfishKey[17]{};
    sprintf(blowfishKey, "%08x#un@e=x>", time);

    binaryWriter.remove(0, 4);
    binaryWriter.insert(0, reinterpret_cast<const char *>(&badSum), sizeof(uint));

    // swap first two bytes
    auto finalBytes = binaryWriter;
    std::swap(finalBytes[0], finalBytes[1]);

    SteamTicketBlowfish *blowfish = miscel_steamticket_blowfish_initialize(reinterpret_cast<uint8_t *>(blowfishKey), 16);

    miscel_steamticket_blowfish_encrypt(blowfish, reinterpret_cast<uint8_t *>(finalBytes.data()), finalBytes.size());
    Q_ASSERT(finalBytes.length() % 8 == 0);

    miscel_steamticket_physis_blowfish_free(blowfish);

    auto encoded = finalBytes.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::KeepTrailingEquals);
    encoded.replace('+', '-');
    encoded.replace('/', '_');
    encoded.replace('=', '*');

    return intoChunks(QString::fromLatin1(encoded), SPLIT_SIZE).join(QLatin1Char(','));
}
