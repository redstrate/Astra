// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import zone.xiv.astra

Kirigami.ApplicationWindow {
    id: appWindow

    width: 1280
    height: 800

    minimumWidth: 800
    minimumHeight: 500

    title: pageStack.currentItem !== undefined ? pageStack.currentItem.title : ""

    property bool checkedAutoLogin: false

    pageStack {
        globalToolBar {
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            // TODO: they should really do this check in kirigami
            showNavigationButtons: if (pageStack.currentItem?.globalToolBarStyle === Kirigami.ApplicationHeaderStyle.ToolBar) {
                Kirigami.ApplicationHeaderStyle.ShowBackButton
            } else {
                Kirigami.ApplicationHeaderStyle.NoNavigationButtons
            }
        }
        initialPage: Kirigami.Page {
            Kirigami.LoadingPlaceholder {
                anchors.centerIn: parent
            }
        }
    }

    onClosing: (close) => {
        if (LauncherCore.isPatching()) {
            applicationWindow().showPassiveNotification(i18n("Please do not quit while patching!"));
        }
        close.accepted = !LauncherCore.isPatching();
    }

    function checkSetup() {
        if (!LauncherCore.loadingFinished) {
            return
        }

        pageStack.clear()
        pageStack.layers.clear();

        if (!LauncherCore.currentProfile.isGameInstalled) {
            // User must set up the profile
            pageStack.push(Qt.createComponent("zone.xiv.astra", "SetupPage"), {
                profile: LauncherCore.currentProfile
            })
        } else if (!LauncherCore.currentProfile.account && !LauncherCore.currentProfile.isBenchmark) {
            // User must select an account for the profile
            pageStack.push(Qt.createComponent("zone.xiv.astra", "AccountSetup"), {
                profile: LauncherCore.currentProfile
            })
        } else {
            if (LauncherCore.autoLoginProfile && !checkedAutoLogin) {
                pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "AutoLoginPage"))
                checkedAutoLogin = true;
            } else {
                pageStack.push(Qt.createComponent("zone.xiv.astra", "MainPage"))
            }
        }
    }

    function cancelAutoLogin() {
        pageStack.clear();
        pageStack.layers.clear();
        pageStack.push(Qt.createComponent("zone.xiv.astra", "MainPage"));
    }

    function pushDialogLayer(url) {
        if (LauncherCore.isSteamDeck) {
            pageStack.layers.push(url)
        } else {
            pageStack.pushDialogLayer(url)
        }
    }

    function openUrl(url) {
        if (LauncherCore.isSteamDeck) {
            appWindow.pageStack.layers.push(Qt.createComponent("zone.xiv.astra", "BrowserPage"), {
                url: url
            })
        } else {
            Qt.openUrlExternally(url)
        }
    }

    Connections {
        target: LauncherCore

        function onLoadingFinished() {
            appWindow.checkSetup();
        }

        function onSuccessfulLaunch() {
            if (LauncherCore.settings.closeWhenLaunched) {
                appWindow.hide();
            } else {
                appWindow.checkSetup();
            }
        }

        function onGameClosed() {
            if (!LauncherCore.settings.closeWhenLaunched) {
                appWindow.checkSetup();
            }
        }

        function onCurrentProfileChanged() {
            appWindow.checkSetup();
        }
    }

    Connections {
        target: LauncherCore.settings

        function onShowNewsChanged() {
            // workaround annoying Qt layout bug
            // TODO: see if this changed in Qt7
            appWindow.pageStack.replace(Qt.createComponent("zone.xiv.astra", "MainPage"))
        }
    }

    Component.onCompleted: checkSetup()
}
