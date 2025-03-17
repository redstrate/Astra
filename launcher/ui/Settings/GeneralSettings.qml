// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "General")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: closeAstraDelegate

            text: i18n("Hide Astra when game is launched")
            checked: LauncherCore.config.closeWhenLaunched
            onCheckedChanged: LauncherCore.config.closeWhenLaunched = checked
        }

        FormCard.FormDelegateSeparator {
            above: closeAstraDelegate
            below: showDevToolsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showDevToolsDelegate

            text: i18n("Show Developer Settings")
            description: i18n("Enable settings that are useful for developers and tinkerers.")
            checked: LauncherCore.config.showDevTools
            onCheckedChanged: LauncherCore.config.showDevTools = checked
        }

        FormCard.FormDelegateSeparator {
            above: showDevToolsDelegate
            below: screenshotsPathDelegate
        }

        FormFolderDelegate {
            id: screenshotsPathDelegate

            text: i18n("Screenshots Folder")
            folder: LauncherCore.config.screenshotDir

            onAccepted: (folder) => LauncherCore.config.screenshotDir = folder
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            text: i18n("Clear Lodestone Avatar Cache")
            description: i18n("Refreshes the avatars for all accounts, and requires Astra to restart to take effect.")
            icon.name: "delete"

            onClicked: LauncherCore.clearAvatarCache()
        }
    }
}
