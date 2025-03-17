// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "Profiles")

    actions: [
        Kirigami.Action {
            text: i18n("Add Profile…")
            icon.name: "list-add"
            onTriggered: {
                page.Window.window.close();
                LauncherCore.currentProfile = LauncherCore.profileManager.addProfile();
            }
        }
    ]

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormButtonDelegate {
            text: i18n("Auto-login Profile")
            description: LauncherCore.autoLoginProfile ? LauncherCore.autoLoginProfile.config.name : i18n("Disabled")

            QQC2.Menu {
                id: profileMenu

                QQC2.MenuItem {
                    text: "Disabled"

                    onClicked: {
                        LauncherCore.autoLoginProfile = null;
                        profileMenu.close();
                    }
                }

                Repeater {
                    model: LauncherCore.profileManager

                    QQC2.MenuItem {
                        required property var profile

                        text: profile.config.name

                        onClicked: {
                            LauncherCore.autoLoginProfile = profile;
                            profileMenu.close();
                        }
                    }
                }
            }

            onClicked: profileMenu.popup()
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: LauncherCore.profileManager

            ColumnLayout {
                id: layout

                required property var profile
                required property int index

                spacing: 0

                FormCard.FormButtonDelegate {
                    id: buttonDelegate

                    text: layout.profile.config.name
                    description: layout.profile.subtitle
                    onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "ProfileSettings"), {
                        profile: layout.profile
                    })
                }

                FormCard.FormDelegateSeparator {
                    visible: layout.index + 1 < LauncherCore.profileManager.numProfiles
                }
            }
        }
    }
}
