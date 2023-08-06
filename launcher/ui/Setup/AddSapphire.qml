// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import zone.xiv.astra 1.0

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
                            applicationWindow().pageStack.layers.pop()
                        }
                    }
                }
            }
        }
    }
}