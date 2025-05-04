// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "crtrand.h"

class CrtRandTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void randomSeed_data()
    {
        QTest::addColumn<uint32_t>("seed");
        QTest::addColumn<uint32_t>("value1");
        QTest::addColumn<uint32_t>("value2");
        QTest::addColumn<uint32_t>("value3");
        QTest::addColumn<uint32_t>("value4");

        QTest::addRow("test 1") << static_cast<uint32_t>(5050) << static_cast<uint32_t>(16529) << static_cast<uint32_t>(23363) << static_cast<uint32_t>(25000)
                                << static_cast<uint32_t>(18427);
        QTest::addRow("test 2") << static_cast<uint32_t>(19147) << static_cast<uint32_t>(29796) << static_cast<uint32_t>(24416) << static_cast<uint32_t>(1377)
                                << static_cast<uint32_t>(24625);
    }

    void randomSeed()
    {
        QFETCH(uint32_t, seed);
        QFETCH(uint32_t, value1);
        QFETCH(uint32_t, value2);
        QFETCH(uint32_t, value3);
        QFETCH(uint32_t, value4);

        auto crtRand = CrtRand(seed);
        QCOMPARE(crtRand.next(), value1);
        QCOMPARE(crtRand.next(), value2);
        QCOMPARE(crtRand.next(), value3);
        QCOMPARE(crtRand.next(), value4);
    }
};

QTEST_MAIN(CrtRandTest)
#include "crtrandtest.moc"
