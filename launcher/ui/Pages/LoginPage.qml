// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

Kirigami.OverlayDrawer {
    id: page

    property var profile: LauncherCore.profileManager.getProfile(0)
    readonly property bool isLoginValid: {
        if (!profile.account) {
            return false
        }

        if (usernameField.text.length === 0) {
            return false
        }

        if (!profile.account.rememberPassword && passwordField.text.length === 0) {
            return false
        }

        if (profile.account.useOTP && !profile.account.rememberOTP && otpField.text.length === 0) {
            return false
        }

        return true;
    }

    function updateFields() {
        usernameField.text = profile.account.name
        passwordField.text = profile.account.rememberPassword ? profile.account.getPassword() : ""
        otpField.text = ""
    }

    Connections {
        target: profile

        function onAccountChanged() {
            updateFields()
        }
    }

    onProfileChanged: updateFields()

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    text: page.profile.name

                    Controls.Menu {
                        id: profileMenu

                        Repeater {
                            model: LauncherCore.profileManager

                            Controls.MenuItem {
                                required property var profile

                                Controls.MenuItem {
                                    text: profile.name

                                    onClicked: {
                                        page.profile = profile
                                        profileMenu.close()
                                    }
                                }
                            }
                        }
                    }

                    onClicked: profileMenu.popup()
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    text: page.profile.account.name

                    leading: Kirigami.Avatar
                        {
                        source: page.profile.account.avatarUrl
                    }

                    leadingPadding: Kirigami.Units.largeSpacing * 2

                    Controls.Menu {
                        id: accountMenu

                        Repeater {
                            model: LauncherCore.accountManager

                            Controls.MenuItem {
                                required property var account

                                Controls.MenuItem {
                                    text: account.name
                                    icon.name: account.avatarUrl.length === 0 ? "actor" : ""
                                    icon.source: account.avatarUrl

                                    onClicked: {
                                        page.profile.account = account
                                        accountMenu.close()
                                    }
                                }
                            }
                        }
                    }

                    onClicked: accountMenu.popup()
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    id: usernameField
                    label: i18n("Username")
                    text: page.profile.account.name
                    enabled: false
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    id: passwordField
                    label: i18n("Password")
                    echoMode: TextInput.Password
                    focus: true
                    onAccepted: otpField.clicked()
                    text: page.profile.account.rememberPassword ? "abcdefg" : ""
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextFieldDelegate {
                    id: otpField
                    label: i18n("One-time password")
                    visible: page.profile.account.useOTP
                    onAccepted: loginButton.clicked()
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    id: loginButton

                    text: i18n("Log In")
                    icon.name: "unlock"
                    enabled: page.isLoginValid
                    onClicked: {
                        LauncherCore.login(page.profile, usernameField.text, passwordField.text, otpField.text)
                        pageStack.layers.push('qrc:/ui/Pages/StatusPage.qml')
                    }
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Settings")
                    icon.name: "configure"
                    onClicked: pageStack.pushDialogLayer('qrc:/ui/Settings/SettingsPage.qml')
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Open Official Launcher")
                    icon.name: "application-x-executable"
                    onClicked: LauncherCore.openOfficialLauncher(page.profile)
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Open System Info")
                    icon.name: "application-x-executable"
                    onClicked: LauncherCore.openSystemInfo(page.profile)
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Open Config Backup")
                    icon.name: "application-x-executable"
                    onClicked: LauncherCore.openConfigBackup(page.profile)
                }
            }
        }
    }
}