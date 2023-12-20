// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

import "../Components"

FormCard.FormCardPage {
    id: page

    property var profile

    title: i18nc("@window:title", "Edit Profile")

    FormCard.FormHeader {
        title: i18n("General")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextFieldDelegate {
            id: nameDelegate

            label: i18n("Name")
            text: page.profile.name
            onTextChanged: page.profile.name = text
        }

        FormCard.FormDelegateSeparator {
            above: nameDelegate
            below: gamePathDelegate
        }

        FormFolderDelegate {
            id: gamePathDelegate

            text: i18n("Game Path")
            folder: page.profile.gamePath

            onAccepted: (folder) => page.profile.gamePath = folder
        }

        FormCard.FormDelegateSeparator {
            above: gamePathDelegate
            below: directXDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: directXDelegate

            text: i18n("DirectX Version")
            model: ["DirectX 11", "DirectX 9"]
            currentIndex: page.profile.directx9Enabled ? 1 : 0
            onCurrentIndexChanged: page.profile.directx9Enabled = (currentIndex === 1)
        }

        FormCard.FormDelegateSeparator {
            above: directXDelegate
        }

        FormCard.FormTextDelegate {
            description: page.profile.expansionVersionText
        }
    }

    FormCard.FormHeader {
        title: i18n("Wine")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormComboBoxDelegate {
            id: wineTypeDelegate

            text: i18n("Wine Type")
            model: [i18n("Built-in"), i18n("Custom")]
            currentIndex: page.profile.wineType
            onCurrentIndexChanged: page.profile.wineType = currentIndex
        }

        FormCard.FormDelegateSeparator {
            above: wineTypeDelegate
            below: winePathDelegate
        }

        FormFileDelegate {
            id: winePathDelegate

            text: i18n("Wine Executable")
            file: page.profile.winePath
            enabled: page.profile.wineType !== Profile.BuiltIn
        }

        FormCard.FormDelegateSeparator {
            above: winePathDelegate
            below: winePrefixPathDelegate
        }

        FormFolderDelegate {
            id: winePrefixPathDelegate

            text: i18n("Wine Prefix Path")
            folder: page.profile.winePrefixPath
        }

        FormCard.FormDelegateSeparator {
            above: winePrefixPathDelegate
        }

        FormCard.FormTextDelegate {
            description: page.profile.wineVersionText
        }
    }

    FormCard.FormHeader {
        title: i18n("Tools")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            text: i18n("Enable Gamescope")
            description: i18n("A micro-compositor that uses Wayland to create a nested session.\nIf you use fullscreen mode, it may improve input handling.")
            checked: page.profile.gamescopeEnabled
            onCheckedChanged: page.profile.gamescopeEnabled = checked
            visible: false
            enabled: false
        }

        FormCard.FormDelegateSeparator {
            visible: false
        }

        FormCard.FormButtonDelegate {
            text: i18n("Configure Gamescope...")
            icon.name: "configure"
            enabled: false
            visible: false
            Kirigami.PromptDialog {
                id: gamescopeSettingsDialog
                title: i18n("Configure Gamescope")

                Kirigami.FormLayout {
                    QQC2.CheckBox {
                        Kirigami.FormData.label: "Fullscreen:"
                        checked: page.profile.gamescopeFullscreen
                        onCheckedChanged: page.profile.gamescopeFullscreen = checked
                    }
                    QQC2.CheckBox {
                        Kirigami.FormData.label: "Borderless:"
                        checked: page.profile.gamescopeBorderless
                        onCheckedChanged: page.profile.gamescopeBorderless = checked
                    }
                    QQC2.SpinBox {
                        Kirigami.FormData.label: "Width:"
                        to: 4096
                        value: page.profile.gamescopeWidth
                        onValueModified: page.profile.gamescopeWidth = value
                    }
                    QQC2.SpinBox {
                        Kirigami.FormData.label: "Height:"
                        to: 4096
                        value: page.profile.gamescopeHeight
                        onValueModified: page.profile.gamescopeHeight = value
                    }
                    QQC2.SpinBox {
                        Kirigami.FormData.label: "Refresh Rate:"
                        to: 512
                        value: page.profile.gamescopeRefreshRate
                        onValueModified: page.profile.gamescopeRefreshRate = value
                    }
                }
            }

            onClicked: gamescopeSettingsDialog.open()
        }

        FormCard.FormDelegateSeparator {
            visible: false
        }

        FormCard.FormCheckDelegate {
            id: gamemodeDelegate

            text: i18n("Enable Gamemode")
            description: i18n("A special game performance tool, that tunes your CPU scheduler among other things.")
            checked: page.profile.gamemodeEnabled
            onCheckedChanged: page.profile.gamemodeEnabled = checked
        }
    }

    FormCard.FormHeader {
        title: i18n("Dalamud")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: enableDalamudDelegate

            text: i18n("Enable Dalamud")
            description: i18n("Dalamud extends the game with useful plugins, but use at your own risk.")
            checked: page.profile.dalamudEnabled
            onCheckedChanged: page.profile.dalamudEnabled = checked
        }

        FormCard.FormDelegateSeparator {
            above: enableDalamudDelegate
            below: dalamudChannelDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: dalamudChannelDelegate

            text: i18n("Update Channel")
            model: ["Stable", "Staging", ".NET 5"]
            currentIndex: page.profile.dalamudChannel
            onCurrentIndexChanged: page.profile.dalamudChannel = currentIndex
            enabled: page.profile.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudChannelDelegate
            below: dalamudInjectDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: dalamudInjectDelegate

            text: i18n("Injection Method")
            description: "It shouldn't be nessecary to change this setting, unless you're running into issues injecting Dalamud."
            model: ["Entrypoint", "DLL Injection"]
            currentIndex: page.profile.dalamudInjectMethod
            onCurrentIndexChanged: page.profile.dalamudInjectMethod = currentIndex
            enabled: page.profile.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudInjectDelegate
            below: dalamudDelayDelegate
        }

        FormCard.FormSpinBoxDelegate {
            id: dalamudDelayDelegate

            label: i18n("Injection Delay")
            value: page.profile.dalamudInjectDelay
            onValueChanged: page.profile.dalamudInjectDelay = value
            enabled: page.profile.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudDelayDelegate
        }

        FormCard.FormTextDelegate {
            description: page.profile.dalamudVersionText
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            text: i18n("Delete Profile")
            description: !enabled ? i18n("Cannot delete the only profile.") : ""
            icon.name: "delete"
            enabled: LauncherCore.profileManager.canDelete(page.profile)

            Kirigami.PromptDialog {
                id: deletePrompt

                title: i18nc("@title", "Delete Profile")
                subtitle: i18nc("@label", "Are you sure you want to delete this profile?")
                standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                showCloseButton: false

                onAccepted: {
                    LauncherCore.profileManager.deleteProfile(page.profile);
                    page.Window.window.pageStack.layers.pop();
                }
            }

            onClicked: deletePrompt.open()
        }
    }
}