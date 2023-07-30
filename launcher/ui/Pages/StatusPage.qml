// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import com.redstrate.astra 1.0

Kirigami.Page {
    property var gameInstaller

    title: i18n("Logging in...")

    Kirigami.LoadingPlaceholder {
        id: placeholder

        anchors.centerIn: parent
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Login error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().pageStack.layers.pop()
        onRejected: applicationWindow().pageStack.layers.pop()
    }

    Connections {
        target: LauncherCore

        function onStageChanged(message) {
            placeholder.text = message
        }

        function onLoginError(message) {
            errorDialog.subtitle = message
            errorDialog.open()
        }
    }
}