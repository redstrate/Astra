// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    property var profile

    title: i18n("Account Setup")

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing * 4
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            text: i18n("Select an account to use for '%1'.", LauncherCore.currentProfile.config.name)
        }
    }

    FormCard.FormHeader {
        title: i18n("Existing Accounts")
        visible: LauncherCore.accountManager.hasAnyAccounts()
    }

    FormCard.FormCard {
        visible: LauncherCore.accountManager.hasAnyAccounts()

        Layout.fillWidth: true

        Repeater {
            model: LauncherCore.accountManager

            FormCard.FormButtonDelegate {
                required property var account

                text: account.config.name

                onClicked: {
                    page.profile.account = account
                    applicationWindow().checkSetup()
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Add Account")
        visible: LauncherCore.accountManager.hasAnyAccounts()
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: addSquareEnixButton

            text: i18n("Add New Account…")
            icon.name: "list-add-symbolic"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"), {
                profile: page.profile
            })
        }
    }
}
