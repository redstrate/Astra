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

    title: i18n("Install Benchmark")

    data: FolderDialog {
        id: installFolderDialog

        onAccepted: page.profile.config.gamePath = decodeURIComponent(selectedFolder.toString().replace("file://", ""))
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing * 4

        FormCard.FormButtonDelegate {
            id: selectInstallFolder

            icon.name: "document-open-folder"
            text: i18n("Select Install Folder")
            description: profile.isGamePathDefault ? i18n("Default Location") : profile.config.gamePath

            onClicked: installFolderDialog.open()
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: downloadBenchmark

            text: i18n("Download & Install Benchmark")
            description: i18n("Download the benchmark from the official website.")
            icon.name: "cloud-download"
            onClicked: page.Window.window.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "BenchmarkInstallProgress"), {
                benchmarkInstaller: LauncherCore.createBenchmarkInstaller(page.profile)
            })
            focus: true
        }

        FormCard.FormDelegateSeparator {
            above: downloadBenchmark
            below: selectBenchmark
        }

        FormCard.FormButtonDelegate {
            id: selectBenchmark

            text: i18n("Select Existing Benchmark…")
            description: i18n("Use this if you're offline or are unable to access the server.")
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
