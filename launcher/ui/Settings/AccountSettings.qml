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

    required property Account account

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
                icon.name: "configure-symbolic"
            },
            Kirigami.Action {
                id: accountAction
                text: i18n("Account")
                icon.name: "user-symbolic"
            },
            Kirigami.Action {
                id: loginAction
                text: i18n("Login")
                icon.name: "password-copy-symbolic"
            },
            Kirigami.Action {
                id: developerAction
                text: i18n("Developer")
                visible: LauncherCore.config.showDevTools
                icon.name: "applications-development-symbolic"
            }
        ]

        Component.onCompleted: actions[0].checked = true
    }

    Connections {
        target: page.account

        function onAutoConfigurationResult(title: string, subtitle: string): void {
            configurationPrompt.title = title;
            configurationPrompt.subtitle = subtitle;
            configurationPrompt.visible = true;
        }
    }

    FormCard.FormCard {
        visible: generalAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: usernameDelegate

            label: i18n("Username")
            text: page.account.config.name
            onTextChanged: {
                page.account.config.name = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: usernameDelegate
            below: languageDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: languageDelegate

            text: i18n("Language")
            description: i18n("The language used in the game client.")
            model: ["Japanese", "English", "German", "French"]
            currentIndex: page.account.config.language
            onCurrentIndexChanged: {
                page.account.config.language = currentIndex;
                page.account.config.save();
            }
        }
    }

    FormCard.FormCard {
        visible: accountAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormComboBoxDelegate {
            id: licenseField

            text: i18n("License")
            description: i18n("If the account holds multiple licenses, choose the preferred one.")
            model: ["Windows", "Steam", "macOS"]
            currentIndex: page.account.config.license
            onCurrentIndexChanged: {
                page.account.config.license = currentIndex;
                page.account.config.save();
            }
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
            onCheckedChanged: {
                page.account.config.isFreeTrial = checked;
                page.account.config.save();
            }
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
            onCheckedChanged: {
                page.account.config.useOTP = checked;
                page.account.config.save();
            }
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
                parent: page.QQC2.Overlay.overlay

                onAccepted: {
                    page.account.config.lodestoneId = lodestoneIdField.text;
                    page.account.config.save();
                }

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
        visible: loginAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: rememberPasswordDelegate

            text: i18n("Remember password")
            description: i18n("Stores the password on the device, using it's existing secure credential storage.")
            checked: page.account.config.rememberPassword
            onCheckedChanged: {
                page.account.config.rememberPassword = checked;
                page.account.config.save();
            }
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
            onCheckedChanged: {
                page.account.config.rememberOTP = checked;
                page.account.config.save();
            }
            enabled: page.account.config.useOTP
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
                parent: page.QQC2.Overlay.overlay

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
    FormCard.FormCard {
        visible: developerAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: serverUrlDelegate

            label: i18n("Server URL")
            placeholderText: "http://ffxiv.localhost"
        }

        FormCard.FormDelegateSeparator {
            above: serverUrlDelegate
            below: downloadConfigDelegate
        }

        FormCard.FormButtonDelegate {
            id: downloadConfigDelegate

            icon.name: "download-symbolic"
            text: i18nc("@action:button", "Download Configuration")
            enabled: serverUrlDelegate.text.length > 0

            onClicked: LauncherCore.downloadServerConfiguration(page.account, serverUrlDelegate.text)
        }
    }

    FormCard.FormCard {
        visible: developerAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormTextFieldDelegate {
            id: squareMainServerDelegate

            label: i18n("Frontier Server")
            text: page.account.config.frontierServer
            placeholderText: page.account.config.defaultFrontierServerValue
            onTextChanged: {
                page.account.config.frontierSrver = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: squareMainServerDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: loginServerDelegate

            label: i18n("Login Server")
            text: page.account.config.loginServer
            placeholderText: page.account.config.defaultLoginServerValue
            onTextChanged: {
                page.account.config.loginServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: loginServerDelegate
            below: mainServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: mainServerDelegate

            label: i18n("Lodestone Server")
            text: page.account.config.lodestoneServer
            placeholderText: page.account.config.defaultLodestoneServerValue
            onTextChanged: {
                page.account.config.lodestoneServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: loginServerDelegate
            below: bootPatchServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: bootPatchServerDelegate

            label: i18n("Boot Patch Server")
            text: page.account.config.bootPatchServer
            placeholderText: page.account.config.defaultBootPatchServerValue
            onTextChanged: {
                page.account.config.bootPatchServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: bootPatchServerDelegate
            below: gamePatchServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gamePatchServerDelegate

            label: i18n("Game Patch Server")
            text: page.account.config.gamePatchServer
            placeholderText: page.account.config.defaultGamePatchServerValue
            onTextChanged: {
                page.account.config.gamePatchServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: gamePatchServerDelegate
            below: gameServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gameServerDelegate

            label: i18n("Lobby Server")
            text: page.account.config.lobbyServer
            placeholderText: i18nc("@info:placeholder", "(Default value in client)")
            onTextChanged: {
                page.account.config.lobbyServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: gameServerDelegate
            below: gameServerPortDelegate
        }

        FormCard.FormSpinBoxDelegate {
            id: gameServerPortDelegate

            label: i18n("Lobby Port")
            value: page.account.config.lobbyPort
            onValueChanged: {
                page.account.config.lobbyPort = value;
                page.account.config.save();
            }
            from: 0
            to: 65535
        }

        FormCard.FormDelegateSeparator {
            above: gameServerPortDelegate
            below: frontierServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: frontierServerDelegate

            label: i18n("Frontier Server")
            text: page.account.config.frontierServer
            placeholderText: i18nc("@info:placeholder", "(Default value in client)")
            onTextChanged: {
                page.account.config.frontierServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: frontierServerDelegate
            below: saveDataBankServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: saveDataBankServerDelegate

            label: i18n("Save Data Bank Server")
            text: page.account.config.saveDataBankServer
            placeholderText: i18nc("@info:placeholder", "(Default value in client)")
            onTextChanged: {
                page.account.config.saveDataBankServer = text;
                page.account.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: saveDataBankServerDelegate
            below: saveDataBankPortDelegate
        }

        FormCard.FormSpinBoxDelegate {
            id: saveDataBankPortDelegate

            label: i18n("Save Data Bank Port")
            value: page.account.config.saveDataBankPort
            onValueChanged: {
                page.account.config.saveDataBankPort = value;
                page.account.config.save();
            }
            from: 0
            to: 999999
        }

        FormCard.FormDelegateSeparator {
            above: saveDataBankPortDelegate
            below: dataCenterTravelServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: dataCenterTravelServerDelegate

            label: i18n("Data Center Travel Server")
            text: page.account.config.dataCenterTravelServer
            placeholderText: i18nc("@info:placeholder", "(Default value in client)")
            onTextChanged: {
                page.account.config.dataCenterTravelServer = text;
                page.account.config.save();
            }
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Account")
        subtitle: i18nc("@label", "Are you sure you want to delete this account?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay

        onAccepted: {
            LauncherCore.accountManager.deleteAccount(page.account);
            page.Window.window.pageStack.layers.pop();
        }
    }

    Kirigami.PromptDialog {
        id: configurationPrompt

        standardButtons: Kirigami.Dialog.Ok
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay
    }
}
