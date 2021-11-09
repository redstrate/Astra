#include "blowfish.h"

BlowfishSession::BlowfishSession() {
    mbedtls_blowfish_init(&ctx);
}

void BlowfishSession::setKey(unsigned int key) {
    mbedtls_blowfish_setkey(&ctx, (unsigned char*)&key, sizeof(key) * 8);
}

QByteArray BlowfishSession::encrypt(QString string) {
    std::string inputStr = string.toStdString();

    QByteArray finalArray;
    for(int i = 0; i < inputStr.length(); i += 8) {
        unsigned char input[MBEDTLS_BLOWFISH_BLOCKSIZE + 1];
        memset(input, 0, 9);
        memcpy(input, (char*)inputStr.c_str() + i, 8);

        unsigned char output[MBEDTLS_BLOWFISH_BLOCKSIZE];
        memset(output, 0, 8);

        mbedtls_blowfish_crypt_ecb(&ctx, MBEDTLS_BLOWFISH_ENCRYPT, input, output);

        finalArray.append((char*)output, 8);
    }

    return finalArray;
}

QString BlowfishSession::decrypt(QByteArray data) {
    QString finalString;

    for(int i = 0; i < data.size(); i += 8) {
        int adjusted_length = 8;
        if((i + 8) > data.size())
            adjusted_length = (i + 8) - data.size();

        unsigned char input[MBEDTLS_BLOWFISH_BLOCKSIZE];
        memset(input, 0, 8);
        memcpy(input, data.data() + i, adjusted_length);

        unsigned char output[MBEDTLS_BLOWFISH_BLOCKSIZE + 1];
        memset(output, 0, 9);

        mbedtls_blowfish_crypt_ecb(&ctx, MBEDTLS_BLOWFISH_DECRYPT, input, output);

        finalString.append((char*)output);
    }

    return finalString;
}