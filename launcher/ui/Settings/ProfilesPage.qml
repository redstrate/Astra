// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "Profiles")

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: LauncherCore.profileManager

            ColumnLayout {
                required property var profile
                required property int index

                spacing: 0

                FormCard.FormButtonDelegate {
                    id: buttonDelegate

                    text: profile.name
                    onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "ProfileSettings"), {
                        profile: profile
                    })
                }

                FormCard.FormDelegateSeparator {
                    visible: index + 1 < LauncherCore.profileManager.numProfiles()
                }
            }
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: addProfileButton

            text: i18n("Add Profile")
            icon.name: "list-add"
            onClicked: {
                page.Window.window.close();
                LauncherCore.currentProfile = LauncherCore.profileManager.addProfile();
            }
        }
    }
}