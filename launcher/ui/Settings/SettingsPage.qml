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

    title: i18n("Settings")

    FormCard.FormHeader {
        title: i18n("General")
    }

    GeneralSettings {
        Layout.fillWidth: true
    }

    FormCard.FormHeader {
        title: i18n("Profiles")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            model: LauncherCore.profileManager

            FormCard.FormButtonDelegate {
                required property var profile

                text: profile.name
                onClicked: applicationWindow().pageStack.layers.push('qrc:/ui/Settings/ProfileSettings.qml', {
                    profile: profile
                })
            }
        }

        FormCard.FormDelegateSeparator {
            below: addProfileButton
        }

        FormCard.FormButtonDelegate {
            id: addProfileButton

            text: i18n("Add Profile")
            icon.name: "list-add"
            onClicked: {
                applicationWindow().currentSetupProfile = LauncherCore.profileManager.addProfile()
                applicationWindow().checkSetup()
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Accounts")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            model: LauncherCore.accountManager

            FormCard.FormButtonDelegate {
                required property var account

                text: account.name

                leading: Kirigami.Avatar
                    {
                    source: account.avatarUrl
                }

                leadingPadding: Kirigami.Units.largeSpacing * 2

                onClicked: applicationWindow().pageStack.layers.push('qrc:/ui/Settings/AccountSettings.qml', {
                    account: account
                })
            }
        }

        FormCard.FormDelegateSeparator {
            below: addSquareEnixButton
        }

        FormCard.FormButtonDelegate {
            id: addSquareEnixButton

            text: i18n("Add Square Enix Account")
            icon.name: "list-add-symbolic"
            onClicked: pageStack.layers.push('qrc:/ui/Setup/AddSquareEnix.qml')
        }

        FormCard.FormDelegateSeparator {
            above: addSquareEnixButton
            below: addSapphireButton
        }

        FormCard.FormButtonDelegate {
            id: addSapphireButton

            text: i18n("Add Sapphire Account")
            icon.name: "list-add-symbolic"
            onClicked: pageStack.layers.push('qrc:/ui/Setup/AddSapphire.qml')
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: setupCompatToolButton

            text: i18n("Setup Compatibility Tool")
            icon.name: "install"
            onClicked: applicationWindow().pageStack.layers.push('qrc:/ui/Settings/CompatibilityToolSetup.qml')
        }

        FormCard.FormDelegateSeparator {
            above: setupCompatToolButton
            below: developerSettingsButton
        }

        FormCard.FormButtonDelegate {
            id: developerSettingsButton

            text: i18n("Developer Settings")
            icon.name: "configure"
            onClicked: applicationWindow().pageStack.layers.push('qrc:/ui/Settings/DeveloperSettings.qml')
        }

        FormCard.FormDelegateSeparator {
            above: developerSettingsButton
            below: aboutButton
        }

        FormCard.FormButtonDelegate {
            id: aboutButton

            text: i18n("About Astra")
            icon.name: "help-about-symbolic"

            Component {
                id: aboutPage
                FormCard.AboutPage {
                    aboutData: About
                }
            }

            onClicked: applicationWindow().pageStack.layers.push(aboutPage)
        }
    }
}