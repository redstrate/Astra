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

    property var profile

    title: i18n("Add Sapphire Account")

    readonly property bool isValid: usernameField.text.length !== 0 && lobbyUrlField.text.length !== 0

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: helpTextDelegate
            description: i18n("The password will be entered on the login page. A username will be associated with this account but can always be changed later.")
        }

        FormCard.FormDelegateSeparator {
            above: helpTextDelegate
            below: usernameField
        }

        FormCard.FormTextFieldDelegate {
            id: usernameField
            label: i18n("Username")
        }

        FormCard.FormDelegateSeparator {
            above: usernameField
            below: lobbyUrlField
        }

        FormCard.FormTextFieldDelegate {
            id: lobbyUrlField
            label: i18n("Lobby URL")
        }

        FormCard.FormDelegateSeparator {
            above: lobbyUrlField
            below: buttonDelegate
        }

        FormCard.FormButtonDelegate {
            id: buttonDelegate
            text: i18n("Add Account")
            icon.name: "list-add-symbolic"
            enabled: page.isValid
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