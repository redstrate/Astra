// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

Kirigami.Page {
    id: page

    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None

    RowLayout {
        width: parent.width
        height: parent.height

        Controls.ScrollView {
            id: scrollView

            Layout.fillWidth: true
            Layout.fillHeight: true

            NewsPage {
                width: scrollView.width
                height: scrollView.height
            }
        }
        LoginPage {
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
        }
    }
}