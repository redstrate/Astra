// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components

import zone.xiv.astra

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Accounts")

    actions: [
        Kirigami.Action {
            text: i18n("Add Account…")
            icon.name: "list-add-symbolic"

            Kirigami.Action {
                text: i18n("Square Enix")
                onTriggered: root.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"))
            }
            Kirigami.Action {
                text: i18n("Sapphire")
                onTriggered: root.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSapphire"))
            }
        }
    ]

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: LauncherCore.accountManager

            ColumnLayout {
                id: layout

                required property var account
                required property int index

                spacing: 0

                FormCard.FormButtonDelegate {
                    text: layout.account.name
                    description: layout.account.isSapphire ? i18n("Sapphire") : i18n("Square Enix")

                    leading: Components.Avatar {
                        name: layout.account.name
                        source: layout.account.avatarUrl
                    }

                    leadingPadding: Kirigami.Units.largeSpacing * 2

                    onClicked: root.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AccountSettings"), {
                        account: layout.account
                    })
                }

                FormCard.FormDelegateSeparator {
                    visible: layout.index + 1 < LauncherCore.accountManager.numAccounts
                }
            }
        }
    }
}