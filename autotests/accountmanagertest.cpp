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
        AccountManager accountManager;
        QAbstractItemModelTester modelTester(&accountManager);

        const QSignalSpy accountAddedSpy(&accountManager, &AccountManager::accountAdded);
        QVERIFY(accountAddedSpy.isValid());

        QCOMPARE(accountManager.rowCount({}), 0);
        accountManager.load();
        QCOMPARE(accountAddedSpy.count(), 0); // no accounts were added

        // There should be no profiles, not even a dummy one
        QCOMPARE(accountManager.rowCount({}), 0);

        // These functions shouldn't crash and return empty when there's no acccounts
        QCOMPARE(accountManager.getByUuid(QString{}), nullptr);
        QVERIFY(!accountManager.canDelete());
    }

    void testAccountManagement()
    {
        AccountManager accountManager;
        QAbstractItemModelTester modelTester(&accountManager);

        const QSignalSpy accountAddedSpy(&accountManager, &AccountManager::accountAdded);
        QVERIFY(accountAddedSpy.isValid());

        QCOMPARE(accountManager.rowCount({}), 0);
        accountManager.load();
        QCOMPARE(accountAddedSpy.count(), 0);

        accountManager.createSquareEnixAccount(QStringLiteral("foo"), static_cast<int>(Account::GameLicense::WindowsStandalone), true);

        QCOMPARE(accountAddedSpy.count(), 1);
        QCOMPARE(accountManager.rowCount({}), 1);

        auto account = accountManager.data(accountManager.index(0, 0), AccountManager::AccountRole).value<Account *>();
        QVERIFY(!accountManager.canDelete());

        // TODO: maybe deleteAccount shouldn't work when canDelete == false?
        accountManager.deleteAccount(account);

        QCOMPARE(accountAddedSpy.count(), 1); // but not here, because nothing was added obviously
        QCOMPARE(accountManager.rowCount({}), 0);
    }
};

QTEST_MAIN(AccountManagerTest)
#include "accountmanagertest.moc"
