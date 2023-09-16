// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtCore
import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import QtQuick.Dialogs
import zone.xiv.astra 1.0

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