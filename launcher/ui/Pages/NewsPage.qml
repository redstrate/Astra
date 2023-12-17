// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

QQC2.Control {
    id: page

    Component.onCompleted: LauncherCore.refreshNews()

    property int currentBannerIndex: 0
    property int numBannerImages: 0

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
                onEntered: applicationWindow().hoverLinkIndicator.text = LauncherCore.headline.banners[page.currentBannerIndex].link
                onExited: applicationWindow().hoverLinkIndicator.text = ""
            }

            layer.enabled: true
            layer.effect: OpacityMask {
                maskSource: Item {
                    width: bannerImage.width
                    height: bannerImage.height
                    Rectangle {
                        anchors.centerIn: parent
                        width: bannerImage.width
                        height: bannerImage.height
                        radius: Kirigami.Units.smallSpacing
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
                    onHoveredChanged: {
                        if (hovered) {
                            applicationWindow().hoverLinkIndicator.text = modelData.url;
                        } else {
                            applicationWindow().hoverLinkIndicator.text = "";
                        }
                    }
                }
            }

            FormCard.FormTextDelegate {
                description: i18n("No news.")
                visible: LauncherCore.headline !== null ? LauncherCore.headline.failedToLoad : false
            }
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
                    onHoveredChanged: {
                        if (hovered) {
                            applicationWindow().hoverLinkIndicator.text = modelData.url;
                        } else {
                            applicationWindow().hoverLinkIndicator.text = "";
                        }
                    }
                }
            }

            FormCard.FormTextDelegate {
                description: i18n("No topics.")
                visible: LauncherCore.headline !== null ? LauncherCore.headline.failedToLoad : false
            }
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