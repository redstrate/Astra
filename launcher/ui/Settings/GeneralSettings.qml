// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

MobileForm.FormCard {
    Layout.topMargin: Kirigami.Units.largeSpacing
    Layout.fillWidth: true
    contentItem: ColumnLayout {
        spacing: 0

        MobileForm.FormCardHeader {
            title: i18n("General")
        }

        MobileForm.FormDelegateSeparator {
        }

        MobileForm.FormCheckDelegate {
            text: i18n("Close Astra when game is launched")
            checked: LauncherCore.closeWhenLaunched
            onCheckedChanged: LauncherCore.closeWhenLaunched = checked
        }

        MobileForm.FormDelegateSeparator {
        }

        MobileForm.FormCheckDelegate {
            text: i18n("Show news banners")
            checked: LauncherCore.showNewsBanners
            onCheckedChanged: LauncherCore.showNewsBanners = checked
        }

        MobileForm.FormDelegateSeparator {
        }

        MobileForm.FormCheckDelegate {
            text: i18n("Show news list")
            checked: LauncherCore.showNewsList
            onCheckedChanged: LauncherCore.showNewsList = checked
        }
    }
}