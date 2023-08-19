// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import zone.xiv.astra 1.0

FormCard.FormCardPage {
    id: page

    property var account

    title: i18n("Account Settings")

    FormCard.FormHeader {
        title: i18n("General")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextFieldDelegate {
            id: usernameDelegate

            label: i18n("Username")
            text: page.account.name
            onTextChanged: page.account.name = text
        }

        FormCard.FormDelegateSeparator {
            above: usernameDelegate
            below: languageDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: languageDelegate

            text: i18n("Language")
            model: ["Japanese", "English", "German", "French"]
            currentIndex: page.account.language
            onCurrentIndexChanged: page.account.language = currentIndex
        }

        FormCard.FormDelegateSeparator {
            above: languageDelegate
            below: accountTypeDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: accountTypeDelegate

            text: i18n("Account type")
            model: ["Square Enix", "Sapphire"]
            currentIndex: page.account.isSapphire ? 1 : 0
            onCurrentIndexChanged: page.account.isSapphire = (currentIndex === 1)
        }

        FormCard.FormDelegateSeparator {
            above: accountTypeDelegate
            below: licenseField
        }

        FormCard.FormComboBoxDelegate {
            id: licenseField

            text: i18n("License")
            description: i18n("If the account holds multiple licenses, choose the preferred one.")
            model: ["Windows", "Steam", "macOS"]
            currentIndex: page.account.license
            onCurrentIndexChanged: page.account.license = currentIndex
            visible: !page.account.isSapphire
        }

        FormCard.FormDelegateSeparator {
            above: licenseField
            below: freeTrialField

            visible: licenseField.visible
        }

        FormCard.FormCheckDelegate {
            id: freeTrialField
            text: i18n("Free trial")
            checked: page.account.isFreeTrial
            onCheckedChanged: page.account.isFreeTrial = checked
            visible: !page.account.isSapphire
        }

        FormCard.FormDelegateSeparator {
            above: freeTrialField
            below: needOTPField

            visible: freeTrialField.visible
        }

        FormCard.FormCheckDelegate {
            id: needOTPField

            text: i18n("Needs a one-time password")
            checked: page.account.useOTP
            onCheckedChanged: page.account.useOTP = checked
            visible: !page.account.isSapphire
        }

        FormCard.FormDelegateSeparator {
            above: needOTPField
            below: lobbyURLDelegate

            visible: needOTPField.visible
        }

        FormCard.FormTextFieldDelegate {
            id: lobbyURLDelegate

            label: i18n("Lobby URL")
            text: page.account.lobbyUrl
            onTextChanged: page.account.lobbyUrl = text
            visible: page.account.isSapphire
            placeholderText: "neolobby0X.ffxiv.com"
        }

        FormCard.FormDelegateSeparator {
            above: lobbyURLDelegate
            below: lodestoneDelegate
        }

        FormCard.FormButtonDelegate {
            id: lodestoneDelegate

            text: i18n("Set Lodestone Character")
            description: i18n("Associate a character's avatar with this account.")
            icon.name: "actor"
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
    }

    FormCard.FormHeader {
        title: i18n("Login")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: rememberPasswordDelegate

            text: i18n("Remember password")
            checked: page.account.rememberPassword
            onCheckedChanged: page.account.rememberPassword = checked
        }

        FormCard.FormDelegateSeparator {
            above: rememberPasswordDelegate
            below: generateOTPField
        }

        FormCard.FormCheckDelegate {
            id: generateOTPField

            text: i18n("Automatically generate one-time passwords")
            checked: page.account.rememberOTP
            onCheckedChanged: page.account.rememberOTP = checked
            enabled: page.account.useOTP
            visible: !page.account.isSapphire
        }

        FormCard.FormDelegateSeparator {
            above: generateOTPField
            below: otpSecretDelegate

            visible: generateOTPField.visible
        }

        FormCard.FormButtonDelegate {
            id: otpSecretDelegate

            text: i18n("Enter OTP Secret")
            icon.name: "list-add-symbolic"
            enabled: page.account.rememberOTP
            visible: generateOTPField.visible
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

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            text: i18n("Delete Account")
            description: !enabled ? i18n("Cannot delete the only account.") : ""
            icon.name: "delete"
            enabled: LauncherCore.accountManager.canDelete(page.account)
            onClicked: {
                LauncherCore.accountManager.deleteAccount(page.account)
                applicationWindow().pageStack.layers.pop()
            }
        }
    }
}