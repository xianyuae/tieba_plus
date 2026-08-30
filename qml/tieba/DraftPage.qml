import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

       
Page {
    id: root

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
    }

    property variant drafts: []

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: drafts
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: list.width
            height: 74
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Column {
                anchors { left: parent.left; leftMargin: 12; right: parent.right; rightMargin: 56; top: parent.top; topMargin: 10 }
                spacing: 4
                Text {
                    font.family: appTheme.fontFamily
                   width: parent.width
                    text: modelData.content !== "" ? modelData.content : S.S0030
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
                text: S.S0034
                color: appTheme.textTertiary
                font.pixelSize: appTheme.fontMedium
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    replySheet.kw = modelData.forum_name
                    replySheet.fid = modelData.forum_id
                    replySheet.tid = modelData.thread_id
                    replySheet.postId = modelData.floor
                    replySheet.initialText = modelData.content
                    replySheet.initialImages = modelData.images !== "" ? modelData.images.split(",") : []
                    replySheet.draftId = modelData.id
                    replySheet.isNewThread = modelData.thread_id === ""
                    replySheet.open()
                }
            }
            MouseArea {
                anchors { right: parent.right; top: parent.top; bottom: parent.bottom; rightMargin: 0 }
                width: 60
                onClicked: { db.removeDraft(modelData.id); root.drafts = db.drafts() }
            }
        }
    }

    StatusView {
        anchors.fill: parent
       visible: root.drafts.length === 0
       status: 1
        emptyText: S.S0031
   }

   ReplySheet {
       id: replySheet
        title: S.S0032
   }

    Component.onCompleted: root.drafts = db.drafts()
}
