import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

        
Page {
    id: root

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
    }

    property variant favorites: []
    property int loadState: 0
    property string errorText: S.S0039
    property string pendingTid: ""

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: favorites
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: list.width
            height: 64
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Column {
                anchors { left: parent.left; leftMargin: 12; right: parent.right; rightMargin: 56; top: parent.top; topMargin: 10 }
                spacing: 4
                Text {
                    font.family: appTheme.fontFamily
                    width: parent.width
                    text: modelData.title
                    color: appTheme.textPrimary
                    font.pixelSize: appTheme.fontMedium
                    elide: Text.ElideRight
                }
                Text {
                    font.family: appTheme.fontFamily
                    text: (modelData.forum_name !== "" ? modelData.forum_name + S.S0033 : "") + util.timeAgo(modelData.updated_at)
                    color: appTheme.textTertiary
                    font.pixelSize: appTheme.fontSmall
                }
            }
            Text {
                font.family: appTheme.fontFamily
                anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                text: root.pendingTid === modelData.tid ? S.S0124 : S.S0023
                color: appTheme.accent
                font.pixelSize: appTheme.fontSmall
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: pageStack.push(Qt.createComponent("ThreadPage.qml"),
                                          { tid: modelData.tid, title: modelData.title,
                                            forumId: modelData.forum_id, forumName: modelData.forum_name })
            }
            MouseArea {
                anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
                width: 70
                enabled: root.pendingTid === ""
                onClicked: {
                    root.pendingTid = modelData.tid
                    api.removeStore(modelData.tid)
                }
            }
        }
    }

    StatusView {
        anchors.fill: parent
        visible: root.loadState !== 1
       status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
        emptyText: S.S0125
       errorText: root.errorText
        onRetry: load()
    }

    Connections {
        target: api
        onStoreListReady: {
            if (error !== "") {
                root.favorites = db.favorites()
                if (root.favorites.length > 0) {
                    root.loadState = 1
                    notifier.notify(S.S0126, S.S0127)
                } else {
                    root.errorText = error
                    root.loadState = 3
                }
                return
            }
            var remote = ({})
            for (var i = 0; i < items.length; i++) {
                remote[items[i].tid] = true
                db.addFavorite({ tid: items[i].tid, title: items[i].title,
                                 author: items[i].author, forum_name: items[i].forumName })
            }
            var local = db.favorites()
            for (var j = 0; j < local.length; j++) {
                if (!remote[local[j].tid]) db.removeFavorite(local[j].tid)
            }
            root.favorites = db.favorites()
            root.loadState = root.favorites.length > 0 ? 1 : 2
        }
        onActionFinished: {
            if (action !== "store" || data.tid !== root.pendingTid) return
            if (ok) {
                db.removeFavorite(root.pendingTid)
                root.favorites = db.favorites()
                root.loadState = root.favorites.length > 0 ? 1 : 2
            } else {
                notifier.notify(S.S0128, message)
            }
            root.pendingTid = ""
        }
    }

    function load() {
        root.loadState = 0
        root.favorites = []
        if (accounts.loggedIn) api.loadStoreList()
        else {
            root.favorites = db.favorites()
            root.loadState = root.favorites.length > 0 ? 1 : 2
        }
    }

    Component.onCompleted: load()
}
