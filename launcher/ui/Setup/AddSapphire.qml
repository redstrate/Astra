// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.mobileform as MobileForm

import zone.xiv.astra

Kirigami.Page {
    id: page

    property var profile

    title: i18n("Add Sapphire Account")

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormTextDelegate {
                    description: i18n("Passwords will be entered on the login page. The username will be associated with this profile but can be changed later.")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    id: lobbyUrlField
                    label: i18n("Lobby URL")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    id: usernameField
                    label: i18n("Username")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Account")
                    icon.name: "list-add-symbolic"
                    onClicked: {
                        let account = LauncherCore.accountManager.createSapphireAccount(lobbyUrlField.text, usernameField.text)
                        if (page.profile) {
                            page.profile.account = account
                            applicationWindow().checkSetup()
                        } else {
                            page.Window.window.pageStack.layers.pop()
                        }
                    }
                }
            }
        }
    }
}