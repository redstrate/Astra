// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import QtGraphicalEffects 1.0
import zone.xiv.astra 1.0

Controls.Control {
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
            if (page.currentBannerIndex + 1 === page.numBannerImages) {
                page.currentBannerIndex = 0
            } else {
                page.currentBannerIndex++
            }
        }
    }

    contentItem: ColumnLayout {
        id: layout

        readonly property real maximumWidth: Kirigami.Units.gridUnit * 50

        Image {
            id: bannerImage

            readonly property real aspectRatio: sourceSize.height / sourceSize.width

            Layout.maximumWidth: layout.maximumWidth
            Layout.fillWidth: true
            Layout.preferredHeight: aspectRatio * width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            source: LauncherCore.headline !== null ? LauncherCore.headline.banners[page.currentBannerIndex].bannerImage : ""

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

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            maximumWidth: layout.maximumWidth
            visible: LauncherCore.headline !== null

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("News")
                }

                Repeater {
                    model: LauncherCore.headline !== null ? LauncherCore.headline.news : undefined

                    MobileForm.FormButtonDelegate {
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

                MobileForm.FormTextDelegate {
                    description: i18n("No news.")
                    visible: LauncherCore.headline !== null ? LauncherCore.headline.failedToLoad : false
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            maximumWidth: layout.maximumWidth
            visible: LauncherCore.headline !== null

            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Topics")
                }

                Repeater {
                    model: LauncherCore.headline !== null ? LauncherCore.headline.topics : undefined

                    MobileForm.FormButtonDelegate {
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

                MobileForm.FormTextDelegate {
                    description: i18n("No topics.")
                    visible: LauncherCore.headline !== null ? LauncherCore.headline.failedToLoad : false
                }
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