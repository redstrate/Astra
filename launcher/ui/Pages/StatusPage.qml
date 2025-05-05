// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window

import org.kde.kirigami as Kirigami

import zone.xiv.astra

Kirigami.Page {
    id: root

    property var gameInstaller

    title: i18n("Logging in...")

    onBackRequested: (event) => {
        if (LauncherCore.isPatching()) {
            // Prevent going back
            applicationWindow().showPassiveNotification(i18n("Please do not quit while patching!"));
            event.accepted = true;
        }
    }

    Kirigami.LoadingPlaceholder {
        id: placeholder

        text: "Logging in..."

        anchors.centerIn: parent
    }

    Kirigami.PromptDialog {
        id: errorDialog

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().checkSetup()
        onRejected: applicationWindow().checkSetup()
    }

    Kirigami.PromptDialog {
        id: dalamudErrorDialog
        title: i18n("Dalamud Error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.Cancel

        onAccepted: {
            LauncherCore.currentProfile.config.dalamudEnabled = false;
            applicationWindow().checkSetup();
        }
        onRejected: applicationWindow().checkSetup()
    }

    Kirigami.PromptDialog {
        id: updateDialog

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.Cancel

        onAccepted: LauncherCore.updateDecided(true)
        onRejected: {
            LauncherCore.updateDecided(false);
            applicationWindow().checkSetup();
        }
    }

    Connections {
        target: LauncherCore

        function onStageChanged(message: string, explanation: string): void {
            placeholder.text = message;
            placeholder.explanation = explanation;
        }

        function onStageIndeterminate(): void {
            placeholder.determinate = false;
        }

        function onStageDeterminate(min: int, max: int, value: int): void {
            placeholder.determinate = true;
            placeholder.progressBar.value = value;
            placeholder.progressBar.from = min;
            placeholder.progressBar.to = max;
        }

        function onLoginError(message: string): void {
            errorDialog.title = i18n("Login Error");
            errorDialog.subtitle = message;
            errorDialog.open();
        }

        function onMiscError(message: string): void {
            errorDialog.title = i18n("Error");
            errorDialog.subtitle = message;
            errorDialog.open();
        }

        function onDalamudError(message: string): void {
            dalamudErrorDialog.subtitle = i18n("An error occurred while updating Dalamud:\n\n%1.\n\nWould you like to disable Dalamud?", message);
            dalamudErrorDialog.open();
        }

        function onRequiresUpdate(message: string): void {
            updateDialog.title = i18n("Update Required");
            updateDialog.subtitle = message;
            updateDialog.open();
        }
    }
}
