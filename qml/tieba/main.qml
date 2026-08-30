import QtQuick 1.1
import com.nokia.meego 1.0
import "util.js" as U
import "strings.js" as S

PageStackWindow {
    id: appWindow
    property bool loginRedirected: false
    initialPage: mainPage
    showStatusBar: true

    MainPage {
        id: mainPage
    }

                            
    Rectangle {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 120
        width: toastText.width + 44
        height: toastText.height + 26
        radius: 10
        color: appTheme.dark ? "#cc222222" : "#cc000000"
        opacity: 0
        visible: false

        Text {
            font.family: appTheme.fontFamily
            id: toastText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: appTheme.fontSmall
        }

        function show(msg) {
            toastText.text = msg
            toast.visible = true
            toast.opacity = 1
            toastTimer.restart()
        }

        Timer {
            id: toastTimer
            interval: 2200
            onTriggered: toast.opacity = 0
        }

        Behavior on opacity { NumberAnimation { duration: 300 } }
    }

    Connections {
        target: util
        onToast: toast.show(message)
    }

    Connections {
        target: api
        onLoginExpired: {
            if (appWindow.loginRedirected) return
            appWindow.loginRedirected = true
           accounts.logout()
            toast.show(S.S0184)
           pageStack.push(Qt.createComponent("LoginPage.qml"), {})
        }
    }

    Connections {
        target: accounts
        onAccountChanged: if (accounts.loggedIn) appWindow.loginRedirected = false
    }

                                                                                   
}
