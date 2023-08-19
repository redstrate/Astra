// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import zone.xiv.astra 1.0

import "../Components"

Kirigami.ScrollablePage {
    id: page

    title: i18n("Developer Settings")

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: keepPatchesDelegate

            text: i18n("Keep Patches")
            description: i18n("Do not delete patches after they're used. Astra will not redownload patch data, if found.")
            checked: LauncherCore.keepPatches
            onCheckedChanged: LauncherCore.keepPatches = checked
        }

        FormCard.FormDelegateSeparator {
            above: keepPatchesDelegate
            below: dalamudServerDelegate
        }

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