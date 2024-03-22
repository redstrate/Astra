// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtWebView

import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: page

    property var url

    title: i18n("Web Browser")

    padding: 0

    WebView {
        anchors.fill: parent

        url: page.url

        Component.onCompleted: setCookie("finalfantasyxiv.com", "ldst_bypass_browser", "1")
    }
}