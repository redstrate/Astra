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
        QTest::addColumn<QByteArray>("ticketBytes");
        QTest::addColumn<uint32_t>("time");
        QTest::addColumn<QString>("encryptedTicket");

        QTest::addRow("test 1") << QByteArrayLiteral(
            "caf59103e80d600b4a9233358c131a437cedcf8802eb705223cce84278e6cb4c3655a7accd98097c06748b33d340f9de79920754616e295b88b833d9d84412ffc6a4406c60a6691ce5"
            "2c27b5b1c90d36a6a810cef4c81fdc3c75aaa87353433f2f3c7232c00d1198a79bb27df6edb89c5fd0a3a11e957c40")
                                << static_cast<uint32_t>(1746386404)
                                << QStringLiteral(
                                       "jX6TeGNIJFgecYIQU4YLSnhkbCpTihoIBB6Dw5iw7rAWQiULVB-NhOsfJwLh7EHW5wYwU1XRJAy5dw6JCWQ6v0R5SwP_84DIVDT5gfET_"
                                       "BoXmUr7rfXtJF62vpZ16XfwH2-P2KlaVRTaSYQ8xqTqC_fef6agOdYHvL_g-cNyjjv3xTMsekWnDCDhdG3Y1NvnPcXdAX9v0pjHUu5W5-"
                                       "k8iZhUeTcTjYhMODBPNXz6uf7mKd1wkwAsTnDZoL89k8U1asq78hyaiWsFb4Nb39vK0n0TZfb20yFoXZh9NRO2VpgS,dSTy86oaqm2ZjKu7FnnHmFU6R_"
                                       "lS8pk8EB6itIekPAfC37LYSBIEI1rT9cEvoXNkX6hIGnZ5sthiNPoi5nx9_HdjWYQ9R0Kar-Bgjeu77Av753T0GpVhJhYFyBkMe-"
                                       "NAqGMEjkYFjRUwOX9pEJGEszEL3_mWEbryQ8wL4ZaYk5xu4HAXe5hRwk-JUv__BW8IpiR_OsphQUgeKtRmXUPw1eIU2NYdsd3AJLAP3tiiENaplJ_y8X_"
                                       "OmC8tzvKCCdXapsas3-Won2R_ryQVJlB9j1tAARpNanIEwhOf9CHmbsYM,-8tTNfrKABIfOSlCdr2ajhLqF1i4hRiM-jSzISXAEmc-Nj75Rrg*");
    }

    void steamTicket()
    {
        QFETCH(QByteArray, ticketBytes);
        QFETCH(uint32_t, time);
        QFETCH(QString, encryptedTicket);

        QCOMPARE(encryptSteamTicket(ticketBytes, time), encryptedTicket);
    }
};

QTEST_MAIN(EncryptedArgTest)
#include "encryptedargtest.moc"
