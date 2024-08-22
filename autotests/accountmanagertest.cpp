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

        const QSignalSpy accountsChangedSpy(&accountManager, &AccountManager::accountsChanged);
        QVERIFY(accountsChangedSpy.isValid());

        const QSignalSpy accountAddedSpy(&accountManager, &AccountManager::accountAdded);
        QVERIFY(accountAddedSpy.isValid());

        QCOMPARE(accountManager.rowCount({}), 0);
        accountManager.load();
        QCOMPARE(accountsChangedSpy.count(), 0); // no accounts were changed
        QCOMPARE(accountAddedSpy.count(), 0); // no accounts were added

        // There should be no profiles, not even a dummy one
        QCOMPARE(accountManager.rowCount({}), 0);
        QCOMPARE(accountManager.numAccounts(), 0);
        QVERIFY(!accountManager.hasAnyAccounts());

        // These functions shouldn't crash and return empty when there's no acccounts
        QCOMPARE(accountManager.getByUuid(QString{}), nullptr);
        QVERIFY(!accountManager.canDelete(nullptr));
    }

    void testAccountManagement_data()
    {
        QTest::addColumn<bool>("isSapphire");

        QTest::addRow("se") << false;
        QTest::addRow("sapphire") << true;
    }

    void testAccountManagement()
    {
        AccountManager accountManager;
        QAbstractItemModelTester modelTester(&accountManager);

        const QSignalSpy accountsChangedSpy(&accountManager, &AccountManager::accountsChanged);
        QVERIFY(accountsChangedSpy.isValid());

        const QSignalSpy accountAddedSpy(&accountManager, &AccountManager::accountAdded);
        QVERIFY(accountAddedSpy.isValid());

        QCOMPARE(accountManager.rowCount({}), 0);
        accountManager.load();
        QCOMPARE(accountsChangedSpy.count(), 0);
        QCOMPARE(accountAddedSpy.count(), 0);
        QVERIFY(!accountManager.hasAnyAccounts());

        QFETCH(bool, isSapphire);

        if (isSapphire) {
            accountManager.createSapphireAccount(QStringLiteral("foo.com"), QStringLiteral("foo"));
        } else {
            accountManager.createSquareEnixAccount(QStringLiteral("foo"), static_cast<int>(Account::GameLicense::WindowsStandalone), true);
        }

        QCOMPARE(accountsChangedSpy.count(), 1);
        QCOMPARE(accountAddedSpy.count(), 1);
        QCOMPARE(accountManager.rowCount({}), 1);
        QCOMPARE(accountManager.numAccounts(), 1);
        QVERIFY(accountManager.hasAnyAccounts());

        auto account = accountManager.data(accountManager.index(0, 0), AccountManager::AccountRole).value<Account *>();
        QVERIFY(!accountManager.canDelete(account));

        // TODO: maybe deleteAccount shouldn't work when canDelete == false?
        accountManager.deleteAccount(account);

        QCOMPARE(accountsChangedSpy.count(), 2); // this signal should be called
        QCOMPARE(accountAddedSpy.count(), 1); // but not here, because nothing was added obviously
        QCOMPARE(accountManager.rowCount({}), 0);
        QCOMPARE(accountManager.numAccounts(), 0);
        QVERIFY(!accountManager.hasAnyAccounts());
    }
};

QTEST_MAIN(AccountManagerTest)
#include "accountmanagertest.moc"
