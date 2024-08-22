// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "accountmanager.h"

class AccountManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testNothing()
    {
        AccountManager profileManager;
        QAbstractItemModelTester modelTester(&profileManager);

        QCOMPARE(profileManager.rowCount({}), 0);
        profileManager.load();

        // There should be no profiles, not even a dummy one
        QCOMPARE(profileManager.rowCount({}), 0);
        QCOMPARE(profileManager.numAccounts(), 0);
        QVERIFY(!profileManager.hasAnyAccounts());

        // These functions shouldn't crash and return empty when there's no acccounts
        QCOMPARE(profileManager.getByUuid(QString{}), nullptr);
        QVERIFY(!profileManager.canDelete(nullptr));
    }
};

QTEST_MAIN(AccountManagerTest)
#include "accountmanagertest.moc"
