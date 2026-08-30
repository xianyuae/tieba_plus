import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

Item {
    id: root
    property variant stack: null

    property int status: 0
    property string errorText: S.S0039
    property variant forums: []

    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 56
        color: appTheme.cardBackground
        Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 2; color: appTheme.accent }

        Text {
            font.family: appTheme.fontFamily
            anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
            text: S.S0053
            color: appTheme.textPrimary
            font.pixelSize: appTheme.fontLarge
            font.bold: true
        }
    }

    ListView {
        id: list
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: forums

        delegate: Item {
            width: list.width
            height: 72
            Rectangle {
                id: card
                anchors.fill: parent
                color: appTheme.cardBackground
            }
            Row {
                anchors { fill: card; leftMargin: 14; rightMargin: 14 }
                spacing: 12
                Avatar { source: modelData.avatar; size: 44; anchors.verticalCenter: parent.verticalCenter }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3
                    width: card.width - 84
                    Text {
                        font.family: appTheme.fontFamily
                        width: parent.width
                        text: modelData.name
                        color: appTheme.textPrimary
                        font.pixelSize: appTheme.fontMedium
                        font.bold: true
                        elide: Text.ElideRight
                    }
                    Text {
                        font.family: appTheme.fontFamily
                        text: S.S0055 + (modelData.level !== undefined ? modelData.level : "")
                        color: appTheme.textTertiary
                        font.pixelSize: appTheme.fontSmall
                    }
                }
            }
            MouseArea {
                anchors.fill: card
                onClicked: root.stack.push(Qt.createComponent("ForumPage.qml"), { kw: modelData.name, fid: modelData.fid })
            }
        }
    }

    StatusView {
        anchors.fill: parent
        visible: root.status !== 1
        status: root.status === 0 ? 0 : (root.status === 3 ? 2 : 1)
        errorText: root.errorText
        emptyText: S.S0058
        onRetry: load()
    }

    Connections {
        target: api
        onFollowedForumsReady: {
            if (error !== "") {
                root.status = 3
                root.errorText = error
            } else {
                root.forums = forums
                root.status = forums.length > 0 ? 1 : 2
            }
        }
    }

    function load() {
        root.status = 0
        root.forums = []
        api.loadFollowedForums()
    }

    Component.onCompleted: load()
}
