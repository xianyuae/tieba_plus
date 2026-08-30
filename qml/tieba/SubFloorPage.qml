import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

       
Page {
    id: root
    property string tid: ""
    property string pid: ""
    property string forumId: ""
    property string forumName: ""

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "edit"; onClicked: replyToParent() }
    }

   property int loadState: 0
    property string errorText: S.S0039
   property variant subposts: []
    property variant parentPost: ({})
    property bool offlineNotified: false
    property int currentPage: 1
    property int requestedPage: 1
    property bool hasMore: false
    property bool loadingMore: false

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: subposts
        spacing: appTheme.spacingMedium
        // Reassigning a JS-array model resets the view to the top; remember
        // the offset across a load-more append and restore it when it lands.
        property real pendingY: -1
        onCountChanged: if (pendingY >= 0) { contentY = pendingY; pendingY = -1 }
        delegate: Item {
            width: list.width
            height: content.height + 20
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Column {
                id: content
                anchors { left: parent.left; leftMargin: 12; right: parent.right; rightMargin: 12; top: parent.top; topMargin: 10 }
                spacing: 6
                Row {
                    spacing: 8
                    Avatar { source: modelData.author ? modelData.author.avatar : ""; size: 32 }
                    Column {
                        width: list.width - 120
                        spacing: 2
                        Text {
                            font.family: appTheme.fontFamily
                            text: modelData.author ? modelData.author.displayName : ""
                            color: appTheme.textPrimary
                            font.pixelSize: appTheme.fontSmall
                            font.bold: true
                        }
                        Text {
                            font.family: appTheme.fontFamily
                            text: util.timeAgo(modelData.time * 1000)
                            color: appTheme.textTertiary
                            font.pixelSize: 11
                        }
                    }
                    Text {
                        font.family: appTheme.fontFamily
                       anchors.verticalCenter: parent.verticalCenter
                        text: S.S0088
                       color: appTheme.accent
                        font.pixelSize: appTheme.fontSmall
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                replySheet.subPostId = modelData.id
                               replySheet.replyUid = modelData.author ? modelData.author.id : ""
                                replySheet.title = S.S0129 + (modelData.author ? modelData.author.displayName : "")
                               replySheet.open()
                            }
                        }
                    }
                }
                RichContent {
                    anchors { left: parent.left; leftMargin: 40; right: parent.right }
                    fragments: modelData.content ? modelData.content : []
                }
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
        }
        footer: Item {
            width: list.width
            height: root.hasMore || root.loadingMore ? 48 : 0
            Button {
                anchors.centerIn: parent
                visible: root.hasMore && !root.loadingMore
                text: S.S0044
                font.family: appTheme.fontFamily
                onClicked: loadMore()
            }
            BusyIndicator {
                anchors.centerIn: parent
                visible: root.loadingMore
                running: root.loadingMore
            }
        }
    }

    StatusView {
        anchors.fill: parent
        visible: root.loadState !== 1
        status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
       errorText: root.errorText
        emptyText: S.S0130
       onRetry: load()
    }

    ReplySheet {
        id: replySheet
        fid: root.forumId
        kw: root.forumName
        tid: root.tid
        postId: root.pid
        isNewThread: false
    }

    Connections {
        target: api
        onActionFinished: {
            if (action === "addPost" && ok && pageStack.currentPage === root)
                root.load()
        }
        onSubFloorReady: {
            if (error !== "") {
               if (root.loadingMore) {
                   root.loadingMore = false
                    notifier.notify(S.S0131, error)
               } else {
                    root.loadState = 3
                    root.errorText = error
                }
            } else {
                if (data.offline === 1 && !root.offlineNotified) {
                   root.offlineNotified = true
                    notifier.notify(S.S0050, S.S0132)
               }
                var incoming = data.subposts ? data.subposts : []
                // Arm the scroll restore before the append (model reset snaps to top).
                if (root.requestedPage > 1) list.pendingY = list.contentY
                root.subposts = root.requestedPage === 1 ? incoming : root.subposts.concat(incoming)
                root.parentPost = data.post ? data.post : root.parentPost
                if (root.forumName === "" && data.thread && data.thread.forumName)
                    root.forumName = data.thread.forumName
                var page = data.page ? data.page : ({})
                root.currentPage = page.currentPage ? page.currentPage : root.requestedPage
                root.hasMore = page.hasMore === 1
                root.loadingMore = false
                root.loadState = root.subposts.length > 0 ? 1 : 2
            }
        }
    }

    function load() {
        root.loadState = 0
        root.subposts = []
        root.parentPost = ({})
        root.offlineNotified = false
        root.currentPage = 1
        root.requestedPage = 1
        root.hasMore = false
        root.loadingMore = false
        api.loadSubFloor(root.tid, root.pid, 1, root.forumId)
    }

    function loadMore() {
        if (root.loadingMore || !root.hasMore) return
        root.loadingMore = true
        root.requestedPage = root.currentPage + 1
        api.loadSubFloor(root.tid, root.pid, root.requestedPage, root.forumId)
    }

    function replyToParent() {
        replySheet.subPostId = ""
        replySheet.replyUid = root.parentPost.author ? root.parentPost.author.id : ""
        replySheet.title = S.S0133
        replySheet.open()
    }

    Component.onCompleted: load()
}
