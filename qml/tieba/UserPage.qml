import QtQuick 1.1
import com.nokia.meego 1.0
import "util.js" as U
import "components"
import "strings.js" as S

           
Item {
    id: root
    property variant stack: null

                     
    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 110
        color: appTheme.cardBackground
        Row {
            anchors { fill: parent; leftMargin: 16; rightMargin: 16 }
            spacing: 14
            Avatar {
                id: avatar
                anchors.verticalCenter: parent.verticalCenter
                size: 64
                source: U.avatarUrl(accounts.portrait)
            }
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4
                Text {
                    font.family: appTheme.fontFamily
                    text: accounts.loggedIn ? (accounts.name !== "" ? accounts.name : S.S0155) : S.S0156
                    color: appTheme.textPrimary
                    font.pixelSize: appTheme.fontLarge
                    font.bold: true
                }
                Text {
                    font.family: appTheme.fontFamily
                    text: accounts.loggedIn ? S.S0157 : S.S0158
                    color: appTheme.textTertiary
                    font.pixelSize: appTheme.fontSmall
                }
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (accounts.loggedIn)
                    root.stack.push(Qt.createComponent("UserProfilePage.qml"), { uid: accounts.uid })
                else
                    root.stack.push(Qt.createComponent("LoginPage.qml"), {})
            }
        }
    }

              
    ListView {
        id: list
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: entryModel
        delegate: Item {
            width: list.width; height: 52
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
                text: modelData.title
                color: appTheme.textPrimary
                font.pixelSize: appTheme.fontMedium
            }
            Text {
                font.family: appTheme.fontFamily
                anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                text: ">"
                color: appTheme.textTertiary
                font.pixelSize: appTheme.fontLarge
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea { anchors.fill: parent; onClicked: handleEntry(modelData.id) }
        }
    }

    property variant entryModel: [
        { id: "posts", title: S.S0159 },
        { id: "store", title: S.S0160 },
        { id: "draft", title: S.S0161 },
        { id: "history", title: S.S0162 },
        { id: "blacklist", title: S.S0163 },
        { id: "settings", title: S.S0083 },
        { id: "about", title: S.S0084 },
        { id: "logout", title: S.S0164 }
    ]

    function handleEntry(id) {
        if (id === "posts") root.stack.push(Qt.createComponent("UserProfilePage.qml"), { uid: accounts.uid })
        else if (id === "store") root.stack.push(Qt.createComponent("StorePage.qml"), {})
        else if (id === "draft") root.stack.push(Qt.createComponent("DraftPage.qml"), {})
        else if (id === "history") root.stack.push(Qt.createComponent("HistoryPage.qml"), {})
        else if (id === "blacklist") root.stack.push(Qt.createComponent("BlacklistPage.qml"), {})
        else if (id === "settings") root.stack.push(Qt.createComponent("SettingsPage.qml"), {})
        else if (id === "about") root.stack.push(Qt.createComponent("AboutPage.qml"), {})
        else if (id === "logout") { accounts.logout() }
    }
}
