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

    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    topPadding: Kirigami.Units.largeSpacing
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: 0

    RowLayout {
        anchors.fill: parent

        spacing: Kirigami.Units.largeSpacing

        Loader {
            active: LauncherCore.settings.showNews

            Layout.fillWidth: true
            Layout.fillHeight: true

            sourceComponent: QQC2.ScrollView {
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
            Layout.minimumWidth: LauncherCore.settings.showNews ? Kirigami.Units.gridUnit * 26 : 0
            Layout.fillWidth: !LauncherCore.settings.showNews
        }
    }
}