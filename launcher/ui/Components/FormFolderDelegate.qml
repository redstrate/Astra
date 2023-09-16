// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick.Dialogs

import org.kde.kirigamiaddons.labs.mobileform as MobileForm

MobileForm.FormButtonDelegate {
    id: control

    property string folder

    icon.name: "document-open-folder"
    description: folder

    onClicked: dialog.open()

    FolderDialog {
        id: dialog

        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
    }
}