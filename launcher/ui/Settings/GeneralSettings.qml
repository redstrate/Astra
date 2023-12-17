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
            checked: LauncherCore.settings.closeWhenLaunched
            onCheckedChanged: LauncherCore.settings.closeWhenLaunched = checked
        }

        FormCard.FormDelegateSeparator {
            above: closeAstraDelegate
            below: showNewsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showNewsDelegate

            text: i18n("Enable and show news")
            checked: LauncherCore.settings.showNews
            onCheckedChanged: {
                LauncherCore.settings.showNews = checked;
                if (page.Window.window !== null) {
                    page.Window.window.close(); // if we don't close the dialog it crashes!
                }
            }
        }

        FormCard.FormDelegateSeparator {
            above: showNewsDelegate
            below: showDevToolsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showDevToolsDelegate

            text: i18n("Show Developer Settings")
            checked: LauncherCore.settings.showDevTools
            onCheckedChanged: LauncherCore.settings.showDevTools = checked
        }

        FormCard.FormDelegateSeparator {
            above: showDevToolsDelegate
            below: screenshotsPathDelegate
        }

        FormFolderDelegate {
            id: screenshotsPathDelegate

            text: i18n("Screenshots Folder")
            folder: LauncherCore.settings.screenshotDir

            onAccepted: (folder) => LauncherCore.settings.screenshotDir = folder
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