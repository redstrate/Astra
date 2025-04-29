// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import zone.xiv.astra

Kirigami.Page {
    id: page

    property var benchmarkInstaller

    title: i18n("Benchmark Installation")

    Kirigami.LoadingPlaceholder {
        anchors.centerIn: parent

        text: i18n("Downloading…")
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Installation Error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: page.Window.window.pageStack.layers.pop()
        onRejected: page.Window.window.pageStack.layers.pop()
    }

    Component.onCompleted: benchmarkInstaller.start()

    Connections {
        target: page.benchmarkInstaller

        function onInstallFinished(): void {
            // Prevents it from failing to push the page if the installation happens too quickly.
            Qt.callLater(() => applicationWindow().checkSetup());
        }

        function onError(message: string): void {
            errorDialog.subtitle = i18n("An error has occurred while installing the benchmark:\n\n%1", message);
            errorDialog.open();
        }
    }
}
