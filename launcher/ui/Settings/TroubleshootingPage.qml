// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

import "../Components"

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "Troubleshooting")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: verboseLoggingDelegate

            text: i18n("Enable Verbose Logging")
            description: i18n("Requires a restart of Astra to take effect.")
            checked: LauncherCore.config.verboseLogging
            onCheckedChanged: {
                LauncherCore.config.verboseLogging = checked;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: verboseLoggingDelegate
            below: logsFolderDelegate
        }

        FormCard.FormButtonDelegate {
            id: logsFolderDelegate

            text: i18nc("@action:button", "Open Logs Folder")
            onClicked: Qt.openUrlExternally(StandardPaths.writableLocation(StandardPaths.AppDataLocation) + "/log")
        }
    }
}
