import QtQuick 1.1
import com.nokia.meego 1.0
import "util.js" as U
import "components"
import "strings.js" as S

             
Item {
    id: root
    property variant stack: null
    property int tab: 0                                
    property int loadState: 0
    property variant items: []
    property string errorText: S.S0039

    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 56
        color: appTheme.toolbarBackground
        Text {
            font.family: appTheme.fontFamily
           anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
            text: S.S0086
           color: appTheme.textPrimary
            font.pixelSize: appTheme.fontLarge
            font.bold: true
        }
    }

    Row {
        id: tabs
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        height: 38
       Repeater {
            model: [ S.S0088, S.S0089 ]
           delegate: Item {
                width: tabs.width / 2
                height: tabs.height
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width; height: 2
                    color: index === root.tab ? appTheme.accent : "transparent"
                }
                Text {
                    font.family: appTheme.fontFamily
                    anchors.centerIn: parent
                    text: modelData
                    color: index === root.tab ? appTheme.accent : appTheme.textSecondary
                    font.pixelSize: appTheme.fontMedium
                }
                MouseArea { anchors.fill: parent; onClicked: { root.tab = index; load() } }
            }
        }
    }

    ListView {
        id: list
        anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: items
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: list.width
            height: 72
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Row {
                anchors { fill: parent; leftMargin: 12; rightMargin: 12 }
                spacing: 10
                Avatar {
                    anchors.verticalCenter: parent.verticalCenter
                    size: 40
                    source: U.avatarUrl(modelData.replyerPortrait)
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: list.width - 130
                    spacing: 3
                   Text {
                       font.family: appTheme.fontFamily
                        text: (modelData.replyer ? modelData.replyer : "") + (root.tab === 0 ? S.S0090 : S.S0091)
                       color: appTheme.textPrimary
                        font.pixelSize: appTheme.fontSmall
                        elide: Text.ElideRight
                    }
                    Text {
                        font.family: appTheme.fontFamily
                        text: modelData.title ? modelData.title : ""
                        color: appTheme.textSecondary
                        font.pixelSize: appTheme.fontSmall
                        elide: Text.ElideRight
                    }
                    Text {
                        font.family: appTheme.fontFamily
                        text: util.timeAgo(parseInt(modelData.time, 10) * 1000)
                        color: appTheme.textTertiary
                        font.pixelSize: 11
                    }
                }
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (modelData.threadId)
                        pageStack.push(Qt.createComponent("ThreadPage.qml"),
                                      { tid: modelData.threadId, title: modelData.title })
                }
            }
        }
    }

    StatusView {
       anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
       visible: root.loadState !== 1
       status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
        emptyText: S.S0092
       errorText: root.errorText
        onRetry: load()
    }

    Connections {
        target: api
        onReplyMeReady: {
            if (root.tab !== 0) return
            if (error !== "") { root.loadState = 3; root.errorText = error }
            else { root.items = items; root.loadState = items.length > 0 ? 1 : 2 }
        }
        onAtMeReady: {
            if (root.tab !== 1) return
            if (error !== "") { root.loadState = 3; root.errorText = error }
            else { root.items = items; root.loadState = items.length > 0 ? 1 : 2 }
        }
    }

    function load() {
        root.loadState = 0
        root.items = []
        if (!accounts.loggedIn) { root.errorText = S.S0093; root.loadState = 3; return }
        if (root.tab === 0) api.loadReplyMe(0)
        else api.loadAtMe(0)
    }

    Component.onCompleted: load()
    onVisibleChanged: if (visible) load()
}
