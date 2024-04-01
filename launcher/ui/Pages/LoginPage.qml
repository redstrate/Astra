// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components as Components

import zone.xiv.astra

QQC2.Control {
    id: page

    readonly property string invalidLoginReason: {
        if (!LauncherCore.currentProfile.account) {
            return i18n("Profile has no associated account.");
        }

        if (usernameField.text.length === 0) {
            return i18n("Username is required.");
        }

        if (LauncherCore.currentProfile.account.needsPassword && passwordField.text.length === 0) {
            return i18n("Password is required.");
        }

        if (LauncherCore.currentProfile.account.useOTP && !LauncherCore.currentProfile.account.rememberOTP && otpField.text.length === 0) {
            return i18n("OTP is required.");
        }

        if (LauncherCore.currentProfile.loggedIn) {
            return i18n("Already logged in.");
        }

        return "";
    }

    readonly property bool isLoginValid: {
        if (!LauncherCore.currentProfile.account) {
            return false;
        }

        if (usernameField.text.length === 0) {
            return false;
        }

        if (LauncherCore.currentProfile.account.needsPassword && passwordField.text.length === 0) {
            return false;
        }

        if (LauncherCore.currentProfile.account.useOTP && !LauncherCore.currentProfile.account.rememberOTP && otpField.text.length === 0) {
            return false;
        }

        if (LauncherCore.currentProfile.loggedIn) {
            return false;
        }

        return true;
    }

    function updateFields() {
        usernameField.text = LauncherCore.currentProfile.account.name;
        passwordField.text = !LauncherCore.currentProfile.account.needsPassword && LauncherCore.currentProfile.account.rememberPassword ? LauncherCore.currentProfile.account.getPassword() : "";
        otpField.text = "";
    }

    Connections {
        target: LauncherCore

        function onCurrentProfileChanged() {
            page.updateFields();
            LauncherCore.refreshLogoImage();
        }
    }

    Connections {
        target: LauncherCore.currentProfile

        function onAccountChanged() {
            page.updateFields();

            if (LauncherCore.currentProfile.account.needsPassword) {
                passwordField.forceActiveFocus();
                return;
            }

            if (LauncherCore.currentProfile.account.useOTP) {
                otpField.forceActiveFocus();
                return;
            }

            loginButton.forceActiveFocus();
        }
    }

    contentItem: ColumnLayout {
        width: parent.width

        spacing: Kirigami.Units.largeSpacing

        Image {
            readonly property real aspectRatio: sourceSize.height / sourceSize.width

            fillMode: Image.PreserveAspectFit
            source: "file://" + LauncherCore.cachedLogoImage
            verticalAlignment: Image.AlignTop
            sourceClipRect: Qt.rect(0, sourceSize.height / 2, sourceSize.width, sourceSize.height / 2)

            Component.onCompleted: LauncherCore.refreshLogoImage()
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: width * aspectRatio
        }

        FormCard.FormCard {
            maximumWidth: Kirigami.Units.gridUnit * 25
            visible: LauncherCore.profileManager.numProfiles > 1

            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                id: currentProfileDelegate

                text: LauncherCore.currentProfile.name

                QQC2.Menu {
                    id: profileMenu

                    modal: true

                    Repeater {
                        model: LauncherCore.profileManager

                        QQC2.MenuItem {
                            id: profileMenuItem

                            required property var profile

                            QQC2.MenuItem {
                                text: profileMenuItem.profile.name

                                onClicked: {
                                    LauncherCore.currentProfile = profileMenuItem.profile;
                                    profileMenu.close();
                                }
                            }
                        }
                    }
                }

                onClicked: profileMenu.popup(currentProfileDelegate, currentProfileDelegate.width - profileMenu.width, currentProfileDelegate.height)
            }
        }

        FormCard.FormCard {
            maximumWidth: Kirigami.Units.gridUnit * 25

            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                id: currentAccountDelegate

                enabled: LauncherCore.accountManager.numAccounts > 1
                text: LauncherCore.currentProfile.account.name

                leading: Components.Avatar {
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium

                    name: LauncherCore.currentProfile.account.name
                    source: LauncherCore.currentProfile.account.avatarUrl
                }

                leadingPadding: Kirigami.Units.largeSpacing * 2

                QQC2.Menu {
                    id: accountMenu

                    modal: true

                    Repeater {
                        model: LauncherCore.accountManager

                        QQC2.MenuItem {
                            id: menuItem

                            required property var account

                            QQC2.MenuItem {
                                text: menuItem.account.name
                                icon.name: menuItem.account.avatarUrl.length === 0 ? "actor" : ""
                                icon.source: menuItem.account.avatarUrl

                                onClicked: {
                                    LauncherCore.currentProfile.account = menuItem.account;
                                    accountMenu.close();
                                }
                            }
                        }
                    }
                }

                onClicked: accountMenu.popup(currentAccountDelegate, currentAccountDelegate.width - accountMenu.width, currentAccountDelegate.height)
            }

            FormCard.FormDelegateSeparator {
                above: currentAccountDelegate
                below: usernameField
            }

            FormCard.FormTextFieldDelegate {
                id: usernameField
                label: LauncherCore.currentProfile.account.isSapphire ? i18n("Username") : i18n("Square Enix ID")
                text: LauncherCore.currentProfile.account.name
                enabled: false

                QQC2.ToolTip.text: i18n("The username can only be changed under account settings.")
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }

            FormCard.FormDelegateSeparator {
                above: usernameField
                below: passwordField
            }

            FormCard.FormTextFieldDelegate {
                id: passwordField
                label: LauncherCore.currentProfile.account.isSapphire ? i18n("Password") : i18n("Square Enix Password")
                echoMode: TextInput.Password
                enabled: LauncherCore.currentProfile.account.rememberPassword ? LauncherCore.currentProfile.account.needsPassword : true
                focus: true
                onAccepted: {
                    if (otpField.visible) {
                        otpField.clicked();
                    } else {
                        loginButton.clicked();
                    }
                }
                text: (!LauncherCore.currentProfile.account.needsPassword && LauncherCore.currentProfile.account.rememberPassword) ? LauncherCore.currentProfile.account.getPassword() : ""
            }

            FormCard.FormDelegateSeparator {
                above: passwordField
                below: LauncherCore.currentProfile.account.useOTP ? otpField : loginButton
            }

            FormCard.FormTextFieldDelegate {
                id: otpField
                label: i18n("One-time Password")
                visible: LauncherCore.currentProfile.account.useOTP
                onAccepted: {
                    if (page.isLoginValid) {
                        loginButton.clicked()
                    }
                }
            }

            FormCard.FormDelegateSeparator {
                above: LauncherCore.currentProfile.account.useOTP ? otpField : passwordField
                below: loginButton
                visible: LauncherCore.currentProfile.account.useOTP
            }

            FormCard.FormButtonDelegate {
                id: loginButton

                text: i18n("Log In")
                description: page.invalidLoginReason
                icon.name: "unlock"
                enabled: page.isLoginValid
                onClicked: {
                    LauncherCore.login(LauncherCore.currentProfile, usernameField.text, passwordField.text, otpField.text)
                    page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "StatusPage"))
                }
            }

            FormCard.FormDelegateSeparator {
                above: loginButton
                below: forgotPasswordButton
                visible: !LauncherCore.currentProfile.account.isSapphire
            }

            FormCard.FormButtonDelegate {
                id: forgotPasswordButton

                text: i18n("Forgot ID or Password")
                icon.name: "dialog-password"
                visible: !LauncherCore.currentProfile.account.isSapphire
                onClicked: applicationWindow().openUrl('https://secure.square-enix.com/account/app/svc/reminder')
            }
        }

        Item {
            Layout.fillHeight: true
        }

        FormCard.FormCard {
            maximumWidth: Kirigami.Units.gridUnit * 25

            Layout.alignment: Qt.AlignBottom

            FormCard.FormButtonDelegate {
                text: i18nc("@action:button", "The Lodestone")
                icon.name: "internet-services-symbolic"
                visible: !LauncherCore.currentProfile.account.isSapphire
                // TODO: how do we link to a "worldwide" lodestone, if that even exists?
                onClicked: applicationWindow().openUrl('https://na.finalfantasyxiv.com/lodestone/')
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormButtonDelegate {
                text: i18nc("@action:button", "Mog Station")
                icon.name: "internet-services-symbolic"
                visible: !LauncherCore.currentProfile.account.isSapphire
                onClicked: applicationWindow().openUrl('https://secure.square-enix.com/account/app/svc/mogstation/')
            }
        }
    }
}