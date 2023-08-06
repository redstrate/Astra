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
                    onClicked: pageStack.layers.push('qrc:/ui/Setup/AddSquareEnix.qml', {
                        profile: page.profile
                    })
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Sapphire Account")
                    icon.name: "list-add-symbolic"
                    onClicked: pageStack.layers.push('qrc:/ui/Setup/AddSapphire.qml', {
                        profile: page.profile
                    })
                }
            }
        }
    }
}