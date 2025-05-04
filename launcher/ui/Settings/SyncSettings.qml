// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

import "../Components"

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "Sync")

    FormCard.FormCard {
        id: infoCard

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextDelegate {
            id: infoDelegate

            text: i18n("Sync character data between devices using <a href='https://matrix.org'>Matrix</a>.")
        }

        FormCard.FormDelegateSeparator {
            above: infoDelegate
            below: enableSyncDelegate
        }

        FormCard.FormCheckDelegate {
            id: enableSyncDelegate

            text: i18n("Enable Sync")
            description: i18n("Syncing will occur before login, and after the game exits.")
            checked: LauncherCore.config.enableSync
            onCheckedChanged: {
                LauncherCore.config.enableSync = checked
                LauncherCore.config.save();
            }
        }
    }

    FormCard.FormCard {
        id: loginCard

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        visible: !LauncherCore.syncManager.connected
        enabled: LauncherCore.config.enableSync

        FormCard.FormTextFieldDelegate {
            id: usernameDelegate

            label: i18n("Username:")
            placeholderText: "@username:domain.com"
        }

        FormCard.FormDelegateSeparator {
            above: usernameDelegate
            below: passwordDelegate
        }

        FormCard.FormPasswordFieldDelegate {
            id: passwordDelegate

            label: i18n("Password:")
        }

        FormCard.FormDelegateSeparator {
            above: passwordDelegate
            below: loginButton
        }

        FormCard.FormButtonDelegate {
            id: loginButton

            text: i18n("Login")

            onClicked: LauncherCore.syncManager.login(usernameDelegate.text, passwordDelegate.text)
        }
    }

    FormCard.FormCard {
        id: logoutCard

        Layout.topMargin: Kirigami.Units.largeSpacing

        visible: LauncherCore.syncManager.connected

        FormCard.FormTextDelegate {
            id: usernameLabelDelegate

            text: i18n("Logged in as %1", LauncherCore.syncManager.userId)
        }

        FormCard.FormDelegateSeparator {
            above: usernameLabelDelegate
            below: initialSyncDelegate
        }

        FormCard.FormCheckDelegate {
            id: initialSyncDelegate

            text: i18n("Overwrite existing data")
            description: i18n("Temporarily overwrite any existing data on the server. This setting is not saved, and is reset when you log in.")
            checked: LauncherCore.syncManager.initialSync
            onCheckedChanged: LauncherCore.syncManager.initialSync = checked
        }

        FormCard.FormDelegateSeparator {
            above: initialSyncDelegate
            below: logoutDelegate
        }

        FormCard.FormButtonDelegate {
            id: logoutDelegate

            text: i18n("Log Out")
            onClicked: LauncherCore.syncManager.logout()
        }
    }

    Connections {
        target: LauncherCore.syncManager

        function onLoginError(message: string): void {
            errorDialog.subtitle = message;
            errorDialog.open();
        }
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Login Error")
        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok
        parent: page.QQC2.Overlay.overlay
    }
}
