// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: LauncherCore.accountManager

            ColumnLayout {
                required property var account
                required property int index

                spacing: 0

                FormCard.FormButtonDelegate {
                    text: account.name
                    description: account.isSapphire ? i18n("Sapphire") : i18n("Square Enix")

                    leading: Components.Avatar {
                        source: account.avatarUrl
                    }

                    leadingPadding: Kirigami.Units.largeSpacing * 2

                    onClicked: root.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AccountSettings"), {
                        account: account
                    })
                }

                FormCard.FormDelegateSeparator {
                    visible: index + 1 < LauncherCore.accountManager.numAccounts()
                }
            }
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: addSquareEnixButton

            text: i18n("Add Square Enix Account")
            icon.name: "list-add-symbolic"
            onClicked: root.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"))
        }

        FormCard.FormDelegateSeparator {
            above: addSquareEnixButton
            below: addSapphireButton
        }

        FormCard.FormButtonDelegate {
            id: addSapphireButton

            text: i18n("Add Sapphire Account")
            icon.name: "list-add-symbolic"
            onClicked: root.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSapphire"))
        }
    }
}