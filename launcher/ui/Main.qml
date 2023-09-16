// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import zone.xiv.astra 1.0

import "Pages"

Kirigami.ApplicationWindow {
    id: appWindow

    width: 1280
    height: 720

    minimumWidth: 800
    minimumHeight: 500

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
            pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "SetupPage"), {
                profile: currentSetupProfile
            })
        } else if (!currentSetupProfile.account) {
            // User must select an account for the profile
            pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "AccountSetup"), {
                profile: currentSetupProfile
            })
        } else {
            pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "MainPage"))
        }
    }

    function pushDialogLayer(url) {
        if (LauncherCore.isSteamDeck) {
            pageStack.layers.push(url)
        } else {
            pageStack.pushDialogLayer(url)
        }
    }

    function openUrl(url) {
        if (LauncherCore.isSteamDeck) {
            pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "BrowserPage"), {
                url: url
            })
        } else {
            Qt.openUrlExternally(url)
        }
    }

    Connections {
        target: LauncherCore

        function onLoadingFinished() {
            checkSetup();
        }

        function onSuccessfulLaunch() {
            if (LauncherCore.closeWhenLaunched) {
                hide();
            } else {
                checkSetup();
            }
        }

        function onGameClosed() {
            if (LauncherCore.closeWhenLaunched) {
                Qt.callLater(Qt.quit);
            } else {
                checkSetup();
            }
        }

        function onShowNewsChanged() {
            // workaround annoying Qt layouting bug
            // TODO: see if this changed in Qt6
            pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "MainPage"))
        }
    }

    Component.onCompleted: checkSetup()

    property Item hoverLinkIndicator: Controls.Control {
        parent: overlay.parent
        property alias text: linkText.text
        opacity: text.length > 0 ? 1 : 0

        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
            }
        }

        z: 999990
        x: 0
        y: parent.height - implicitHeight
        contentItem: Controls.Label {
            id: linkText
        }
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }
    }
}
