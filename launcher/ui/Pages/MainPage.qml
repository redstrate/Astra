// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import zone.xiv.astra

Kirigami.Page {
    id: page

    title: i18n("Home")

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Server Status")
            icon.name: "cloudstatus"
            onTriggered: applicationWindow().openUrl('https://na.finalfantasyxiv.com/lodestone/worldstatus/')
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Settings")
            icon.name: "configure"
            onTriggered: Qt.createComponent("zone.xiv.astra", "SettingsPage").createObject().open()
        }
    ]

    RowLayout {
        anchors.fill: parent

        spacing: 0

        LoginPage {
            id: loginPage

            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            Layout.minimumWidth: Kirigami.Units.gridUnit * 26
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillHeight: true
        }

        Kirigami.Separator {
            Layout.fillHeight: true
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: QQC2.ScrollView {
                id: scrollView

                NewsPage {
                    width: scrollView.availableWidth
                    height: Math.max(scrollView.availableHeight, page.height, implicitHeight)
                }
            }
        }
    }
}
