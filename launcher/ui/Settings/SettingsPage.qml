// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import org.kde.kirigamiaddons.settings as KirigamiSettings
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import zone.xiv.astra

KirigamiSettings.CategorizedSettings {
    id: settingsPage

    KirigamiComponents.FloatingButton {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        z: 100
        margins: Kirigami.Units.largeSpacing
        visible: LauncherCore.isSteamDeck

        action: Kirigami.Action {
            text: i18nc("@action:button", "Close Settings")
            icon.name: "dialog-close-symbolic"
            onTriggered: pageStack.layers.pop()
        }
    }

    actions: [
        KirigamiSettings.SettingAction {
            actionName: "general"
            text: i18n("General")
            icon.name: "zone.xiv.astra"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/GeneralSettings.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "profiles"
            text: i18n("Profiles")
            icon.name: "preferences-desktop-gaming"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/ProfilesPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "accounts"
            text: i18n("Accounts")
            icon.name: "preferences-system-users"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/AccountsPage.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "sync"
            text: i18n("Sync")
            icon.name: "state-sync-symbolic"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/SyncSettings.qml")
            visible: LauncherCore.supportsSync()
        },
        KirigamiSettings.SettingAction {
            actionName: "compattool"
            text: i18n("Compatibility Tool")
            icon.name: "system-run"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/CompatibilityToolSetup.qml")
            visible: !LauncherCore.isWindows
        },
        KirigamiSettings.SettingAction {
            actionName: "devtool"
            text: i18n("Developer Settings")
            icon.name: "preferences-others"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/DeveloperSettings.qml")
            visible: LauncherCore.settings.showDevTools
        },
        KirigamiSettings.SettingAction {
            actionName: "about"
            text: i18n("About Astra")
            icon.name: "help-about"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/AboutPage.qml")
        }
    ]
}
