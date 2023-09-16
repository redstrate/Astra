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

    padding: 0

    title: i18n("Home")

    actions: [
        Kirigami.Action {
            text: i18n("Tools")
            icon.name: "tools-symbolic"

            Kirigami.Action {
                text: i18n("Open Official Launcher")
                icon.name: "application-x-executable"
                onTriggered: LauncherCore.openOfficialLauncher(loginPage.profile)
            }

            Kirigami.Action {
                text: i18n("Open System Info")
                icon.name: "application-x-executable"
                onTriggered: LauncherCore.openSystemInfo(loginPage.profile)
            }

            Kirigami.Action {
                text: i18n("Open Config Backup")
                icon.name: "application-x-executable"
                onTriggered: LauncherCore.openConfigBackup(loginPage.profile)
            }
        },
        Kirigami.Action {
            text: i18n("Server Status")
            icon.name: "cloudstatus"
            onTriggered: applicationWindow().openUrl('https://na.finalfantasyxiv.com/lodestone/worldstatus/')
        },
        Kirigami.Action {
            text: i18n("Settings")
            icon.name: "configure"
            onTriggered: applicationWindow().pushDialogLayer(Qt.createComponent("zone.xiv.astra", "SettingsPage"))
        }
    ]

    RowLayout {
        anchors.fill: parent

        Loader {
            active: LauncherCore.showNews

            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: Controls.ScrollView {
                id: scrollView

                NewsPage {
                    width: scrollView.availableWidth
                    height: scrollView.availableHeight
                }
            }
        }

        LoginPage {
            id: loginPage

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.minimumWidth: LauncherCore.showNews ? Kirigami.Units.gridUnit * 25 : 0
            Layout.fillWidth: !LauncherCore.showNews
        }
    }
}