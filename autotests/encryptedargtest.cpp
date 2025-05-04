// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "encryptedarg.h"

class EncryptedArgTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void steamTicket_data()
    {
        QTest::addColumn<QString>("ticketBytes");
        QTest::addColumn<uint32_t>("time");
        QTest::addColumn<QString>("encryptedTicket");
        QTest::addColumn<int>("encryptedLength");

        QTest::addRow("real ticket")
            << QStringLiteral(
                   "140000009efb2c79d200f07599f633050100100195c517681800000001000000020000002c9ea991afec1a49f7e72d000b000000b8000000380000000400000099f63305010"
                   "010012a990000c47323496501a8c000000000807a1268002a2e680100908400000100c5000400000000004e408e518c259b8b329b5fcfdb00903fadad36d7e088813dc9b174"
                   "b06e08b69f8f46d083cf4118d1eb4062e009147906d9c80759af3eb221f7fbb8e28d402e11ba80e3cf4a101487f87ccce9688daf99dd412e6bcaab6e58f70030ff99323e4c8"
                   "1824659f7aa89188fadc6402bcb540843e1578cd112d4536fd65c4bd926f754")
            << static_cast<uint32_t>(1746389891)
            << QStringLiteral(
                   "Tjr8Lv1O0HjJ7U4dOfkA9BdLAnEaCl_TU0GnYGGLTBM06TV9Ggf-fYb7WMqD-Xv758Q1zzSPTeaJctl8au-"
                   "imM4ACRgl0Y4LqJpLFfgBhkumd4dne2P9oM6qLzMnHfspPq8AFQFHXaiSicu2gSaCwpk36ZK-WX17DaTOkYFncIKl_rSZAkb8OzTpNX0aB_590hUpAf74-"
                   "TU368A1fgXLw2aunwn0wBNvz0ywFEiAjmD8PfgUzA6IrvkP1eoKoY4A_NNBXnirca7CjWxOoguXRGaHjzq9vrDm8ABTk2o0u29R,Nqmz_4LN1Fj9cNhtyhHTXuV6huLxmsflb_"
                   "6DR5B8dwk8IMYup0z5AXHhLww0BmZkDKKCWjVehxWvoHkz8FNViV9Oduwv7ZGyHUYs47HUpOIr1Wirp6LEvsxBcDBf-T_XOK945j-z_"
                   "MtxXiNKqtAuaL8iw7OOIpVnXqIa77yGuOFFW-u2wv1cK1M3s_OqmgEdj0JZfoYbjT6lIEVsSXKMYwwf9zkAjx23K-gqrM8c8nStv4EYT7ZU6o_"
                   "I0KZ6OJVnCFElYLamz82NIRiPdzyuJcPoslNCXpQV_vWlyGJ0OIoR,2MrkkwMnTNx7HR4FJ6ACh0cQZmdBEB2pM4eQSqpJEC367JtCMzM*")
            << 652;
    }

    void steamTicket()
    {
        QFETCH(QString, ticketBytes);
        QFETCH(uint32_t, time);
        QFETCH(QString, encryptedTicket);
        QFETCH(int, encryptedLength);

        auto pair = std::pair{encryptedTicket, encryptedLength};
        QCOMPARE(encryptSteamTicket(ticketBytes, time), pair);
    }
};

QTEST_MAIN(EncryptedArgTest)
#include "encryptedargtest.moc"
