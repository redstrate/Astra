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
            text: i18n("Installing Astra as a Compatibility Tool in Steam lets you bypass the official launcher.\n\nThis is needed to use Astra with a Steam service account.")
            textItem.wrapMode: Text.WordWrap
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: installToolButton

            text: i18n("Install Compatibility Tool")
            icon.name: "install"
            visible: page.installer.hasSteam && !page.installer.isInstalled
            onClicked: page.installer.installCompatibilityTool()
        }

        FormCard.FormButtonDelegate {
            id: removeToolButton

            text: i18n("Remove Compatibility Tool")
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
            page.errorDialog.title = i18n("Successful Installation");
            page.errorDialog.subtitle = i18n("You need to relaunch Steam for Astra to show up as a Compatibility Tool.");
            page.errorDialog.open();
        }

        function onError(message: string): void {
            page.errorDialog.title = i18n("Installation Error");
            page.errorDialog.subtitle = message;
            page.errorDialog.open();
        }

        function onRemovalFinished(): void {
            page.errorDialog.title = i18n("Successful Removal");
            page.errorDialog.subtitle = i18n("You need to relaunch Steam for Astra to stop showing up as a Compatibility Tool.");
            page.errorDialog.open();
        }
    }
}
