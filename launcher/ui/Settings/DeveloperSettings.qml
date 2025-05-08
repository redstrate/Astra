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
            checked: LauncherCore.config.keepPatches
            onCheckedChanged: {
                LauncherCore.config.keepPatches = checked;
                LauncherCore.config.save();
            }
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
            checked: LauncherCore.config.encryptArguments
            onCheckedChanged: {
                LauncherCore.config.encryptArguments = checked;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: encryptArgDelegate
            below: renderDocCaptureDelegate
        }

        FormCard.FormCheckDelegate {
            id: renderDocCaptureDelegate

            text: i18n("Allow RenderDoc Capture")
            description: i18n("Inject the RenderDoc capture layer.")
            checked: LauncherCore.config.enableRenderDocCapture
            onCheckedChanged: {
                LauncherCore.config.enableRenderDocCapture = checked;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: renderDocCaptureDelegate
            below: enableWaylandDelegate
        }

        FormCard.FormCheckDelegate {
            id: enableWaylandDelegate

            text: i18n("Run Game on Wayland")
            description: i18n("Use the native Wine Wayland driver instead of going through XWayland.")
            checked: LauncherCore.config.enableWayland
            onCheckedChanged: {
                LauncherCore.config.enableWayland = checked;
                LauncherCore.config.save();
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Servers")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextFieldDelegate {
            id: preferredProtocolDelegate

            label: i18n("Preferred Protocol")
            text: LauncherCore.config.preferredProtocol
            onTextChanged: {
                LauncherCore.config.preferredProtocol = text;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: preferredProtocolDelegate
            below: dalamudServerDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: dalamudServerDelegate

            label: i18n("Dalamud Distribution Server")
            text: LauncherCore.config.dalamudDistribServer
            onTextChanged: {
                LauncherCore.config.dalamudDistribServer = text;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: dalamudServerDelegate
            below: githubApiDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: githubApiDelegate

            label: i18n("GitHub API")
            text: LauncherCore.config.githubApi
            onTextChanged: {
                LauncherCore.config.githubApi = text;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: githubApiDelegate
            below: dxvkRepositoryDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: dxvkRepositoryDelegate

            label: i18n("DXVK Repository")
            text: LauncherCore.config.dXVKRepository
            onTextChanged: {
                LauncherCore.config.dXVKRepository = text;
                LauncherCore.config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: dxvkRepositoryDelegate
            below: wineRepositoryDelegate
        }

        FormCard.FormTextFieldDelegate {
            id: wineRepositoryDelegate

            label: i18n("Wine Repository")
            text: LauncherCore.config.wineRepository
            onTextChanged: {
                LauncherCore.config.wineRepository = text;
                LauncherCore.config.save();
            }
        }
    }
}
