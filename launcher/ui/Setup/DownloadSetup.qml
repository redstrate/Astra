// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    property var profile

    title: i18n("Download Game")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            text: i18n("Press the button below to download and setup the game.")
            description: i18n("This only installs the base files required for the initial update. Astra can only download patches with a legitimate Square Enix account.")
        }

        FormCard.FormDelegateSeparator {
            above: helpTextDelegate
            below: buttonDelegate
        }

        FormCard.FormButtonDelegate {
            id: buttonDelegate

            text: i18n("Begin installation")
            icon.name: "cloud-download"
            onClicked: page.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "InstallProgress"), {
                gameInstaller: LauncherCore.createInstaller(page.profile)
            })
        }
    }
}