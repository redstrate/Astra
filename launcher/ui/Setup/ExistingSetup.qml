// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

import "../Components"

FormCard.FormCardPage {
    id: page

    property var profile

    title: i18n("Find Existing Installation")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            text: i18n("Please select the path to your existing installation.")
        }

        FormCard.FormDelegateSeparator {
            above: helpTextDelegate
            below: selectDelegate
        }

        FormCard.FormButtonDelegate {
            id: selectDelegate

            text: i18n("Select Existing Path")
            icon.name: "document-open-folder"
            onClicked: dialog.open()
        }
    }

    data: FolderDialog {
        id: dialog

        onAccepted: {
            page.profile.gamePath = decodeURIComponent(selectedFolder.toString().replace("file://", ""));
            applicationWindow().checkSetup();
        }
    }
}