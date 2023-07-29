#pragma once

#include <QString>
#include <physis.hpp>

// from xivdev
static char ChecksumTable[] = {'f', 'X', '1', 'p', 'G', 't', 'd', 'S', '5', 'C', 'A', 'P', '4', '_', 'V', 'L'};

inline char GetChecksum(const unsigned int key) {
    auto value = key & 0x000F0000;
    return ChecksumTable[value >> 16];
}

uint32_t TickCount();

inline QString encryptGameArg(const QString& arg) {
    const uint32_t rawTicks = TickCount();
    const uint32_t ticks = rawTicks & 0xFFFFFFFFu;
    const uint32_t key = ticks & 0xFFFF0000u;

    char buffer[9]{};
    sprintf(buffer, "%08x", key);

    Blowfish const* blowfish = physis_blowfish_initialize(reinterpret_cast<uint8_t*>(buffer), 9);

    uint8_t* out_data = nullptr;
    uint32_t out_size = 0;

    QByteArray toEncrypt = (QString(" /T =%1").arg(ticks) + arg).toUtf8();

    physis_blowfish_encrypt(
        blowfish, reinterpret_cast<uint8_t*>(toEncrypt.data()), toEncrypt.size(), &out_data, &out_size);

    const QByteArray encryptedArg =
        QByteArray::fromRawData(reinterpret_cast<const char*>(out_data), static_cast<int>(out_size));

    const QString base64 = encryptedArg.toBase64(
        QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::KeepTrailingEquals);
    const char checksum = GetChecksum(key);

    return QString("//**sqex0003%1%2**//").arg(base64, QString(checksum));
}