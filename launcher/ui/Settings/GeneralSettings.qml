// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    title: i18nc("@title:window", "General")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            text: i18n("Auto-login Profile")
            description: LauncherCore.autoLoginProfile ? LauncherCore.autoLoginProfile.name : i18n("Disabled")

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

                        text: profile.name

                        onClicked: {
                            LauncherCore.autoLoginProfile = profile;
                            profileMenu.close();
                        }
                    }
                }
            }

            onClicked: profileMenu.popup()
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormCheckDelegate {
            id: closeAstraDelegate

            text: i18n("Close Astra when game is launched")
            checked: LauncherCore.closeWhenLaunched
            onCheckedChanged: LauncherCore.closeWhenLaunched = checked
        }

        FormCard.FormDelegateSeparator {
            above: closeAstraDelegate
            below: showNewsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showNewsDelegate

            text: i18n("Enable and show news")
            checked: LauncherCore.showNews
            onCheckedChanged: LauncherCore.showNews = checked
        }

        FormCard.FormDelegateSeparator {
            above: showNewsDelegate
            below: showDevToolsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showDevToolsDelegate

            text: i18n("Show Developer Settings")
            checked: LauncherCore.showDevTools
            onCheckedChanged: LauncherCore.showDevTools = checked
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