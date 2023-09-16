// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick.Dialogs

import org.kde.kirigamiaddons.labs.mobileform as MobileForm

MobileForm.FormButtonDelegate {
    id: control

    property string file

    icon.name: "document-open"
    description: file

    onClicked: dialog.open()

    FileDialog {
        id: dialog

        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
    }
}