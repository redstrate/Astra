// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "utility.h"

class UtilityTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testToWindowsPath()
    {
        QCOMPARE(Utility::toWindowsPath(QStringLiteral("/home/testuser/test.txt")), QStringLiteral("Z:\\home\\testuser\\test.txt"));
    }

    void testCreatePathIfNeeded()
    {
        QTemporaryDir dir;
        QString testDirPath = dir.filePath(QStringLiteral("test-dir"));

        QVERIFY(!QDir().exists(testDirPath));

        Utility::createPathIfNeeded(testDirPath);

        QVERIFY(QDir().exists(testDirPath));
    }

    void testReadVersion()
    {
        QTemporaryFile verFile;
        verFile.open();

        verFile.write(QByteArrayLiteral("2023.09.15.0000.0000"));
        verFile.flush();

        QCOMPARE(Utility::readVersion(verFile.fileName()), QStringLiteral("2023.09.15.0000.0000"));

        // lines should not affect the output
        verFile.write(QByteArrayLiteral("\r\n"));
        verFile.flush();

        QCOMPARE(Utility::readVersion(verFile.fileName()), QStringLiteral("2023.09.15.0000.0000"));
    }

    void testWriteVersion()
    {
        QTemporaryFile verFile;
        verFile.open();

        Utility::writeVersion(verFile.fileName(), QStringLiteral("2023.09.15.0000.0000"));
        QCOMPARE(Utility::readVersion(verFile.fileName()), QStringLiteral("2023.09.15.0000.0000"));
    }

    void testIsSteamDeck()
    {
        QCOMPARE(Utility::isSteamDeck(), false);

        qputenv("SteamDeck", "0");
        QCOMPARE(Utility::isSteamDeck(), false);

        qputenv("SteamDeck", "1");

        QCOMPARE(Utility::isSteamDeck(), true);
    }
};

QTEST_MAIN(UtilityTest)
#include "utilitytest.moc"
