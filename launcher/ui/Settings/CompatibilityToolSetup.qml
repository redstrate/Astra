// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    property var installer: null

    title: i18n("Install Compatibility Tool")

    FormCard.FormHeader {
        title: i18n("Compatibility Tool")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            text: i18n("Press the button below to install the compatibility tool for Steam.")
        }

        FormCard.FormDelegateSeparator {
            below: installToolButton
        }

        FormCard.FormButtonDelegate {
            id: installToolButton

            text: i18n("Install Tool")
            icon.name: "install"
            onClicked: {
                page.installer = LauncherCore.createCompatInstaller();
                page.installer.installCompatibilityTool();
            }
        }

        FormCard.FormDelegateSeparator {
            above: installToolButton
            below: removeToolButton
        }

        FormCard.FormButtonDelegate {
            id: removeToolButton

            text: i18n("Remove Tool")
            icon.name: "delete"
            onClicked: {
                page.installer = LauncherCore.createCompatInstaller();
                page.installer.removeCompatibilityTool();
            }
        }
    }

    property Kirigami.PromptDialog errorDialog: Kirigami.PromptDialog {
        title: i18n("Install error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().pageStack.layers.pop()
        onRejected: applicationWindow().pageStack.layers.pop()
    }

    data: Connections {
        enabled: page.installer !== null
        target: page.installer

        function onInstallFinished() {
            applicationWindow().pageStack.layers.pop()
        }

        function onError(message) {
            errorDialog.subtitle = message
            errorDialog.open()
        }
    }
}