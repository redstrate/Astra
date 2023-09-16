// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kirigamiaddons.components as Components
import zone.xiv.astra 1.0

FormCard.FormCardPage {
    FormCard.FormHeader {
        title: i18n("Accounts")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            model: LauncherCore.accountManager

            FormCard.FormButtonDelegate {
                required property var account

                text: account.name

                leading: Components.Avatar
                    {
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