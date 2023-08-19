// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import zone.xiv.astra 1.0

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