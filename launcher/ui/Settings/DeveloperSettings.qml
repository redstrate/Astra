// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import zone.xiv.astra 1.0

import "../Components"

Kirigami.ScrollablePage {
    id: page

    title: i18n("Developer Settings")

    ColumnLayout {
        width: parent.width

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCheckDelegate {
                    text: i18n("Keep Patches")
                    description: i18n("Do not delete patches after they're used. Astra will not redownload patch data, if found.")
                }
            }
        }
    }
}