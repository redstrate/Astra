// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    readonly property CompatibilityToolInstaller installer: LauncherCore.createCompatInstaller()

    title: i18nc("@title:window", "Compatibility Tool")

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextDelegate {
            text: i18n("Press the button below to install the compatibility tool for Steam.")
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: installToolButton

            text: i18n("Install Tool")
            icon.name: "install"
            visible: page.installer.hasSteam && !page.installer.isInstalled
            onClicked: page.installer.installCompatibilityTool()
        }

        FormCard.FormButtonDelegate {
            id: removeToolButton

            text: i18n("Remove Tool")
            icon.name: "delete"
            visible: page.installer.hasSteam && page.installer.isInstalled
            onClicked: page.installer.removeCompatibilityTool()
        }

        FormCard.FormTextDelegate {
            text: i18n("Steam is not installed.")
            visible: !page.installer.hasSteam
            textItem.color: Kirigami.Theme.disabledTextColor
        }
    }

    readonly property Kirigami.PromptDialog errorDialog: Kirigami.PromptDialog {
        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok
        parent: page.QQC2.Overlay.overlay
    }

    data: Connections {
        target: page.installer

        function onInstallFinished(): void {
            page.errorDialog.title = i18n("Install Success");
            page.errorDialog.subtitle = i18n("Compatibility tool successfully installed!");
            page.errorDialog.open();
        }

        function onError(message: string): void {
            page.errorDialog.title = i18n("Install Error");
            page.errorDialog.subtitle = message;
            page.errorDialog.open();
        }

        function onRemovalFinished(): void {
            page.errorDialog.title = i18n("Removal Success");
            page.errorDialog.subtitle = i18n("Compatibility tool successfully removed!");
            page.errorDialog.open();
        }
    }
}
