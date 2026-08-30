import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

                                                              
Page {
    id: root
    property bool saving: false

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
    }

    Rectangle { anchors.fill: parent; color: appTheme.background }

    Flickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: contentCol.height + 20

        Column {
            id: contentCol
            anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 12 }
            spacing: 8

            Text {
                font.family: appTheme.fontFamily
               anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: accounts.loggedIn ? S.S0067 + (accounts.name !== "" ? accounts.name : S.S0068) : S.S0069
               color: appTheme.textPrimary
                font.pixelSize: appTheme.fontLarge
                font.bold: true
            }

            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: S.S0070
                color: appTheme.textSecondary
                font.pixelSize: appTheme.fontSmall
                wrapMode: Text.WordWrap
            }

            Text { font.family: appTheme.fontFamily; anchors { left: parent.left; leftMargin: 12 } text: S.S0071; color: appTheme.textSecondary; font.pixelSize: appTheme.fontSmall }
            TextArea {
                id: bdussInput
                platformStyle: TextAreaStyle { textFont.family: appTheme.fontFamily }
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                height: 84
                text: accounts.bduss
            }

            Text { font.family: appTheme.fontFamily; anchors { left: parent.left; leftMargin: 12 } text: S.S0072; color: appTheme.textSecondary; font.pixelSize: appTheme.fontSmall }
            TextField {
                id: stokenInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: accounts.stoken
            }

            Text { font.family: appTheme.fontFamily; anchors { left: parent.left; leftMargin: 12 } text: S.S0073; color: appTheme.textSecondary; font.pixelSize: appTheme.fontSmall }
            TextField {
                id: tbsInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: accounts.tbs
            }

            Text { font.family: appTheme.fontFamily; anchors { left: parent.left; leftMargin: 12 } text: S.S0074; color: appTheme.textSecondary; font.pixelSize: appTheme.fontSmall }
            TextField {
                id: baiduidInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: settings.baiduId
            }

            Button {
               anchors { left: parent.left; leftMargin: 12 }
                text: root.saving ? S.S0075 : S.S0076
               font.family: appTheme.fontFamily
                enabled: !root.saving && bdussInput.text !== ""
                onClicked: {
                    root.saving = true
                    api.login(bdussInput.text, stokenInput.text, tbsInput.text)
                }
            }

            Item { width: 1; height: 8 }
            Rectangle {
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                height: 1
                color: appTheme.divider
            }

            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; right: parent.right; leftMargin: 12; rightMargin: 12 }
                text: S.S0187 + " · " + S.S0188
                color: appTheme.textSecondary
                font.pixelSize: appTheme.fontSmall
                wrapMode: Text.WordWrap
            }

            Button {
                anchors { left: parent.left; leftMargin: 12 }
                text: S.S0187
                font.family: appTheme.fontFamily
                enabled: !root.saving
                onClicked: pageStack.push(Qt.resolvedUrl("QrLoginPage.qml"))
            }
        }
    }

    Connections {
        target: api
        onActionFinished: {
            if (action !== "login") return
            root.saving = false
           if (ok) {
               settings.baiduId = baiduidInput.text
                notifier.notify(S.S0077, S.S0078)
               pageStack.pop()
           } else {
                notifier.notify(S.S0079, message !== "" ? message : S.S0080)
            }
        }
    }
}
