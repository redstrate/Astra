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

    title: i18n("Add Square Enix Account")

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
                    id: usernameField
                    label: i18n("Username")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormComboBoxDelegate {
                    id: licenseField
                    text: i18n("License")
                    description: i18n("If the account holds multiple licenses, choose the preferred one.")
                    model: ["Windows", "Steam", "macOS"]
                    currentIndex: 0
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    id: freeTrialField
                    text: i18n("Free Trial")
                    description: i18n("Check if the account is currently on free trial.")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Account")
                    icon.name: "list-add-symbolic"
                    onClicked: {
                        let account = LauncherCore.accountManager.createSquareEnixAccount(usernameField.text, licenseField.currentIndex, freeTrialField.checkState === Qt.Checked)
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