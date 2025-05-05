// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    id: page

    property var profile
    readonly property bool isInitialSetup: !LauncherCore.profileManager.hasAnyExistingInstallations()

    title: isInitialSetup ? i18n("Initial Setup") : i18n("Profile Setup")
    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    header: Kirigami.Separator {
        width: root.width
    }

    data: FolderDialog {
        id: installFolderDialog

        onAccepted: page.profile.config.gamePath = decodeURIComponent(selectedFolder.toString().replace("file://", ""))
    }

    Image {
        source: "qrc:/zone.xiv.astra.svg"

        fillMode: Image.PreserveAspectFit
        visible: !LauncherCore.profileManager.hasAnyExistingInstallations()

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: Kirigami.Units.largeSpacing * 3
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: LauncherCore.profileManager.hasAnyExistingInstallations() ? Kirigami.Units.largeSpacing : 0

        FormCard.FormTextDelegate {
            id: helpText

            text: {
                if (page.isInitialSetup) {
                    return i18n("You have to setup the game to continue.");
                } else {
                    return i18n("You need to select a game installation for '%1'.", page.profile.config.name);
                }
            }
        }
    }


    FormCard.FormCard {
        visible: LauncherCore.profileManager.hasAnyExistingInstallations()

        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: existingHelpDelegate

            text: i18n("You can select an existing game installation from another profile.")
        }

        FormCard.FormDelegateSeparator {
            above: existingHelpDelegate
        }

        Repeater {
            model: LauncherCore.profileManager

            FormCard.FormButtonDelegate {
                required property var profile

                text: profile.config.name
                description: profile.config.gamePath
                visible: profile.isGameInstalled

                onClicked: {
                    LauncherCore.currentProfile.config.gamePath = profile.config.gamePath;
                    applicationWindow().checkSetup();
                }
            }
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: installGameDelegate

            text: i18n("Install Game")
            description: i18n("This installation will be managed by Astra.")
            icon.name: "cloud-download"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "InstallGame"), {
                profile: page.profile
            })
        }

        FormCard.FormDelegateSeparator {
            above: installGameDelegate
            below: installBenchmarkDelegate
        }

        FormCard.FormButtonDelegate {
            id: installBenchmarkDelegate

            text: i18n("Install Benchmark")
            description: i18n("This installation will be managed by Astra.")
            icon.name: "cloud-download"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "InstallBenchmark"), {
                profile: page.profile
            })
        }

        FormCard.FormDelegateSeparator {
            above: installBenchmarkDelegate
            below: useExistingDelegate
        }

        FormCard.FormButtonDelegate {
            id: useExistingDelegate

            text: i18n("Select an Existing Installation")
            description: i18n("Use an existing installation from another launcher.")
            icon.name: "document-import-symbolic"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "UseExistingInstall"), {
                profile: page.profile
            })
        }
    }

    FormCard.FormCard {
        visible: root.isInitialSetup

        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: settingsButton

            text: i18nc("@action:button Application settings", "Settings")
            icon.name: "settings-configure"

            onClicked: Qt.createComponent("zone.xiv.astra", "SettingsPage").createObject(page, { window: applicationWindow() }).open()
        }
    }
}
