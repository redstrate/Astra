// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components

import zone.xiv.astra

QQC2.Control {
    id: page

    readonly property bool isLoginValid: {
        if (!LauncherCore.currentProfile.account) {
            return false
        }

        if (usernameField.text.length === 0) {
            return false
        }

        if (!LauncherCore.currentProfile.account.rememberPassword && passwordField.text.length === 0) {
            return false
        }

        if (LauncherCore.currentProfile.account.useOTP && !LauncherCore.currentProfile.account.rememberOTP && otpField.text.length === 0) {
            return false
        }

        if (LauncherCore.currentProfile.loggedIn) {
            return false;
        }

        return true;
    }

    function updateFields() {
        usernameField.text = LauncherCore.currentProfile.account.name
        passwordField.text = LauncherCore.currentProfile.account.rememberPassword ? LauncherCore.currentProfile.account.getPassword() : ""
        otpField.text = ""
    }

    Connections {
        target: LauncherCore

        function onCurrentProfileChanged() {
            updateFields();
        }
    }

    Connections {
        target: LauncherCore.currentProfile

        function onAccountChanged() {
            updateFields()
        }
    }

    contentItem: ColumnLayout {
        width: parent.width

        FormCard.FormCard {
            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                text: i18n("Current Profile")
                description: LauncherCore.currentProfile.name

                QQC2.Menu {
                    id: profileMenu

                    Repeater {
                        model: LauncherCore.profileManager

                        QQC2.MenuItem {
                            required property var profile

                            QQC2.MenuItem {
                                text: profile.name

                                onClicked: {
                                    LauncherCore.currentProfile = profile
                                    profileMenu.close()
                                }
                            }
                        }
                    }
                }

                onClicked: profileMenu.popup()
            }
        }

        FormCard.FormCard {
            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                text: i18n("Current Account")
                description: LauncherCore.currentProfile.account.name

                leading: Components.Avatar
                {
                    source: LauncherCore.currentProfile.account.avatarUrl
                }

                leadingPadding: Kirigami.Units.largeSpacing * 2

                QQC2.Menu {
                    id: accountMenu

                    Repeater {
                        model: LauncherCore.accountManager

                        QQC2.MenuItem {
                            required property var account

                            QQC2.MenuItem {
                                text: account.name
                                icon.name: account.avatarUrl.length === 0 ? "actor" : ""
                                icon.source: account.avatarUrl

                                onClicked: {
                                    LauncherCore.currentProfile.account = account
                                    accountMenu.close()
                                }
                            }
                        }
                    }
                }

                onClicked: accountMenu.popup()
            }

            FormCard.FormDelegateSeparator {
            }

            FormCard.FormTextFieldDelegate {
                id: usernameField
                label: LauncherCore.currentProfile.account.isSapphire ? i18n("Username") : i18n("Square Enix ID")
                text: LauncherCore.currentProfile.account.name
                enabled: false
            }

            FormCard.FormDelegateSeparator {
            }

            FormCard.FormTextFieldDelegate {
                id: passwordField
                label: LauncherCore.currentProfile.account.isSapphire ? i18n("Password") : i18n("Square Enix Password")
                echoMode: TextInput.Password
                focus: true
                onAccepted: {
                    if (otpField.visible) {
                        otpField.clicked();
                    } else {
                        loginButton.clicked();
                    }
                }
                text: LauncherCore.currentProfile.account.rememberPassword ? LauncherCore.currentProfile.account.getPassword() : ""
            }

            FormCard.FormDelegateSeparator {
            }

            FormCard.FormTextFieldDelegate {
                id: otpField
                label: i18n("One-time Password")
                visible: LauncherCore.currentProfile.account.useOTP
                onAccepted: loginButton.clicked()
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormButtonDelegate {
                id: loginButton

                text: i18n("Log In")
                icon.name: "unlock"
                enabled: page.isLoginValid
                onClicked: {
                    LauncherCore.login(LauncherCore.currentProfile, usernameField.text, passwordField.text, otpField.text)
                    pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "StatusPage"))
                }
            }

            FormCard.FormDelegateSeparator {
                visible: forgotPasswordButton.visible
            }

            FormCard.FormButtonDelegate {
                id: forgotPasswordButton

                text: i18n("Forgot ID or Password")
                icon.name: "dialog-password"
                visible: !LauncherCore.currentProfile.account.isSapphire
                onClicked: applicationWindow().openUrl('https://secure.square-enix.com/account/app/svc/reminder')
            }
        }
    }

    Component.onCompleted: passwordField.forceActiveFocus()
}