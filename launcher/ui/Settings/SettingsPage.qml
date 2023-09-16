// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import org.kde.kirigami 2.18 as Kirigami
import org.kde.kirigamiaddons.settings 1.0 as KirigamiSettings
import QtQuick.Layouts 1.15
import zone.xiv.astra 1.0

KirigamiSettings.CategorizedSettings {
    id: settingsPage

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
            actionName: "compattool"
            text: i18n("Compatibility Tool")
            icon.name: "system-run"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/CompatibilityToolSetup.qml")
        },
        KirigamiSettings.SettingAction {
            actionName: "devtool"
            text: i18n("Developer Settings")
            icon.name: "preferences-others"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/DeveloperSettings.qml")
            visible: LauncherCore.showDevTools
        },
        KirigamiSettings.SettingAction {
            actionName: "about"
            text: i18n("About Astra")
            icon.name: "help-about"
            page: Qt.resolvedUrl("/qt/qml/zone/xiv/astra/ui/Settings/AboutPage.qml")
        }
    ]
}
