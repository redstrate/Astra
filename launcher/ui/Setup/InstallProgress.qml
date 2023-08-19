// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import zone.xiv.astra 1.0

Kirigami.Page {
    property var gameInstaller

    title: i18n("Game Installation")

    Kirigami.LoadingPlaceholder {
        anchors.centerIn: parent

        text: i18n("Installing...")
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Install error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().pageStack.layers.pop()
        onRejected: applicationWindow().pageStack.layers.pop()
    }

    Component.onCompleted: gameInstaller.installGame()

    Connections {
        target: gameInstaller

        function onInstallFinished() {
            applicationWindow().checkSetup()
        }

        function onError(message) {
            errorDialog.subtitle = message
            errorDialog.open()
        }
    }
}