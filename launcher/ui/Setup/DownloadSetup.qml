// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import com.redstrate.astra 1.0

Kirigami.Page {
    id: page

    property var profile

    title: i18n("Download Game")

    ColumnLayout {
        width: parent.width
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0

                MobileForm.FormCardHeader {
                    title: i18n("Download Game")
                }

                MobileForm.FormTextDelegate {
                    text: i18n("Press the button below to download and setup the game.")
                    description: i18n("This is for the base files required for start-up, only when logged in will Astra begin downloading the full game.")
                }

                MobileForm.FormDelegateSeparator {
                }

                MobileForm.FormButtonDelegate {
                    text: i18n("Begin installation")
                    icon.name: "cloud-download"
                    onClicked: pageStack.layers.push('qrc:/ui/Setup/InstallProgress.qml', {
                        gameInstaller: LauncherCore.createInstaller(page.profile)
                    })
                }
            }
        }
    }
}