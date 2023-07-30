// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import com.redstrate.astra 1.0

import "Pages"

Kirigami.ApplicationWindow {
    id: appWindow

    width: 1280
    height: 720
    visible: true
    title: LauncherCore.isSteam ? "Astra (Steam)" : "Astra"

    property var currentSetupProfile: LauncherCore.profileManager.getProfile(0)

    pageStack.initialPage: Kirigami.Page {
        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
        }
    }

    function checkSetup() {
        if (!LauncherCore.loadingFinished) {
            return
        }

        pageStack.layers.clear()

        if (!currentSetupProfile.isGameInstalled) {
            // User must set up the profile
            pageStack.layers.replace('qrc:/ui/Setup/SetupPage.qml', {
                profile: currentSetupProfile
            })
        } else if (!currentSetupProfile.account) {
            // User must select an account for the profile
            pageStack.layers.replace('qrc:/ui/Setup/AccountSetup.qml', {
                profile: currentSetupProfile
            })
        } else {
            pageStack.layers.replace('qrc:/ui/Pages/MainPage.qml')
        }
    }

    Connections {
        target: LauncherCore

        function onLoadingFinished() {
            checkSetup()
        }
    }

    Component.onCompleted: checkSetup()
}
