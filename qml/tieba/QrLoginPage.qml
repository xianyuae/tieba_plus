import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

// Native QR login: shows the passport QR code, polls for scan/confirm and
// finishes via /c/s/login. No embedded browser involved.
Page {
    id: root
    property bool finishing: false

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: { qrlogin.cancel(); pageStack.pop() } }
        ToolButton { icon: "refresh"; onClicked: qrlogin.start() }
    }

    Component.onCompleted: qrlogin.start()
    Component.onDestruction: qrlogin.cancel()

    Rectangle { anchors.fill: parent; color: appTheme.background }

    Flickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: contentCol.height + 20

        Column {
            id: contentCol
            anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 16 }
            spacing: 14

            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: S.S0187
                color: appTheme.textPrimary
                font.pixelSize: appTheme.fontLarge
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: S.S0188
                color: appTheme.textSecondary
                font.pixelSize: appTheme.fontSmall
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Item { width: 1; height: 1 }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 264
                height: 264
                color: "white"
                radius: 8

                Image {
                    id: qrImage
                    anchors.centerIn: parent
                    width: 240
                    height: 240
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    sourceSize.width: 480
                    sourceSize.height: 480
                    source: ""
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: qrImage.source === "" || root.finishing
                    visible: running
                    platformStyle: BusyIndicatorStyle { size: "large" }
                }

                Text {
                    anchors.centerIn: parent
                    visible: qrlogin.state === qrlogin.Error
                    width: parent.width - 40
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    text: qrlogin.statusText
                    color: "#c62828"
                    font.family: appTheme.fontFamily
                    font.pixelSize: appTheme.fontMedium
                }
            }

            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: qrlogin.statusText
                color: appTheme.textSecondary
                font.pixelSize: appTheme.fontSmall
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: S.S0196
                font.family: appTheme.fontFamily
                onClicked: qrlogin.start()
            }
        }
    }

    Rectangle {
        visible: root.finishing
        anchors.fill: parent
        color: "#cc000000"
        Column {
            anchors.centerIn: parent
            spacing: 12
            BusyIndicator { running: true; anchors.horizontalCenter: parent.horizontalCenter; platformStyle: BusyIndicatorStyle { size: "large" } }
            Text {
                font.family: appTheme.fontFamily
                text: S.S0190
                color: "white"
                font.pixelSize: appTheme.fontMedium
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
        MouseArea { anchors.fill: parent }
    }

    Connections {
        target: qrlogin
        onQrImageReady: qrImage.source = filePath.indexOf("file://") === 0 ? filePath : "file://" + filePath
        onFinished: {
            if (!ok) {
                root.finishing = false
                notifier.notify(S.S0191, displayName !== "" ? displayName : S.S0192)
                return
            }
            if (root.finishing) return
            root.finishing = true
            api.login(bduss, stoken, "")
        }
        onStateChanged: {
            if (qrlogin.state === qrlogin.Error)
                root.finishing = false
        }
    }

    Connections {
        target: api
        onActionFinished: {
            if (action !== "login") return
            if (!root.finishing) return
            root.finishing = false
            qrlogin.cancel()
            if (ok) {
                notifier.notify(S.S0077, S.S0078)
                pageStack.pop()
            } else {
                notifier.notify(S.S0191, message !== "" ? message : S.S0192)
            }
        }
    }
}
