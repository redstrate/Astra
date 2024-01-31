// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick.Dialogs

import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormButtonDelegate {
    id: control

    property string file

    signal accepted(string path)

    icon.name: "document-open"
    description: file

    onClicked: dialog.open()

    FileDialog {
        id: dialog

        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]

        onAccepted: control.accepted(decodeURIComponent(selectedFile.toString().replace("file://", "")))
    }
}