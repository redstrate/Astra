// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

import "../Components"

Kirigami.ScrollablePage {
    id: page

    property var profile

    title: i18n("Profile Settings")

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("General")
                }

                MobileForm.FormTextFieldDelegate {
                    label: i18n("Name")
                    text: page.profile.name
                    onTextChanged: page.profile.name = text
                }

                MobileForm.FormDelegateSeparator {
                }

                FormFolderDelegate {
                    text: i18n("Game Path")
                    folder: page.profile.gamePath
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormComboBoxDelegate {
                    text: i18n("DirectX Version")
                    model: ["DirectX 11", "DirectX 9"]
                    currentIndex: page.profile.directx9Enabled ? 1 : 0
                    onCurrentIndexChanged: page.profile.directx9Enabled = (currentIndex === 1)
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Encrypt Game Arguments")
                    checked: page.profile.argumentsEncrypted
                    onCheckedChanged: page.profile.argumentsEncrypted = checked
                    enabled: false
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Enable Watchdog")
                    description: i18n("Gives real-time queue updates. X11 only.")
                    checked: page.profile.watchdogEnabled
                    onCheckedChanged: page.profile.watchdogEnabled = checked
                    enabled: false
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextDelegate {
                    description: page.profile.expansionVersionText
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Wine")
                }

                MobileForm.FormComboBoxDelegate {
                    text: i18n("Wine Type")
                    model: ["System", "Custom"]
                    currentIndex: page.profile.wineType
                    onCurrentIndexChanged: page.profile.wineType = currentIndex
                    enabled: !LauncherCore.isSteam
                }

                MobileForm.FormDelegateSeparator {
                }

                FormFileDelegate {
                    text: i18n("Wine Path")
                    file: page.profile.winePath
                    enabled: !LauncherCore.isSteam
                }

                MobileForm.FormDelegateSeparator {
                }

                FormFolderDelegate {
                    text: i18n("Wine Prefix Path")
                    folder: page.profile.winePrefixPath
                    enabled: !LauncherCore.isSteam
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextDelegate {
                    description: page.profile.wineVersionText
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Tools")
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Enable ESync")
                    description: i18n("Could improve game performance, but requires a patched Wine and kernel.")
                    checked: page.profile.esyncEnabled
                    onCheckedChanged: page.profile.esyncEnabled = checked
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Enable Gamescope")
                    description: i18n("A micro-compositor that uses Wayland to create a nested session.\nIf you use fullscreen mode, it may improve input handling.")
                    checked: page.profile.gamescopeEnabled
                    onCheckedChanged: page.profile.gamescopeEnabled = checked
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Configure Gamescope...")
                    icon.name: "configure"
                    enabled: page.profile.gamescopeEnabled
                    Kirigami.PromptDialog {
                        id: gamescopeSettingsDialog
                        title: i18n("Configure Gamescope")

                        Kirigami.FormLayout {
                            Controls.CheckBox {
                                Kirigami.FormData.label: "Fullscreen:"
                                checked: page.profile.gamescopeFullscreen
                                onCheckedChanged: page.profile.gamescopeFullscreen = checked
                            }
                            Controls.CheckBox {
                                Kirigami.FormData.label: "Borderless:"
                                checked: page.profile.gamescopeBorderless
                                onCheckedChanged: page.profile.gamescopeBorderless = checked
                            }
                            Controls.SpinBox {
                                Kirigami.FormData.label: "Width:"
                                to: 4096
                                value: page.profile.gamescopeWidth
                                onValueModified: page.profile.gamescopeWidth = value
                            }
                            Controls.SpinBox {
                                Kirigami.FormData.label: "Height:"
                                to: 4096
                                value: page.profile.gamescopeHeight
                                onValueModified: page.profile.gamescopeHeight = value
                            }
                            Controls.SpinBox {
                                Kirigami.FormData.label: "Refresh Rate:"
                                to: 512
                                value: page.profile.gamescopeRefreshRate
                                onValueModified: page.profile.gamescopeRefreshRate = value
                            }
                        }
                    }

                    onClicked: gamescopeSettingsDialog.open()
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Enable Gamemode")
                    description: i18n("A special game performance tool, that tunes your CPU scheduler among other things.")
                    checked: page.profile.gamemodeEnabled
                    onCheckedChanged: page.profile.gamemodeEnabled = checked
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Dalamud")
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Enable Dalamud Plugins")
                    checked: page.profile.dalamudEnabled
                    onCheckedChanged: page.profile.dalamudEnabled = checked
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormComboBoxDelegate {
                    text: i18n("Update Channel")
                    model: ["Stable", "Staging", ".NET 5"]
                    currentIndex: page.profile.dalamudChannel
                    onCurrentIndexChanged: page.profile.dalamudChannel = currentIndex
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("Opt Out of Automatic Marketboard Collection")
                    checked: page.profile.dalamudOptOut
                    onCheckedChanged: page.profile.dalamudOptOut = checked
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormTextDelegate {
                    description: page.profile.dalamudVersionText
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormButtonDelegate {
                    text: i18n("Delete Profile")
                    description: !enabled ? i18n("Cannot delete the only profile") : ""
                    icon.name: "delete"
                    enabled: LauncherCore.profileManager.canDelete(page.profile)
                    onClicked: {
                        LauncherCore.profileManager.deleteProfile(page.profile)
                        applicationWindow().pageStack.layers.pop()
                    }
                }
            }
        }
    }
}