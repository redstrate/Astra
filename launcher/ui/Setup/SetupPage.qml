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

    Image {
        source: "qrc:/zone.xiv.astra.svg"

        fillMode: Image.PreserveAspectFit

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: Kirigami.Units.largeSpacing * 3
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            text: {
                if (page.isInitialSetup) {
                    return i18n("You must have a legitimate installation of the FFXIV to continue.");
                } else {
                    return i18n("Select a game installation of FFXIV for '%1'.", page.profile.name);
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Existing Installation")
        visible: LauncherCore.profileManager.hasAnyExistingInstallations()
    }

    FormCard.FormCard {
        visible: LauncherCore.profileManager.hasAnyExistingInstallations()

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

                text: profile.name
                description: profile.gamePath
                visible: profile.isGameInstalled

                onClicked: {
                    LauncherCore.currentProfile.gamePath = profile.gamePath;
                    applicationWindow().checkSetup();
                }
            }
        }

        FormCard.FormDelegateSeparator {
            below: importDelegate
        }

        FormCard.FormButtonDelegate {
            id: importDelegate

            text: i18n("Import Existing Installation…")
            description: i18n("Select an existing installation on disk or import from another launcher.")
            icon.name: "document-import-symbolic"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "ExistingSetup"), {
                profile: page.profile
            })
        }
    }

    FormCard.FormHeader {
        title: i18n("Retail Game")
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: downloadDelegate

            text: i18n("Download & Install Game")
            description: i18n("Download the retail installer online from Square Enix.")
            icon.name: "cloud-download"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "InstallProgress"), {
                gameInstaller: LauncherCore.createInstaller(page.profile)
            })
        }

        FormCard.FormDelegateSeparator {
            above: downloadDelegate
            below: selectInstallDelegate
        }

        FormCard.FormButtonDelegate {
            id: selectInstallDelegate

            text: i18n("Select Existing Installer…")
            description: i18n("Use a previously downloaded installer. Useful if offline or can't otherwise access the official servers.")
            icon.name: "edit-find"

            FileDialog {
                id: dialog

                currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
                nameFilters: [i18n("Windows executable (*.exe)")]

                onAccepted: {
                    const url = decodeURIComponent(selectedFile.toString().replace("file://", ""));
                    page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "InstallProgress"), {
                        gameInstaller: LauncherCore.createInstallerFromExisting(page.profile, url)
                    });
                }
            }

            onClicked: dialog.open()
        }
    }

    FormCard.FormHeader {
        title: i18n("Benchmark")
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: downloadBenchmark

            text: i18n("Download & Install Benchmark")
            description: i18n("Download the official benchmark from Square Enix.")
            icon.name: "cloud-download"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "BenchmarkInstallProgress"), {
                benchmarkInstaller: LauncherCore.createBenchmarkInstaller(page.profile)
            })
        }

        FormCard.FormDelegateSeparator {
            above: downloadBenchmark
            below: selectBenchmark
        }

        FormCard.FormButtonDelegate {
            id: selectBenchmark

            text: i18n("Select Existing Benchmark…")
            description: i18n("Use a previously downloaded benchmark. Useful if offline or can't otherwise access the official servers.")
            icon.name: "edit-find"

            FileDialog {
                id: benchmarkDialog

                currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
                nameFilters: [i18n("Benchmark zip archive (*.zip)")]

                onAccepted: {
                    const url = decodeURIComponent(selectedFile.toString().replace("file://", ""));
                    page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "BenchmarkInstallProgress"), {
                        benchmarkInstaller: LauncherCore.createBenchmarkInstallerFromExisting(page.profile, url)
                    });
                }
            }

            onClicked: benchmarkDialog.open()
        }
    }
}