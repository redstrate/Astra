// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

import "../Components"

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "Developer Settings")

    FormCard.FormHeader {
        title: i18n("Patching")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: keepPatchesDelegate

            text: i18n("Keep Patches")
            description: i18n("Do not delete patches after they're used. Astra will not download patch data, if found.")
            checked: LauncherCore.settings.keepPatches
            onCheckedChanged: LauncherCore.settings.keepPatches = checked
        }
    }

    FormCard.FormHeader {
        title: i18n("Launching")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: encryptArgDelegate

            text: i18n("Encrypt Game Arguments")
            checked: LauncherCore.settings.argumentsEncrypted
            onCheckedChanged: LauncherCore.settings.argumentsEncrypted = checked
        }
    }

    FormCard.FormHeader {
        title: i18n("Servers")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextFieldDelegate {
            id: preferredProtocolDelegate

            label: i18n("Preferred Protocol")
            text: LauncherCore.settings.preferredProtocol
            onTextChanged: LauncherCore.settings.preferredProtocol = text
        }

        FormCard.FormDelegateSeparator {
            above: preferredProtocolDelegate
            below: dalamudServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: dalamudServerDelegate

            label: i18n("Dalamud Distribution Server")
            text: LauncherCore.settings.dalamudDistribServer
            onTextChanged: LauncherCore.settings.dalamudDistribServer = text
        }

        FormCard.FormDelegateSeparator {
            above: dalamudServerDelegate
            below: mainServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: mainServerDelegate

            label: i18n("SE Main Server")
            text: LauncherCore.settings.squareEnixServer
            onTextChanged: LauncherCore.settings.squareEnixServer = text
        }

        FormCard.FormDelegateSeparator {
            above: mainServerDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: loginServerDelegate

            label: i18n("SE Login Server")
            text: LauncherCore.settings.squareEnixLoginServer
            onTextChanged: LauncherCore.settings.squareEnixLoginServer = text
        }

        FormCard.FormDelegateSeparator {
            above: loginServerDelegate
            below: xivApiServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: xivApiServerDelegate

            label: i18n("XIV Api Server")
            text: LauncherCore.settings.xivApiServer
            onTextChanged: LauncherCore.settings.xivApiServer = text
        }
    }
}