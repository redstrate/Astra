// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

import "../Components"

FormCard.FormCardPage {
    id: page

    title: i18nc("@title:window", "Developer Settings")

    FormCard.FormHeader {
        title: i18n("Patching")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormCheckDelegate {
            id: keepPatchesDelegate

            text: i18n("Keep Patches")
            description: i18n("Do not delete patches after they're used. Astra will not download patch data, if found.")
            checked: LauncherCore.settings.keepPatches
            onCheckedChanged: LauncherCore.settings.keepPatches = checked
        }
    }

    FormCard.FormHeader {
        title: i18n("Launching")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: launchGameDelegate

            text: i18n("Launch Game Now")
            description: i18n("This is meant for testing if we can get to the title screen and will fail at doing anything else.")

            onClicked: LauncherCore.immediatelyLaunch(LauncherCore.currentProfile)
        }

        FormCard.FormDelegateSeparator {
            above: launchGameDelegate
            below: encryptArgDelegate
        }

        FormCard.FormCheckDelegate {
            id: encryptArgDelegate

            text: i18n("Encrypt Game Arguments")
            description: i18n("Disable encryption if you want to inspect the raw arguments passed to the game.")
            checked: LauncherCore.settings.argumentsEncrypted
            onCheckedChanged: LauncherCore.settings.argumentsEncrypted = checked
        }

        FormCard.FormDelegateSeparator {
            above: encryptArgDelegate
            below: renderDocCaptureDelegate
        }

        FormCard.FormCheckDelegate {
            id: renderDocCaptureDelegate

            text: i18n("Allow RenderDoc Capture")
            description: i18n("Inject the RenderDoc capture layer.")
            checked: LauncherCore.settings.enableRenderDocCapture
            onCheckedChanged: LauncherCore.settings.enableRenderDocCapture = checked
        }
    }

    FormCard.FormHeader {
        title: i18n("Servers")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: resetToDefaultsDelegate

            text: i18n("Reset to Defaults")

            onClicked: {
                preferredProtocolDelegate.text = LauncherCore.settings.defaultPreferredProtocol();
                dalamudServerDelegate.text = LauncherCore.settings.defaultDalamudDistribServer();
                squareMainServerDelegate.text = LauncherCore.settings.defaultSquareEnixServer();
                loginServerDelegate.text = LauncherCore.settings.defaultSquareEnixLoginServer();
                mainServerDelegate.text = LauncherCore.settings.defaultMainServer();
            }
        }

        FormCard.FormDelegateSeparator {
            above: resetToDefaultsDelegate
            below: localServerDelegate
        }

        FormCard.FormButtonDelegate {
            id: localServerDelegate

            text: i18n("Set to localhost")

            onClicked: {
                preferredProtocolDelegate.text = "http";
                squareMainServerDelegate.text = "ffxiv.local";
                loginServerDelegate.text = "square.local";
            }
        }

        FormCard.FormDelegateSeparator {
            above: localServerDelegate
            below: preferredProtocolDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: preferredProtocolDelegate

            label: i18n("Preferred Protocol")
            text: LauncherCore.settings.preferredProtocol
            onTextChanged: LauncherCore.settings.preferredProtocol = text
        }

        FormCard.FormDelegateSeparator {
            above: preferredProtocolDelegate
            below: dalamudServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: dalamudServerDelegate

            label: i18n("Dalamud Distribution Server")
            text: LauncherCore.settings.dalamudDistribServer
            onTextChanged: LauncherCore.settings.dalamudDistribServer = text
        }

        FormCard.FormDelegateSeparator {
            above: dalamudServerDelegate
            below: squareMainServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: squareMainServerDelegate

            label: i18n("SE Main Server (ffxiv.com)")
            text: LauncherCore.settings.squareEnixServer
            onTextChanged: LauncherCore.settings.squareEnixServer = text
        }

        FormCard.FormDelegateSeparator {
            above: squareMainServerDelegate
            below: loginServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: loginServerDelegate

            label: i18n("SE Login Server (square-enix.com)")
            text: LauncherCore.settings.squareEnixLoginServer
            onTextChanged: LauncherCore.settings.squareEnixLoginServer = text
        }

        FormCard.FormDelegateSeparator {
            above: loginServerDelegate
            below: mainServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: mainServerDelegate

            label: i18n("Main Server (finalfantasyxiv.com)")
            text: LauncherCore.settings.mainServer
            onTextChanged: LauncherCore.settings.mainServer = text
        }

        FormCard.FormDelegateSeparator {
            above: mainServerDelegate
            below: gameServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: gameServerDelegate

            label: i18n("Game Server (leave blank for default)")
            text: LauncherCore.settings.customGameServer
            onTextChanged: LauncherCore.settings.customGameServer = text
        }

        FormCard.FormDelegateSeparator {
            above: gameServerDelegate
            below: gameServerPortDelegate
        }

        FormCard.FormSpinBoxDelegate {
            id: gameServerPortDelegate

            label: i18n("Game Server Port")
            value: LauncherCore.settings.customGameServerPort
            onValueChanged: LauncherCore.settings.customGameServerPort = value
            from: 1
            to: 999999
        }
    }
}
