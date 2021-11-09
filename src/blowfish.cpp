#include "blowfish.h"

BlowfishSession::BlowfishSession() {
    mbedtls_blowfish_init(&ctx);
}

void BlowfishSession::setKey(QString key) {
    std::string keyStr = key.toStdString();
    mbedtls_blowfish_setkey(&ctx, (unsigned char*)keyStr.c_str(), key.length() * 8);
}

QByteArray BlowfishSession::encrypt(QString string) {
    QByteArray finalArray;
    for(int i = 0; i < string.length(); i += 8) {
        unsigned char input[MBEDTLS_BLOWFISH_BLOCKSIZE];
        memset(input, 0, 8);

        std::string inputStr = string.toStdString().substr(i, 8);
        strcpy((char*)input, inputStr.c_str());

        unsigned char output[MBEDTLS_BLOWFISH_BLOCKSIZE];
        memset(output, 0, 8);

        mbedtls_blowfish_crypt_ecb(&ctx, MBEDTLS_BLOWFISH_ENCRYPT, input, output);

        QByteArray arr((char*)output, 8);
        finalArray.append(arr);
    }

    return finalArray;
}

QString BlowfishSession::decrypt(QByteArray data) {
    QString finalString;

    for(int i = 0; i < data.length(); i += 8) {
        unsigned char input[MBEDTLS_BLOWFISH_BLOCKSIZE];
        memset(input, 0, 8);
        memcpy(input, data.data() + i, 8);

        unsigned char output[MBEDTLS_BLOWFISH_BLOCKSIZE];
        memset(output, 0, 8);

        mbedtls_blowfish_crypt_ecb(&ctx, MBEDTLS_BLOWFISH_DECRYPT, input, output);

        QString str((char*)output);
        finalString.append(str);
    }

    return finalString;
}