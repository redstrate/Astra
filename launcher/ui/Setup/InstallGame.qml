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

    title: i18n("Install Game")

    data: FolderDialog {
        id: installFolderDialog

        onAccepted: page.profile.config.gamePath = decodeURIComponent(selectedFolder.toString().replace("file://", ""))
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: selectInstallFolder

            icon.name: "document-open-folder"
            text: i18n("Select Install Folder")
            description: profile.config.gamePath

            onClicked: installFolderDialog.open()
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: downloadDelegate

            text: i18n("Download Installer")
            description: i18n("Download the installer from the official website.")
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
            description: i18n("Use this if you're offline or are unable to access the server.")
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
}
