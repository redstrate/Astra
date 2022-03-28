#pragma once

#include <QString>
#include "blowfish.h"

// from xivdev
static char ChecksumTable[] = {
    'f', 'X', '1', 'p', 'G', 't', 'd', 'S',
    '5', 'C', 'A', 'P', '4', '_', 'V', 'L'
};

inline char GetChecksum(unsigned int key) {
    auto value = key & 0x000F0000;
    return ChecksumTable[value >> 16];
}

uint32_t TickCount();

inline QString encryptGameArg(QString arg) {
    unsigned int rawTicks = TickCount();
    unsigned int ticks = rawTicks & 0xFFFFFFFFu;
    unsigned int key = ticks & 0xFFFF0000u;

    char buffer[9] = {};
    sprintf(buffer, "%08x", key);

    Blowfish session(QByteArray(buffer, 8));
    QByteArray encryptedArg = session.Encrypt((QString(" /T =%1").arg(ticks) + arg).toUtf8());
    QString base64 = encryptedArg.toBase64(QByteArray::Base64Option::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals);
    char checksum = GetChecksum(key);

    return QString("//**sqex0003%1%2**//").arg(base64, QString(checksum));
}