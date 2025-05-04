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
                id: dalamudAction
                text: i18n("Dalamud")
            },
            Kirigami.Action {
                id: developerAction
                text: i18n("Developer")
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
            visible: page.profile.config.wineType !== Profile.BuiltIn

            onAccepted: (path) => page.profile.winePath = path
        }

        FormCard.FormDelegateSeparator {
            above: winePathDelegate
            below: winePrefixPathDelegate
            visible: winePathDelegate.visible
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

    FormCard.FormCard {
        visible: developerAction.checked

        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormTextFieldDelegate {
            id: platformDelegate

            label: i18n("Platform")
            text: page.profile.config.platform
            onTextChanged: page.profile.config.platform = text
        }

        FormCard.FormDelegateSeparator {
            above: platformDelegate
            below: bootUpdateChannel
        }

        FormCard.FormTextFieldDelegate {
            id: bootUpdateChannel

            label: i18n("Boot Update Channel")
            text: page.profile.config.bootUpdateChannel
            onTextChanged: page.profile.config.bootUpdateChannel = text
        }

        FormCard.FormDelegateSeparator {
            above: bootUpdateChannel
            below: gameUpdateChannel
        }

        FormCard.FormTextFieldDelegate {
            id: gameUpdateChannel

            label: i18n("Game Update Channel")
            text: page.profile.config.gameUpdateChannel
            onTextChanged: page.profile.config.gameUpdateChannel = text
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
