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

    bottomPadding: Kirigami.Units.largeSpacing

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

        if (LauncherCore.currentProfile.account.config.useOTP && !LauncherCore.currentProfile.account.config.rememberOTP && otpField.text.length === 0) {
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

        if (LauncherCore.currentProfile.account.config.useOTP && !LauncherCore.currentProfile.account.config.rememberOTP && otpField.text.length === 0) {
            return false;
        }

        return !LauncherCore.currentProfile.loggedIn;
    }

    function updateFields(): void {
        usernameField.text = LauncherCore.currentProfile.account.config.name;
        passwordField.text = !LauncherCore.currentProfile.account.needsPassword && LauncherCore.currentProfile.account.config.rememberPassword ? LauncherCore.currentProfile.account.getPassword() : "";
        if (LauncherCore.currentProfile.account.config.rememberOTP) {
            otpField.text = "Auto-generated";
        } else {
            otpField.text = "";
        }
    }

    Connections {
        target: LauncherCore.currentProfile

        function onAccountChanged(): void {
            page.updateFields();

            if (LauncherCore.currentProfile.account.needsPassword) {
                passwordField.forceActiveFocus();
                return;
            }

            if (LauncherCore.currentProfile.account.config.useOTP) {
                otpField.forceActiveFocus();
                return;
            }

            loginButton.forceActiveFocus();
        }
    }

    Component.onCompleted: updateFields()

    contentItem: ColumnLayout {
        width: parent.width

        spacing: Kirigami.Units.largeSpacing

        Image {
            readonly property real aspectRatio: sourceSize.height / sourceSize.width

            fillMode: Image.PreserveAspectFit
            source: "file://" + LauncherCore.cachedLogoImage
            verticalAlignment: Image.AlignTop
            sourceClipRect: Qt.rect(0, sourceSize.height / 2, sourceSize.width, sourceSize.height / 2)
            mipmap: true

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

                text: LauncherCore.currentProfile.config.name

                QQC2.Menu {
                    id: profileMenu

                    modal: true

                    Repeater {
                        model: LauncherCore.profileManager

                        QQC2.MenuItem {
                            id: profileMenuItem

                            required property var profile

                            QQC2.MenuItem {
                                text: profileMenuItem.profile.config.name

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
            id: regularLoginCard

            visible: !LauncherCore.currentProfile.config.isBenchmark
            maximumWidth: Kirigami.Units.gridUnit * 25

            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                id: currentAccountDelegate

                enabled: LauncherCore.accountManager.numAccounts > 1
                text: LauncherCore.currentProfile.account.config.name

                leading: Components.Avatar {
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium

                    name: LauncherCore.currentProfile.account.config.name
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
                                text: menuItem.account.config.name
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
                label: i18n("Square Enix ID")
                text: LauncherCore.currentProfile.account.config.name
                enabled: false

                QQC2.ToolTip.text: i18n("The username can only be changed under account settings.")
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }

            FormCard.FormDelegateSeparator {
                above: usernameField
                below: passwordField
            }

            FormCard.FormPasswordFieldDelegate {
                id: passwordField
                label: i18n("Square Enix Password")
                focus: true
                onAccepted: {
                    if (otpField.visible) {
                        otpField.clicked();
                    } else {
                        loginButton.clicked();
                    }
                }
            }

            FormCard.FormDelegateSeparator {
                above: passwordField
                below: LauncherCore.currentProfile.account.config.useOTP ? otpField : loginButton
            }

            FormCard.FormTextFieldDelegate {
                id: otpField

                enabled: !LauncherCore.currentProfile.account.config.rememberOTP
                label: i18n("One-time Password")
                visible: LauncherCore.currentProfile.account.config.useOTP
                onAccepted: {
                    if (page.isLoginValid) {
                        loginButton.clicked()
                    }
                }
            }

            FormCard.FormDelegateSeparator {
                above: LauncherCore.currentProfile.account.config.useOTP ? otpField : passwordField
                below: loginButton
                visible: LauncherCore.currentProfile.account.config.useOTP
            }

            FormCard.FormButtonDelegate {
                id: loginButton

                text: i18n("Log In")
                description: page.invalidLoginReason
                icon.name: "unlock"
                enabled: page.isLoginValid
                onClicked: {
                    page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "StatusPage"))
                    LauncherCore.login(LauncherCore.currentProfile, usernameField.text, passwordField.text, otpField.text)
                }
            }

            FormCard.FormDelegateSeparator {
                above: loginButton
                below: forgotPasswordButton
            }

            FormCard.FormButtonDelegate {
                id: forgotPasswordButton

                text: i18n("Forgot ID or Password")
                icon.name: "question-symbolic"
                onClicked: applicationWindow().openUrl('https://secure.square-enix.com/account/app/svc/reminder')
            }
        }

        FormCard.FormCard {
            id: benchmarkLaunchCard

            visible: LauncherCore.currentProfile.config.isBenchmark
            maximumWidth: Kirigami.Units.gridUnit * 25

            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                id: launchBenchmarkDelegate

                text: i18n("Launch Benchmark")
                onClicked: {
                    page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "StatusPage"))
                    LauncherCore.login(LauncherCore.currentProfile, "", "", "")
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        FormCard.FormCard {
            maximumWidth: Kirigami.Units.gridUnit * 25

            Layout.alignment: Qt.AlignBottom

            FormCard.FormLinkDelegate {
                text: i18nc("@action:button", "The Lodestone")
                icon.name: "internet-services-symbolic"
                // TODO: how do we link to a "worldwide" lodestone, if that even exists?
                url: 'https://na.finalfantasyxiv.com/lodestone/'
                onClicked: applicationWindow().openUrl(url)
            }

            FormCard.FormDelegateSeparator {}

            FormCard.FormLinkDelegate {
                text: i18nc("@action:button", "Mog Station")
                icon.name: "internet-services-symbolic"
                url: 'https://secure.square-enix.com/account/app/svc/mogstation/'
                onClicked: applicationWindow().openUrl(url)
            }
        }
    }
}
