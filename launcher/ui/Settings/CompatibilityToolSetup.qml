// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import zone.xiv.astra 1.0

Kirigami.Page {
    id: page

    property var installer: null

    title: i18n("Install Compatibility Tool")

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Compatibility Tool")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Press the button below to install the compatibility tool for Steam.")
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormButtonDelegate {
                    text: i18n("Install Tool")
                    icon.name: "install"
                    onClicked: {
                        page.installer = LauncherCore.createCompatInstaller();
                        page.installer.installCompatibilityTool();
                    }
                }

                MobileForm.FormDelegateSeparator {}

                MobileForm.FormButtonDelegate {
                    text: i18n("Remove Tool")
                    icon.name: "delete"
                    onClicked: {
                        page.installer = LauncherCore.createCompatInstaller();
                        page.installer.removeCompatibilityTool();
                    }
                }
            }
        }
    }

    Kirigami.PromptDialog {
        id: errorDialog
        title: i18n("Install error")

        showCloseButton: false
        standardButtons: Kirigami.Dialog.Ok

        onAccepted: applicationWindow().pageStack.layers.pop()
        onRejected: applicationWindow().pageStack.layers.pop()
    }

    Connections {
        enabled: page.installer !== null
        target: page.installer

        function onInstallFinished() {
            applicationWindow().pageStack.layers.pop()
        }

        function onError(message) {
            errorDialog.subtitle = message
            errorDialog.open()
        }
    }
}