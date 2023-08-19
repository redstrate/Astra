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
        title: i18n("Profiles")
    }

    FormCard.FormCard {
        Layout.fillWidth: true

        Repeater {
            model: LauncherCore.profileManager

            FormCard.FormButtonDelegate {
                required property var profile

                text: profile.name
                onClicked: applicationWindow().pageStack.layers.push('qrc:/ui/Settings/ProfileSettings.qml', {
                    profile: profile
                })
            }
        }

        FormCard.FormDelegateSeparator {
            below: addProfileButton
        }

        FormCard.FormButtonDelegate {
            id: addProfileButton

            text: i18n("Add Profile")
            icon.name: "list-add"
            onClicked: {
                applicationWindow().currentSetupProfile = LauncherCore.profileManager.addProfile()
                applicationWindow().checkSetup()
            }
        }
    }
}