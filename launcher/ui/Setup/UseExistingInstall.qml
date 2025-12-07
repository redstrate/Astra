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

    title: i18n("Select Existing Installation")

    data: FolderDialog {
        id: dialog

        onAccepted: {
            page.profile.config.gamePath = decodeURI(selectedFolder.toString().replace("file://", "").substr(Qt.platform.os === "windows" ? 1 : 0));
            applicationWindow().checkSetup();
        }
    }

    FormCard.FormCard {
        visible: existingInstallRepeater.count > 0

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextDelegate {
            id: foundTextDelegate

            text: i18n("Astra has found the following installations:")
        }

        FormCard.FormDelegateSeparator {
            above: foundTextDelegate
        }

        Repeater {
            id: existingInstallRepeater
            model: ExistingInstallModel {}

            delegate: FormCard.FormButtonDelegate {
                required property string path
                required property string type
                required property string version

                text: type
                description: i18nc("version (path)", "%1 (%2)", version, path)

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

            text: i18n("If Astra did not detect your installation automatically, try selecting it manually below.")
            textItem.wrapMode: Text.WordWrap
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
