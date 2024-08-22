// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "profilemanager.h"

class ProfileManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDummyProfile()
    {
        ProfileManager profileManager;

        QCOMPARE(profileManager.rowCount({}), 0);
        profileManager.load();

        // the dummy profile
        QCOMPARE(profileManager.rowCount({}), 1);
        QCOMPARE(profileManager.numProfiles(), 1);
        QVERIFY(!profileManager.canDelete(profileManager.getProfile(0))); // the first profile can never be deleted
    }
};

QTEST_MAIN(ProfileManagerTest)
#include "profilemanagertest.moc"
