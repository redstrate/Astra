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
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            text: i18n("Select an account to use for '%1'.", LauncherCore.currentProfile.name)
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

                text: account.name

                onClicked: {
                    page.profile.account = account
                    applicationWindow().checkSetup()
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Create New Account")
        visible: LauncherCore.accountManager.hasAnyAccounts()
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: addSquareEnixButton

            text: i18n("Add Square Enix Account…")
            description: i18n("Used for logging into the official game servers.")
            icon.name: "list-add-symbolic"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"), {
                profile: page.profile
            })
        }

        FormCard.FormDelegateSeparator {
            above: addSquareEnixButton
            below: addSapphireButton
        }

        FormCard.FormButtonDelegate {
            id: addSapphireButton

            text: i18n("Add Sapphire Account…")
            description: i18n("Only used for Sapphire servers, don't select this if unless you plan to connect to one.")
            icon.name: "list-add-symbolic"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSapphire"), {
                profile: page.profile
            })
        }
    }
}