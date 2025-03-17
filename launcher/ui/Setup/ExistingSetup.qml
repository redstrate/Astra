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

    data: FolderDialog {
        id: dialog

        onAccepted: {
            page.profile.config.gamePath = decodeURIComponent(selectedFolder.toString().replace("file://", ""));
            applicationWindow().checkSetup();
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: ExistingInstallModel {}

            delegate: FormCard.FormButtonDelegate {
                required property var path
                required property var type

                text: path
                description: type

                onClicked: {
                    page.profile.config.gamePath = path;
                    applicationWindow().checkSetup();
                }
            }
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            text: i18n("If you can't find your existing game installation, manually select the folder below.")
        }
        FormCard.FormDelegateSeparator {
            above: helpTextDelegate
            below: selectDelegate
        }
        FormCard.FormButtonDelegate {
            id: selectDelegate

            icon.name: "document-open-folder"
            text: i18n("Select Existing Folder")

            onClicked: dialog.open()
        }
    }
}

