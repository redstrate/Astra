// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import zone.xiv.astra

import "Pages"

Kirigami.ApplicationWindow {
    id: appWindow

    width: 1280
    height: 720

    minimumWidth: 800
    minimumHeight: 500

    visible: true
    title: LauncherCore.isSteam ? "Astra (Steam)" : "Astra"

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

        if (!LauncherCore.currentProfile.isGameInstalled) {
            // User must set up the profile
            pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "SetupPage"), {
                profile: LauncherCore.currentProfile
            })
        } else if (!LauncherCore.currentProfile.account) {
            // User must select an account for the profile
            pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "AccountSetup"), {
                profile: LauncherCore.currentProfile
            })
        } else {
            if (LauncherCore.autoLoginProfile) {
                pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "AutoLoginPage"))
            } else {
                pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "MainPage"))
            }
        }
    }

    function cancelAutoLogin() {
        pageStack.layers.clear();
        pageStack.layers.replace(Qt.createComponent("zone.xiv.astra", "MainPage"));
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

        function onCurrentProfileChanged() {
            checkSetup();
        }
    }

    Component.onCompleted: checkSetup()

    property Item hoverLinkIndicator: QQC2.Control {
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
        contentItem: QQC2.Label {
            id: linkText
        }
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }
    }
}
