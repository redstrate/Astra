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

    title: i18n("Developer Settings")

    FormCard.FormHeader {
        title: i18n("Patching")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: keepPatchesDelegate

            text: i18n("Keep Patches")
            description: i18n("Do not delete patches after they're used. Astra will not redownload patch data, if found.")
            checked: LauncherCore.keepPatches
            onCheckedChanged: LauncherCore.keepPatches = checked
        }
    }

    FormCard.FormHeader {
        title: i18n("Servers")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextFieldDelegate {
            id: dalamudServerDelegate

            label: i18n("Dalamud Distribution Server")
            text: LauncherCore.dalamudDistribServer
            onTextChanged: LauncherCore.dalamudDistribServer = text
        }

        FormCard.FormDelegateSeparator {
            above: dalamudServerDelegate
            below: mainServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: mainServerDelegate

            label: i18n("SE Main Server")
            text: LauncherCore.squareEnixServer
            onTextChanged: LauncherCore.squareEnixServer = text
        }

        FormCard.FormDelegateSeparator {
            above: mainServerDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: loginServerDelegate

            label: i18n("SE Login Server")
            text: LauncherCore.squareEnixLoginServer
            onTextChanged: LauncherCore.squareEnixLoginServer = text
        }
    }
}