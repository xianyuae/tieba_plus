import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

        
Page {
    id: root

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "close"; onClicked: { db.clearHistory(); root.history = db.history() } }
    }

    property variant history: []

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: history
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: list.width
            height: 58
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Column {
                anchors { left: parent.left; leftMargin: 12; right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
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
                    text: util.timeAgo(modelData.updated_at)
                    color: appTheme.textTertiary
                    font.pixelSize: appTheme.fontSmall
                }
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: pageStack.push(Qt.createComponent("ThreadPage.qml"),
                                          { tid: modelData.tid, title: modelData.title,
                                            forumId: modelData.forum_id, forumName: modelData.forum_name })
            }
        }
    }

    StatusView {
        anchors.fill: parent
       visible: root.history.length === 0
       status: 1
        emptyText: S.S0052
   }

    Component.onCompleted: root.history = db.history()
}
