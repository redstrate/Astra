// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import org.kde.kirigamiaddons.formcard as FormCard

import zone.xiv.astra

FormCard.FormCardPage {
    title: i18n("General")

    FormCard.FormHeader {
        title: i18n("General settings")
    }

    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: closeAstraDelegate

            text: i18n("Close Astra when game is launched")
            checked: LauncherCore.closeWhenLaunched
            onCheckedChanged: LauncherCore.closeWhenLaunched = checked
        }

        FormCard.FormDelegateSeparator {
            above: closeAstraDelegate
            below: showNewsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showNewsDelegate

            text: i18n("Enable and show news")
            checked: LauncherCore.showNews
            onCheckedChanged: LauncherCore.showNews = checked
        }

        FormCard.FormDelegateSeparator {
            above: showNewsDelegate
            below: showDevToolsDelegate
        }

        FormCard.FormCheckDelegate {
            id: showDevToolsDelegate

            text: i18n("Show Developer Settings")
            checked: LauncherCore.showDevTools
            onCheckedChanged: LauncherCore.showDevTools = checked
        }
    }
}