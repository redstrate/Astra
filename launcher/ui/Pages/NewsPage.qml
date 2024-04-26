// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Effects

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

QQC2.Control {
    id: page

    property int currentBannerIndex: 0
    property int numBannerImages: 0

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    Component.onCompleted: LauncherCore.refreshNews()

    Connections {
        target: LauncherCore

        function onNewsChanged() {
            page.currentBannerIndex = 0
            page.numBannerImages = LauncherCore.headline.banners.length
        }
    }

    Timer {
        interval: 10000
        running: true
        repeat: true
        onTriggered: {
            if (page.numBannerImages === 0) {
                return;
            }

            if (page.currentBannerIndex + 1 === page.numBannerImages) {
                page.currentBannerIndex = 0;
            } else {
                page.currentBannerIndex++;
            }
        }
    }

    background: Rectangle {
        anchors.fill: parent

        color: Kirigami.Theme.backgroundColor

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
    }

    contentItem: ColumnLayout {
        id: layout

        readonly property real maximumWidth: Kirigami.Units.gridUnit * 50

        spacing: Kirigami.Units.largeSpacing

        Image {
            id: bannerImage

            readonly property real aspectRatio: sourceSize.height / sourceSize.width

            Layout.maximumWidth: layout.maximumWidth
            Layout.fillWidth: true
            Layout.preferredHeight: aspectRatio * width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.topMargin: Kirigami.Units.largeSpacing

            source: {
                if (LauncherCore.headline === null) {
                    return "";
                }

                if (page.numBannerImages === 0) {
                    return "";
                }

                return LauncherCore.headline.banners[page.currentBannerIndex].bannerImage;
            }

            MouseArea {
                anchors.fill: parent

                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true

                onClicked: applicationWindow().openUrl(LauncherCore.headline.banners[page.currentBannerIndex].link)
            }

            layer.enabled: !(bannerImage.width < layout.maximumWidth)
            layer.effect: MultiEffect {
                id: root

                maskEnabled: true
                maskSpreadAtMax: 1
                maskSpreadAtMin: 1
                maskThresholdMin: 0.5
                maskSource: ShaderEffectSource {
                    sourceItem: Rectangle {
                        width: root.width
                        height: root.height
                        radius: Kirigami.Units.mediumSpacing
                    }
                }
            }
        }

        FormCard.FormHeader {
            title: i18n("News")

            Layout.fillWidth: true
            maximumWidth: layout.maximumWidth
        }

        FormCard.FormCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            maximumWidth: layout.maximumWidth
            visible: LauncherCore.headline !== null

            Repeater {
                model: LauncherCore.headline !== null ? LauncherCore.headline.news : undefined

                FormCard.FormButtonDelegate {
                    required property var modelData

                    text: modelData.title
                    description: Qt.formatDate(modelData.date)

                    onClicked: applicationWindow().openUrl(modelData.url)
                }
            }

            FormCard.FormTextDelegate {
                description: i18n("No news.")
                visible: LauncherCore.headline !== null ? LauncherCore.headline.failedToLoad : false
            }

            Kirigami.Theme.colorSet: Kirigami.Theme.Window
        }

        FormCard.FormHeader {
            title: i18n("Topics")

            Layout.fillWidth: true
            maximumWidth: layout.maximumWidth
        }

        FormCard.FormCard {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            maximumWidth: layout.maximumWidth
            visible: LauncherCore.headline !== null

            Repeater {
                model: LauncherCore.headline !== null ? LauncherCore.headline.topics : undefined

                FormCard.FormButtonDelegate {
                    required property var modelData

                    text: modelData.title
                    description: Qt.formatDate(modelData.date)

                    hoverEnabled: true
                    onClicked: applicationWindow().openUrl(modelData.url)
                }
            }

            FormCard.FormTextDelegate {
                description: i18n("No topics.")
                visible: LauncherCore.headline !== null ? LauncherCore.headline.failedToLoad : false
            }

            Kirigami.Theme.colorSet: Kirigami.Theme.Window
        }

        Item {
            Layout.fillHeight: true
        }
    }

    Kirigami.LoadingPlaceholder {
        anchors.centerIn: parent
        visible: LauncherCore.headline === null
    }
}