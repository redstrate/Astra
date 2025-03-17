// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    property var account

    title: i18nc("@title:window", "Edit Account")

    actions: [
        Kirigami.Action {
            text: i18n("Open User Folder…")
            tooltip: i18n("The user folder contains saved appearance data and character settings for this account.")
            icon.name: "folder-open-symbolic"

            onTriggered: Qt.openUrlExternally("file://" + page.account.getConfigPath())
        },
        Kirigami.Action {
            text: i18n("Delete Account…")
            tooltip: !enabled ? i18n("Cannot delete the only account.") : ""
            icon.name: "delete"
            enabled: LauncherCore.accountManager.canDelete(page.account)

            onTriggered: deletePrompt.open()
        }
    ]

    header: Kirigami.NavigationTabBar {
        width: parent.width

        actions: [
            Kirigami.Action {
                id: generalAction
                text: i18n("General")
            },
            Kirigami.Action {
                id: accountAction
                text: i18n("Account")
            },
            Kirigami.Action {
                id: loginAction
                text: i18n("Login")
            }
        ]

        Component.onCompleted: actions[0].checked = true
    }

    FormCard.FormCard {
        visible: generalAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: usernameDelegate

            label: i18n("Username")
            text: page.account.config.name
            onTextChanged: page.account.config.name = text
        }

        FormCard.FormDelegateSeparator {
            above: usernameDelegate
            below: accountTypeDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: accountTypeDelegate

            text: i18n("Account type")
            model: ["Square Enix", "Sapphire"]
            currentIndex: page.account.config.isSapphire ? 1 : 0
            onCurrentIndexChanged: page.account.config.isSapphire = (currentIndex === 1)
        }

        FormCard.FormDelegateSeparator {
            above: accountTypeDelegate
            below: languageDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: languageDelegate

            text: i18n("Language")
            description: i18n("The language used in the game client.")
            model: ["Japanese", "English", "German", "French"]
            currentIndex: page.account.config.language
            onCurrentIndexChanged: page.account.config.language = currentIndex
        }
    }

    FormCard.FormCard {
        visible: accountAction.checked && !page.account.config.isSapphire

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormComboBoxDelegate {
            id: licenseField

            text: i18n("License")
            description: i18n("If the account holds multiple licenses, choose the preferred one.")
            model: ["Windows", "Steam", "macOS"]
            currentIndex: page.account.config.license
            onCurrentIndexChanged: page.account.config.license = currentIndex
        }

        FormCard.FormDelegateSeparator {
            above: licenseField
            below: freeTrialField
        }

        FormCard.FormCheckDelegate {
            id: freeTrialField
            text: i18n("Free trial")
            description: i18n("If the account has a free trial license.")
            checked: page.account.config.isFreeTrial
            onCheckedChanged: page.account.config.isFreeTrial = checked
        }

        FormCard.FormDelegateSeparator {
            above: freeTrialField
            below: needOTPField
        }

        FormCard.FormCheckDelegate {
            id: needOTPField

            text: i18n("Needs a one-time password")
            description: i18n("Prompt for the one-time password when logging in.")
            checked: page.account.config.useOTP
            onCheckedChanged: page.account.config.useOTP = checked
        }

        FormCard.FormDelegateSeparator {
            above: needOTPField
            below: lodestoneDelegate
        }

        FormCard.FormButtonDelegate {
            id: lodestoneDelegate

            text: i18n("Set Lodestone Character")
            description: i18n("Associate a character's avatar with this account for easier identification.")
            icon.name: "actor"
            Kirigami.PromptDialog {
                id: lodestoneDialog
                title: i18n("Enter Lodestone Id")

                standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

                onAccepted: page.account.config.lodestoneId = lodestoneIdField.text

                QQC2.TextField {
                    id: lodestoneIdField
                    text: page.account.config.lodestoneId
                    placeholderText: qsTr("123456...")
                }
            }

            onClicked: lodestoneDialog.open()
        }
    }

    FormCard.FormCard {
        visible: accountAction.checked && page.account.config.isSapphire

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: lobbyURLDelegate

            label: i18n("Lobby URL")
            text: page.account.config.lobbyUrl
            onTextChanged: page.account.config.lobbyUrl = text
            visible: page.account.config.isSapphire
            placeholderText: "neolobby0X.ffxiv.com"
        }
    }

    FormCard.FormCard {
        visible: loginAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: rememberPasswordDelegate

            text: i18n("Remember password")
            description: i18n("Stores the password on the device, using it's existing secure credential storage.")
            checked: page.account.config.rememberPassword
            onCheckedChanged: page.account.config.rememberPassword = checked
        }

        FormCard.FormDelegateSeparator {
            above: rememberPasswordDelegate
            below: generateOTPField
        }

        FormCard.FormCheckDelegate {
            id: generateOTPField

            text: i18n("Automatically generate one-time passwords")
            description: i18n("Stores the one-time password secret on this device, making it inherently insecure. Only use this feature if you understand the risks.")

            checked: page.account.config.rememberOTP
            onCheckedChanged: page.account.config.rememberOTP = checked
            enabled: page.account.config.useOTP
            visible: !page.account.config.isSapphire
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
            enabled: page.account.config.rememberOTP
            visible: generateOTPField.visible
            Kirigami.PromptDialog {
                id: otpDialog
                title: i18n("Enter OTP Secret")

                standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

                onAccepted: page.account.setOTPSecret(otpSecretField.text)

                QQC2.TextField {
                    id: otpSecretField
                    placeholderText: qsTr("ABCD EFGH...")
                }
            }

            onClicked: otpDialog.open()
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Account")
        subtitle: i18nc("@label", "Are you sure you want to delete this account?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false

        onAccepted: {
            LauncherCore.accountManager.deleteAccount(page.account);
            page.Window.window.pageStack.layers.pop();
        }
    }
}
