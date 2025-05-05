// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQml

import org.kde.kirigamiaddons.settings as KirigamiSettings
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import zone.xiv.astra

KirigamiSettings.ConfigurationView {
    id: settingsPage

    readonly property bool isInitialSetup: !LauncherCore.profileManager.hasAnyExistingInstallations()

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "general"
            text: i18n("General")
            icon.name: "zone.xiv.astra"
            page: () => Qt.createComponent("zone.xiv.astra", "GeneralSettings")
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "profiles"
            text: i18n("Profiles")
            icon.name: "applications-games-symbolic"
            page: () => Qt.createComponent("zone.xiv.astra", "ProfilesPage")
            visible: !settingsPage.isInitialSetup
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "accounts"
            text: i18n("Accounts")
            icon.name: "preferences-system-users-symbolic"
            page: () => Qt.createComponent("zone.xiv.astra", "AccountsPage")
            visible: !settingsPage.isInitialSetup
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "sync"
            text: i18n("Synchronization")
            icon.name: "state-sync-symbolic"
            page: () => Qt.createComponent("zone.xiv.astra", "SyncSettings")
            visible: LauncherCore.supportsSync()
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "compattool"
            text: i18n("Compatibility Tool")
            icon.name: "system-run-symbolic"
            page: () => Qt.createComponent("zone.xiv.astra", "CompatibilityToolSetup")
            visible: !LauncherCore.isWindows
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "devtool"
            text: i18n("Developer Settings")
            icon.name: "applications-development-symbolic"
            page: () => Qt.createComponent("zone.xiv.astra", "DeveloperSettings")
            visible: LauncherCore.config.showDevTools
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "about"
            text: i18n("About Astra")
            icon.name: "help-about-symbolic"
            page: () => Qt.createComponent("zone.xiv.astra", "AboutPage")
        }
    ]
}
