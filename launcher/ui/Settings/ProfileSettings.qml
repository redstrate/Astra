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

    actions: [
        Kirigami.Action {
            text: i18n("Delete Profile…")
            icon.name: "delete"
            enabled: LauncherCore.profileManager.canDelete(page.profile)
            tooltip: !enabled ? i18n("Cannot delete the only profile.") : ""

            onTriggered: deletePrompt.open()
        }
    ]

    header: Kirigami.NavigationTabBar {
        width: parent.width

        actions: [
            Kirigami.Action {
                id: generalAction
                text: i18n("General")
            },
            Kirigami.Action {
                id: wineAction
                text: i18n("Wine")
                visible: !LauncherCore.isWindows
            },
            Kirigami.Action {
                id: toolsAction
                text: i18n("Tools")
                visible: !LauncherCore.isWindows
            },
            Kirigami.Action {
                id: dalamudAction
                text: i18n("Dalamud")
            }
        ]

        Component.onCompleted: actions[0].checked = true
    }

    FormCard.FormCard {
        visible: generalAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: nameDelegate

            label: i18n("Name")
            text: page.profile.config.name
            onTextChanged: page.profile.config.name = text
        }

        FormCard.FormDelegateSeparator {
            above: nameDelegate
            below: gamePathDelegate
        }

        FormFolderDelegate {
            id: gamePathDelegate

            text: i18n("Game Folder")
            folder: page.profile.config.gamePath

            onAccepted: (folder) => page.profile.config.gamePath = folder
        }

        FormCard.FormDelegateSeparator {
            above: gamePathDelegate
            below: directXDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: directXDelegate

            text: i18n("DirectX Version")
            model: ["DirectX 11", "DirectX 9"]
            currentIndex: page.profile.config.directx9Enabled ? 1 : 0
            onCurrentIndexChanged: page.profile.config.directx9Enabled = (currentIndex === 1)
            visible: page.profile.hasDirectx9
        }

        FormCard.FormDelegateSeparator {
            above: directXDelegate
            visible: page.profile.hasDirectx9
        }

        FormCard.FormSwitchDelegate {
            id: allowUpdatesDelegate

            text: i18n("Allow Updating")
            description: i18n("If unchecked, Astra won't try to update the game automatically.")
            checked: page.profile.config.allowPatching
            onCheckedChanged: page.profile.config.allowPatching = checked
            visible: LauncherCore.config.showDevTools
        }

        FormCard.FormDelegateSeparator {
            above: allowUpdatesDelegate
            visible: LauncherCore.config.showDevTools
        }

        FormCard.FormTextDelegate {
            description: page.profile.expansionVersionText
        }
    }

    FormCard.FormCard {
        visible: wineAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormComboBoxDelegate {
            id: wineTypeDelegate

            text: i18n("Wine Type")
            model: [i18n("Built-in"), i18n("Custom")]
            currentIndex: page.profile.config.wineType
            onCurrentIndexChanged: page.profile.config.wineType = currentIndex
        }

        FormCard.FormDelegateSeparator {
            above: wineTypeDelegate
            below: winePathDelegate
        }

        FormFileDelegate {
            id: winePathDelegate

            text: i18n("Wine Executable")
            file: page.profile.winePath
            enabled: page.profile.config.wineType !== Profile.BuiltIn

            onAccepted: (path) => page.profile.winePath = path
        }

        FormCard.FormDelegateSeparator {
            above: winePathDelegate
            below: winePrefixPathDelegate
        }

        FormFolderDelegate {
            id: winePrefixPathDelegate

            text: i18n("Wine Prefix Folder")
            folder: page.profile.config.winePrefixPath
        }

        FormCard.FormDelegateSeparator {
            above: winePrefixPathDelegate
        }

        FormCard.FormTextDelegate {
            description: page.profile.wineVersionText
        }
    }

    FormCard.FormCard {
        visible: toolsAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            text: i18n("Enable Gamescope")
            description: i18n("A micro-compositor that uses Wayland to create a nested session.\nIf you use fullscreen mode, it may improve input handling.")
            checked: page.profile.config.useGamescope
            onCheckedChanged: page.profile.config.useGamescope = checked
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormButtonDelegate {
            text: i18n("Configure Gamescope...")
            icon.name: "configure"
            enabled: page.profile.config.useGamescope
            Kirigami.PromptDialog {
                id: gamescopeSettingsDialog
                title: i18n("Configure Gamescope")
                parent: page

                Kirigami.FormLayout {
                    QQC2.CheckBox {
                        Kirigami.FormData.label: "Fullscreen:"
                        checked: page.profile.config.gamescopeFullscreen
                        onCheckedChanged: page.profile.config.gamescopeFullscreen = checked
                    }
                    QQC2.CheckBox {
                        Kirigami.FormData.label: "Borderless:"
                        checked: page.profile.config.gamescopeBorderless
                        onCheckedChanged: page.profile.config.gamescopeBorderless = checked
                    }
                    QQC2.SpinBox {
                        Kirigami.FormData.label: "Width:"
                        to: 4096
                        value: page.profile.config.gamescopeWidth
                        onValueModified: page.profile.config.gamescopeWidth = value
                    }
                    QQC2.SpinBox {
                        Kirigami.FormData.label: "Height:"
                        to: 4096
                        value: page.profile.config.gamescopeHeight
                        onValueModified: page.profile.config.gamescopeHeight = value
                    }
                    QQC2.SpinBox {
                        Kirigami.FormData.label: "Refresh Rate:"
                        to: 512
                        value: page.profile.config.gamescopeRefreshRate
                        onValueModified: page.profile.config.gamescopeRefreshRate = value
                    }
                }
            }

            onClicked: gamescopeSettingsDialog.open()
        }
    }

    FormCard.FormCard {
        visible: dalamudAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormCheckDelegate {
            id: enableDalamudDelegate

            text: i18n("Enable Dalamud")
            description: i18n("Dalamud extends the game with useful plugins, but use at your own risk.")
            checked: page.profile.config.dalamudEnabled
            onCheckedChanged: page.profile.config.dalamudEnabled = checked
        }

        FormCard.FormDelegateSeparator {
            above: enableDalamudDelegate
            below: dalamudChannelDelegate
        }

        FormCard.FormComboBoxDelegate {
            id: dalamudChannelDelegate

            text: i18n("Update Channel")
            model: LauncherCore.config.showDevTools ? [i18n("Stable"), i18n("Staging"), i18n("Local")] : [i18n("Stable"), i18n("Staging")]
            currentIndex: page.profile.config.dalamudChannel
            onCurrentIndexChanged: page.profile.config.dalamudChannel = currentIndex
            enabled: page.profile.config.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudChannelDelegate
            below: dalamudInjectDelegate
            visible: LauncherCore.config.showDevTools
        }

        FormCard.FormComboBoxDelegate {
            id: dalamudInjectDelegate

            visible: LauncherCore.config.showDevTools
            text: i18n("Injection Method")
            description: "It shouldn't be necessary to change this setting, unless you're running into issues injecting Dalamud."
            model: ["Entrypoint", "DLL Injection"]
            currentIndex: page.profile.config.dalamudInjectMethod
            onCurrentIndexChanged: page.profile.config.dalamudInjectMethod = currentIndex
            enabled: page.profile.config.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudInjectDelegate
            below: dalamudDelayDelegate
        }

        FormCard.FormSpinBoxDelegate {
            id: dalamudDelayDelegate

            visible: LauncherCore.config.showDevTools
            label: i18n("Injection Delay")
            value: page.profile.config.dalamudInjectDelay
            onValueChanged: page.profile.config.dalamudInjectDelay = value
            enabled: page.profile.config.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudDelayDelegate
        }

        FormCard.FormTextDelegate {
            description: page.profile.dalamudVersionText
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Profile")
        subtitle: i18nc("@label", "Are you sure you want to delete this profile?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false
        parent: page

        QQC2.Switch {
            id: deleteFilesSwitch

            checked: true

            text: i18n("Delete Files")
        }

        onAccepted: {
            LauncherCore.profileManager.deleteProfile(page.profile, deleteFilesSwitch.checked);
            page.Window.window.pageStack.layers.pop();
        }
    }
}
