// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

Kirigami.ScrollablePage {
    id: page

    property var account

    title: i18n("Account Settings")

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("General")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    label: i18n("Username")
                    text: page.account.name
                    onTextChanged: page.account.name = text
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormComboBoxDelegate {
                    text: i18n("Account type")
                    model: ["Square Enix", "Sapphire"]
                    currentIndex: page.account.isSapphire ? 1 : 0
                    onCurrentIndexChanged: page.account.isSapphire = (currentIndex === 1)
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormComboBoxDelegate {
                    id: licenseField
                    text: i18n("License")
                    description: i18n("If the account holds multiple licenses, choose the preferred one.")
                    model: ["Windows", "Steam", "macOS"]
                    currentIndex: page.account.license
                    onCurrentIndexChanged: page.account.license = currentIndex
                    visible: !page.account.isSapphire
                }

                MobileForm.FormDelegateSeparator {
                    visible: licenseField.visible
                }

                MobileForm.FormCheckDelegate {
                    id: freeTrialField
                    text: i18n("Free trial")
                    checked: page.account.isFreeTrial
                    onCheckedChanged: page.account.isFreeTrial = checked
                    visible: !page.account.isSapphire
                }

                MobileForm.FormDelegateSeparator {
                    visible: freeTrialField.visible
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Needs a one-time password")
                    checked: page.account.useOTP
                    onCheckedChanged: page.account.useOTP = checked
                    visible: !page.account.isSapphire
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Set Lodestone Character")
                    description: i18n("Associate a character's avatar with this account.")
                    icon.name: "actor"
                    visible: !page.account.isSapphire
                    Kirigami.PromptDialog {
                        id: lodestoneDialog
                        title: i18n("Enter Lodestone Id")

                        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

                        onAccepted: page.account.lodestoneId = lodestoneIdField.text

                        Controls.TextField {
                            id: lodestoneIdField
                            text: page.account.lodestoneId
                            placeholderText: qsTr("123456...")
                        }
                    }

                    onClicked: lodestoneDialog.open()
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    label: i18n("Lobby URL")
                    text: page.account.lobbyUrl
                    onTextChanged: page.account.lobbyUrl = text
                    visible: page.account.isSapphire
                    placeholderText: "neolobby0X.ffxiv.com"
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Login")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Remember password")
                    checked: page.account.rememberPassword
                    onCheckedChanged: page.account.rememberPassword = checked
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Automatically generate one-time passwords")
                    checked: page.account.rememberOTP
                    onCheckedChanged: page.account.rememberOTP = checked
                    enabled: page.account.useOTP
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Enter OTP Secret")
                    icon.name: "list-add-symbolic"
                    enabled: page.account.rememberOTP
                    Kirigami.PromptDialog {
                        id: otpDialog
                        title: i18n("Enter OTP Secret")

                        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

                        onAccepted: page.account.setOTPSecret(otpSecretField.text)

                        Controls.TextField {
                            id: otpSecretField
                            placeholderText: qsTr("ABCD EFGH...")
                        }
                    }

                    onClicked: otpDialog.open()
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    text: i18n("Delete Account")
                    description: !enabled ? i18n("Cannot delete the only account") : ""
                    icon.name: "delete"
                    enabled: LauncherCore.accountManager.canDelete(page.account)
                    onClicked: {
                        LauncherCore.accountManager.deleteAccount(page.account)
                        applicationWindow().pageStack.layers.pop()
                    }
                }
            }
        }
    }
}