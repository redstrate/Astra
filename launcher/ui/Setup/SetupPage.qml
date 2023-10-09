// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
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
                if (isInitialSetup) {
                    return i18n("You must have a legitimate installation of the FFXIV to continue.");
                } else {
                    return i18n("You must select a legitimate installation of FFXIV for '%1'", page.profile.name);
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18n("Existing Installations")
        visible: LauncherCore.profileManager.hasAnyExistingInstallations()
    }

    FormCard.FormCard {
        visible: LauncherCore.profileManager.hasAnyExistingInstallations()

        Layout.fillWidth: true

        FormCard.FormTextDelegate {
            id: existingHelpDelegate

            text: i18n("You can select an existing installation from another profile.")
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
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true

        FormCard.FormButtonDelegate {
            id: findExistingDelegate

            text: i18n("Find Existing Installation")
            icon.name: "edit-find"
            onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "ExistingSetup"), {
                profile: page.profile
            })
        }

        FormCard.FormDelegateSeparator {
            above: findExistingDelegate
            below: downloadDelegate
        }

        FormCard.FormButtonDelegate {
            id: downloadDelegate

            text: i18n("Download Game")
            icon.name: "cloud-download"
            onClicked: pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "DownloadSetup"), {
                profile: page.profile
            })
        }
    }
}