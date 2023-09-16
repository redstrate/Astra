// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.mobileform as MobileForm

import zone.xiv.astra

Kirigami.Page {
    id: page

    property var profile

    title: i18n("Account Setup")

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Accounts")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Select an account below to use.")
                }

                Repeater {
                    model: LauncherCore.accountManager

                    MobileForm.FormButtonDelegate {
                        required property var account

                        text: account.name

                        onClicked: {
                            page.profile.account = account
                            applicationWindow().checkSetup()
                        }
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Square Enix Account")
                    icon.name: "list-add-symbolic"
                    onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"), {
                        profile: page.profile
                    })
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Sapphire Account")
                    icon.name: "list-add-symbolic"
                    onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AddSapphire"), {
                        profile: page.profile
                    })
                }
            }
        }
    }
}