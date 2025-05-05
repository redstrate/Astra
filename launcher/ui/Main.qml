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

    title: pageStack.currentItem?.title ?? ""

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

    function checkSetup(): void {
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
        } else if (!LauncherCore.currentProfile.account && !LauncherCore.currentProfile.config.isBenchmark) {
            // User must select an account for the profile
            if (LauncherCore.accountManager.hasAnyAccounts()) {
                // They have another account they could select
                pageStack.push(Qt.createComponent("zone.xiv.astra", "AccountSetup"), {
                    profile: LauncherCore.currentProfile
                });
            } else {
                // They have no pre-existing accounts, and should be shoved into the new account page
                pageStack.push(Qt.createComponent("zone.xiv.astra", "AddSquareEnix"), {
                    profile: LauncherCore.currentProfile
                });
            }
        } else {
            if (LauncherCore.autoLoginProfile && !checkedAutoLogin) {
                pageStack.push(Qt.createComponent("zone.xiv.astra", "AutoLoginPage"))
                checkedAutoLogin = true;
            } else {
                pageStack.push(Qt.createComponent("zone.xiv.astra", "MainPage"))
            }
        }
    }

    function cancelAutoLogin(): void {
        pageStack.clear();
        pageStack.layers.clear();
        pageStack.push(Qt.createComponent("zone.xiv.astra", "MainPage"));
    }

    function pushDialogLayer(url: string): void {
        if (LauncherCore.isSteamDeck) {
            pageStack.layers.push(url)
        } else {
            pageStack.pushDialogLayer(url)
        }
    }

    function openUrl(url: string): void {
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

        function onLoadingFinished(): void {
            appWindow.checkSetup();
        }

        function onSuccessfulLaunch(): void {
            if (LauncherCore.config.closeWhenLaunched) {
                appWindow.hide();
            } else {
                appWindow.checkSetup();
            }
        }

        function onGameClosed(): void {
            if (!LauncherCore.config.closeWhenLaunched) {
                appWindow.checkSetup();
            }
        }

        function onCurrentProfileChanged(): void {
            appWindow.checkSetup();
        }

        function onShowWindow(): void {
            appWindow.show();
        }
    }

    Component.onCompleted: checkSetup()
}
