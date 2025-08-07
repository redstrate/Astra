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
                icon.name: "configure-symbolic"
            },
            Kirigami.Action {
                id: wineAction
                text: i18n("Wine")
                visible: !LauncherCore.isWindows
                icon.name: "wine-symbolic"
            },
            Kirigami.Action {
                id: dalamudAction
                text: i18n("Dalamud")
                icon.name: "extension-symbolic"
            },
            Kirigami.Action {
                id: developerAction
                text: i18n("Developer")
                visible: LauncherCore.config.showDevTools
                icon.name: "applications-development-symbolic"
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
            onTextChanged: {
                page.profile.config.name = text;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: nameDelegate
            below: gamePathDelegate
        }

        FormFolderDelegate {
            id: gamePathDelegate

            text: i18n("Game Folder")
            folder: page.profile.config.gamePath
            displayText: page.profile.isGamePathDefault ? i18n("Default Location") : folder

            onAccepted: (folder) => {
                page.profile.config.gamePath = folder;
                page.profile.config.save();
            }
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
            onCurrentIndexChanged: {
                page.profile.config.directx9Enabled = (currentIndex === 1);
                page.profile.config.save();
            }
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
            onCheckedChanged: {
                page.profile.config.allowPatching = checked;
                page.profile.config.save();
            }
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
            onCurrentIndexChanged: {
                page.profile.config.wineType = currentIndex;
                page.profile.config.save();
            }
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

            onAccepted: (path) => {
                page.profile.winePath = path;
                page.profile.config.save();
            }
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
            displayText: page.profile.isWinePrefixDefault ? i18n("Default Location") : folder

            onAccepted: (path) => {
                page.profile.config.winePrefixPath = path;
                page.profile.config.save();
            }
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
            onCheckedChanged: {
                page.profile.config.dalamudEnabled = checked;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: enableDalamudDelegate
            below: dalamudChannelDelegate
            visible: page.profile.config.dalamudEnabled
        }

        FormCard.FormComboBoxDelegate {
            id: dalamudChannelDelegate

            ListModel {
                id: normalTracks
                ListElement { name: "Release"; track: "release" }
                ListElement { name: "Custom"; track: "custom" }
            }

            ListModel {
                id: devTracks
                ListElement { name: "Release"; track: "release" }
                ListElement { name: "Custom"; track: "custom" }
                ListElement { name: "Local Build"; track: "local" }
            }

            text: i18n("Update Track")
            model: LauncherCore.config.showDevTools ? devTracks : normalTracks
            textRole: "name"
            valueRole: "track"
            onCurrentValueChanged: {
                // custom one is set below
                if (currentValue !== "custom") {
                    page.profile.config.dalamudChannel = currentValue;
                    page.profile.config.save();
                }
            }
            Component.onCompleted: {
                const index = indexOfValue(page.profile.config.dalamudChannel);
                // if you can't find it, it's probably custom
                if (index === -1) {
                    currentIndex = 1;
                } else {
                    currentIndex = index;
                }
            }
            enabled: page.profile.config.dalamudEnabled
            visible: page.profile.config.dalamudEnabled
        }

        FormCard.FormTextFieldDelegate {
            id: customTrackDelegate

            visible: page.profile.config.dalamudEnabled && dalamudChannelDelegate.currentIndex === 1 // custom
            label: i18nc("@info:label", "Track Name")
            enabled: page.profile.config.dalamudEnabled

            Component.onCompleted: text = page.profile.config.dalamudChannel
            onTextChanged: {
                page.profile.config.dalamudChannel = text;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: dalamudChannelDelegate
            below: dalamudInjectDelegate
            visible: LauncherCore.config.showDevTools && page.profile.config.dalamudEnabled
        }

        FormCard.FormComboBoxDelegate {
            id: dalamudInjectDelegate

            text: i18n("Injection Method")
            description: "It shouldn't be necessary to change this setting, unless you're running into issues injecting Dalamud."
            model: ["Entrypoint", "DLL Injection"]
            currentIndex: page.profile.config.dalamudInjectMethod
            onCurrentIndexChanged: {
                page.profile.config.dalamudInjectMethod = currentIndex;
                page.profile.config.save();
            }
            enabled: page.profile.config.dalamudEnabled
            visible: LauncherCore.config.showDevTools && page.profile.config.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudInjectDelegate
            below: dalamudDelayDelegate
            visible: LauncherCore.config.showDevTools && page.profile.config.dalamudEnabled
        }

        FormCard.FormSpinBoxDelegate {
            id: dalamudDelayDelegate

            label: i18n("Injection Delay")
            value: page.profile.config.dalamudInjectDelay
            onValueChanged: {
                page.profile.config.dalamudInjectDelay = value;
                page.profile.config.save();
            }
            enabled: page.profile.config.dalamudEnabled
            visible: LauncherCore.config.showDevTools && page.profile.config.dalamudEnabled
        }

        FormCard.FormDelegateSeparator {
            above: dalamudDelayDelegate
            below: dalamudVersionTextDelegate
            visible: page.profile.config.dalamudEnabled
        }

        FormCard.FormTextDelegate {
            id: dalamudVersionTextDelegate

            description: page.profile.dalamudVersionText
            visible: page.profile.config.dalamudEnabled
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
            onTextChanged: {
                page.profile.config.platform = text;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: platformDelegate
            below: bootUpdateChannel
        }

        FormCard.FormTextFieldDelegate {
            id: bootUpdateChannel

            label: i18n("Boot Update Channel")
            text: page.profile.config.bootUpdateChannel
            onTextChanged: {
                page.profile.config.bootUpdateChannel = text;
                page.profile.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: bootUpdateChannel
            below: gameUpdateChannel
        }

        FormCard.FormTextFieldDelegate {
            id: gameUpdateChannel

            label: i18n("Game Update Channel")
            text: page.profile.config.gameUpdateChannel
            onTextChanged: {
                page.profile.config.gameUpdateChannel = text;
                page.profile.config.save();
            }
        }
    }

    Kirigami.PromptDialog {
        id: deletePrompt

        title: i18nc("@title", "Delete Profile")
        subtitle: i18nc("@label", "Are you sure you want to delete this profile?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        showCloseButton: false
        parent: page.QQC2.Overlay.overlay

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
