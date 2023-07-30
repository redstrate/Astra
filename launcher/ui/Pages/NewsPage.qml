// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

Kirigami.ScrollablePage {
    id: page

    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    Component.onCompleted: LauncherCore.refreshNews()

    property int currentBannerIndex: 0
    property int numBannerImages: 0

    Connections {
        target: LauncherCore

        function onNewsChanged() {
            page.currentBannerIndex = 0
            page.numBannerImages = LauncherCore.headline.banners.length
            console.log(LauncherCore.headline.banners)
        }
    }

    Timer {
        interval: 10000
        running: true
        repeat: true
        onTriggered: {
            if (page.currentBannerIndex + 1 === page.numBannerImages) {
                page.currentBannerIndex = 0
            } else {
                page.currentBannerIndex++
            }
        }
    }

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Banner")
                }

                Image {
                    Layout.fillWidth: true

                    source: LauncherCore.headline !== null ? LauncherCore.headline.banners[page.currentBannerIndex].bannerImage : ""

                    fillMode: Image.PreserveAspectFit

                    MouseArea {
                        anchors.fill: parent

                        cursorShape: Qt.PointingHandCursor

                        onClicked: Qt.openUrlExternally(LauncherCore.headline.banners[page.currentBannerIndex].link)
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("News")
                }

                Repeater {
                    model: LauncherCore.headline.news

                    MobileForm.FormButtonDelegate {
                        text: modelData.title
                        description: Qt.formatDate(modelData.date)

                        onClicked: Qt.openUrlExternally(modelData.url)
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Topics")
                }

                Repeater {
                    model: LauncherCore.headline.topics

                    MobileForm.FormButtonDelegate {
                        text: modelData.title
                        description: Qt.formatDate(modelData.date)

                        onClicked: Qt.openUrlExternally(modelData.url)
                    }
                }
            }
        }
    }
}