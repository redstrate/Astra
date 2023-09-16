// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import QtWebView 1.10

Kirigami.Page {
    id: page

    property var url

    title: i18n("Web Browser")

    padding: 0

    WebView {
        anchors.fill: parent

        url: page.url
    }
}