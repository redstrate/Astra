// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components

import zone.xiv.astra

FormCard.FormCardPage {
    title: i18nc("@title:window", "Accounts")

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        Repeater {
            model: LauncherCore.accountManager

            FormCard.FormButtonDelegate {
                required property var account

                text: account.name

                leading: Components.Avatar {
                    source: account.avatarUrl
                }

                leadingPadding: Kirigami.Units.largeSpacing * 2

                onClicked: applicationWindow().pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AccountSettings"), {
                    account: account
                })
            }
        }

        FormCard.FormDelegateSeparator {
            below: addSquareEnixButton
        }

        FormCard.FormButtonDelegate {
            id: addSquareEnixButton

            text: i18n("Add Square Enix Account")
            icon.name: "list-add-symbolic"
            onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"))
        }

        FormCard.FormDelegateSeparator {
            above: addSquareEnixButton
            below: addSapphireButton
        }

        FormCard.FormButtonDelegate {
            id: addSapphireButton

            text: i18n("Add Sapphire Account")
            icon.name: "list-add-symbolic"
            onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSapphire"))
        }
    }
}