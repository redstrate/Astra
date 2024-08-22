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

        auto profile = profileManager.getProfile(0);

        QCOMPARE(profileManager.getProfileByUUID(profile->uuid()), profile);
        QCOMPARE(profileManager.getProfileIndex(profile->uuid()), 0);

        QVERIFY(!profileManager.hasAnyExistingInstallations());
    }

    void testProfileManagement()
    {
        ProfileManager profileManager;

        QCOMPARE(profileManager.rowCount({}), 0);
        profileManager.load();

        profileManager.addProfile();

        QCOMPARE(profileManager.rowCount({}), 2);
        QCOMPARE(profileManager.numProfiles(), 2);
        QVERIFY(profileManager.canDelete(profileManager.getProfile(1)));

        profileManager.deleteProfile(profileManager.getProfile(1));

        QCOMPARE(profileManager.rowCount({}), 1);
        QCOMPARE(profileManager.numProfiles(), 1);
    }
};

QTEST_MAIN(ProfileManagerTest)
#include "profilemanagertest.moc"
