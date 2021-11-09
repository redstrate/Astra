#pragma once

#include <QString>

#include <mbedtls/blowfish.h>

class BlowfishSession {
public:
    BlowfishSession();

    void setKey(QString key);

    QByteArray encrypt(QString string);
    QString decrypt(QByteArray data);

private:
    mbedtls_blowfish_context ctx;
};
