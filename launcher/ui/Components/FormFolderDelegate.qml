// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: root

    property string folder
    property string displayText: folder

    signal accepted(string folder)

    onClicked: dialog.open()

    contentItem: RowLayout {
        spacing: Kirigami.Units.mediumSpacing

        ColumnLayout {
            spacing: Kirigami.Units.mediumSpacing

            Layout.fillWidth: true

            QQC2.Label {
                text: root.text

                Layout.fillWidth: true
            }

            QQC2.Label {
                text: root.displayText
                elide: Text.ElideRight
                maximumLineCount: 1
                color: Kirigami.Theme.disabledTextColor

                Layout.fillWidth: true
            }
        }

        QQC2.ToolButton {
            text: i18n("Open Folder")
            icon.name: "document-open-folder"
            display: QQC2.AbstractButton.IconOnly
            onClicked: Qt.openUrlExternally("file://" + root.folder)

            QQC2.ToolTip.text: text
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
        }

        FormCard.FormArrow {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            direction: Qt.RightArrow
        }
    }

    FolderDialog {
        id: dialog

        currentFolder: "file://" + root.folder
        selectedFolder: "file://" + root.folder

        onAccepted: root.accepted(decodeURIComponent(selectedFolder.toString().replace("file://", "")))
    }
}