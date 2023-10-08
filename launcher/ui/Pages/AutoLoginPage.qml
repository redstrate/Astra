// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import zone.xiv.astra

Kirigami.Page {
    id: root

    title: i18n("Auto Login")

    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    Kirigami.PlaceholderMessage {
        id: placeholderMessage

        anchors.centerIn: parent

        text: i18n("Logging in...")

        helpfulAction: Kirigami.Action {
            icon.name: "Cancel"
            text: "Cancel"
            enabled: autoLoginTimer.running
            onTriggered: {
                autoLoginTimer.stop();
                applicationWindow().cancelAutoLogin();
            }
        }
    }

    Timer {
        id: autoLoginTimer

        interval: 5000
        running: true
        onTriggered: {
            autoLoginTimer.stop();
            if (LauncherCore.autoLogin(LauncherCore.autoLoginProfile)) {
                pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "StatusPage"));
            }
        }
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Login Error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().cancelAutoLogin()
        onRejected: applicationWindow().cancelAutoLogin()
    }

    Connections {
        target: LauncherCore

        function onLoginError(message) {
            errorDialog.subtitle = message
            errorDialog.open()
        }
    }
}
