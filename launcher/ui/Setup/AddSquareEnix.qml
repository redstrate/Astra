// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    readonly property bool isValid: usernameField.text.length !== 0
    property var profile

    title: i18n("Add New Account")

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormTextDelegate {
            id: helpTextDelegate

            description: i18n("The password will be entered on the login page. A username will be associated with this account but can always be changed later.")
        }
        FormCard.FormDelegateSeparator {
            above: helpTextDelegate
            below: usernameField
        }
        FormCard.FormTextFieldDelegate {
            id: usernameField

            label: i18n("Username")
        }
        FormCard.FormDelegateSeparator {
            above: usernameField
            below: licenseField
        }
        FormCard.FormComboBoxDelegate {
            id: licenseField

            currentIndex: 0
            description: i18n("If the account holds multiple licenses, choose the preferred one.")
            model: ["Windows", "Steam", "macOS"]
            text: i18n("License")
        }
        FormCard.FormDelegateSeparator {
            above: licenseField
            below: freeTrialField
        }
        FormCard.FormCheckDelegate {
            id: freeTrialField

            description: i18n("Check if the account is currently on free trial.")
            text: i18n("Free Trial")
        }
        FormCard.FormDelegateSeparator {
            above: freeTrialField
            below: buttonDelegate
        }
        FormCard.FormButtonDelegate {
            id: buttonDelegate

            enabled: page.isValid
            icon.name: "list-add-symbolic"
            text: i18n("Add Account")

            onClicked: {
                let account = LauncherCore.accountManager.createSquareEnixAccount(usernameField.text, licenseField.currentIndex, freeTrialField.checkState === Qt.Checked);
                if (page.profile) {
                    page.profile.account = account;
                    applicationWindow().checkSetup();
                } else {
                    page.Window.window.pageStack.layers.pop();
                }
            }
        }
    }
}
