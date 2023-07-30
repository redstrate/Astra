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

    title: i18n("Settings")

    ColumnLayout {
        width: parent.width

        GeneralSettings {
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Profiles")
                }

                Repeater {
                    model: LauncherCore.profileManager

                    MobileForm.FormButtonDelegate {
                        required property var profile

                        text: profile.name
                        onClicked: applicationWindow().pageStack.layers.push('qrc:/ui/Settings/ProfileSettings.qml', {
                            profile: profile
                        })
                    }
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Profile")
                    icon.name: "list-add"
                    onClicked: {
                        applicationWindow().currentSetupProfile = LauncherCore.profileManager.addProfile()
                        applicationWindow().checkSetup()
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Accounts")
                }

                Repeater {
                    model: LauncherCore.accountManager

                    MobileForm.FormButtonDelegate {
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

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Square Enix Account")
                    icon.name: "list-add-symbolic"
                    onClicked: pageStack.layers.push('qrc:/ui/Setup/AddSquareEnix.qml')
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Add Sapphire Account")
                    icon.name: "list-add-symbolic"
                    onClicked: pageStack.layers.push('qrc:/ui/Setup/AddSapphire.qml')
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                Component {
                    id: aboutPage
                    MobileForm.AboutPage {
                        aboutData: About
                    }
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("About Astra")
                    icon.name: "help-about-symbolic"
                    onClicked: applicationWindow().pageStack.layers.push(aboutPage)
                }
            }
        }
    }
}