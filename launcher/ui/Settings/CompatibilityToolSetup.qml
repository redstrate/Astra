// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kquickcontrolsaddons as KQuickControlsAddons

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

    readonly property Kirigami.Action copyCommandAction: Kirigami.Action {
        icon.name: "edit-copy-symbolic"
        text: i18n("Copy Command")
        onTriggered: clipboard.content = "flatpak override com.valvesoftware.Steam --talk-name=org.freedesktop.Flatpak"
    }

    readonly property KQuickControlsAddons.Clipboard clipboard: KQuickControlsAddons.Clipboard {}

    data: Connections {
        target: page.installer

        function onInstallFinished(flatpak: bool): void {
            page.errorDialog.title = i18n("Successful Installation");
            if (flatpak) {
                page.errorDialog.subtitle = i18n("You need to relaunch Steam for Astra to show up as a Compatibility Tool.\n\nSince you are using the Steam Flatpak, you must give it permissions to allow it to launch Astra:\nflatpak override com.valvesoftware.Steam --talk-name=org.freedesktop.Flatpak");
                page.errorDialog.customFooterActions = [copyCommandAction];
            } else {
                page.errorDialog.subtitle = i18n("You need to relaunch Steam for Astra to show up as a Compatibility Tool.");
                page.errorDialog.customFooterActions = [];
            }
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
