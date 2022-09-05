import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQuick.Window 2.0
import Astra 1.0

ApplicationWindow {
    id: window

    width:  640
    height: 480

    visible: true

    Label {
        id: usernameLabel

        text: "Username"
    }

    TextField {
        id: usernameField

        anchors.top: usernameLabel.bottom
    }

    Label {
        id: passwordLabel

        text: "Password"

        anchors.top: usernameField.bottom
    }

    TextField {
        id: passwordField

        echoMode: TextInput.Password

        anchors.top: passwordLabel.bottom
    }

    Label {
        id: otpLabel

        text: "One-Time Password"

        anchors.top: passwordField.bottom
    }

    TextField {
        id: otpField

        anchors.top: otpLabel.bottom
    }

    Button {
        id: loginButton

        text: "Login"

        anchors.top: otpField.bottom

        onClicked: {
            var info = core.createNewLoginInfo();
            info.settings = core.getProfileQML(0);
            info.username = usernameField.text
            info.password = passwordField.text
            info.oneTimePassword = otpField.text

            print(info);
            core.squareBoot.checkGateStatus(info);
        }
    }
}