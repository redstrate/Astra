// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.mobileform as MobileForm

import zone.xiv.astra

Kirigami.Page {
    id: page

    property var profile

    title: i18n("Game Setup")

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Welcome to Astra")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("The game must be installed to continue. Please select a setup option below.")
                    description: i18n("A valid game account will be required at the end of installation.")
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    text: i18n("Find Existing Installation")
                    icon.name: "edit-find"
                    onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "ExistingSetup"), {
                        profile: page.profile
                    })
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Download Game")
                    icon.name: "cloud-download"
                    onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "DownloadSetup"), {
                        profile: page.profile
                    })
                }
            }
        }
    }
}