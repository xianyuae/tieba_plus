import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

                     
Page {
    id: root
    property string uid: ""

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
    }

    property int loadState: 0
    property variant user: ({})
   property variant posts: []
    property string errorText: S.S0039

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: posts
        spacing: appTheme.spacingMedium
        header: headerComponent
        delegate: Item {
            width: list.width
            height: content.height + 20
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Column {
                id: content
                anchors { left: parent.left; leftMargin: 12; right: parent.right; rightMargin: 12; top: parent.top; topMargin: 10 }
                spacing: 7
                Text {
                    font.family: appTheme.fontFamily
                    text: modelData.forumName + (modelData.title !== "" ? S.S0168 + modelData.title : "")
                    color: appTheme.textPrimary
                    font.pixelSize: appTheme.fontMedium
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }
                Text {
                    font.family: appTheme.fontFamily
                    visible: text !== ""
                    text: modelData.abstract
                    color: appTheme.textSecondary
                    font.pixelSize: appTheme.fontSmall
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                }
                ImageGrid { media: modelData.media ? modelData.media : [] }
                Row {
                    spacing: 10
                    Text { font.family: appTheme.fontFamily; text: util.timeAgo(modelData.createTime * 1000); color: appTheme.textTertiary; font.pixelSize: appTheme.fontSmall }
                    Text { font.family: appTheme.fontFamily; text: S.S0129 + (modelData.replyNum || 0); color: appTheme.textTertiary; font.pixelSize: appTheme.fontSmall }
                }
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: pageStack.push(Qt.createComponent("ThreadPage.qml"),
                                          { tid: modelData.tid, title: modelData.title, forumId: modelData.forumId, forumName: modelData.forumName })
            }
        }
    }

    Component {
        id: headerComponent
        Column {
            width: list.width
            Rectangle {
                width: parent.width
                height: 120
                color: appTheme.cardBackground
                Row {
                    anchors { fill: parent; leftMargin: 16; rightMargin: 16 }
                    spacing: 14
                    Avatar { anchors.verticalCenter: parent.verticalCenter; size: 72; source: root.user.avatar ? root.user.avatar : "" }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 5
                        Text {
                            font.family: appTheme.fontFamily
                            text: root.user.displayName ? root.user.displayName : ""
                            color: appTheme.textPrimary
                            font.pixelSize: appTheme.fontLarge
                            font.bold: true
                        }
                        Text {
                            font.family: appTheme.fontFamily
                           text: (root.user.intro ? root.user.intro : "") +
                                  (root.user.ip ? (S.S0033 + S.S0165 + root.user.ip) : "")
                           color: appTheme.textTertiary
                            font.pixelSize: appTheme.fontSmall
                            wrapMode: Text.WordWrap
                        }
                       Text {
                           font.family: appTheme.fontFamily
                            text: S.S0040 + (root.user.postNum || 0) + S.S0033 + S.S0166 + (root.user.tbAge || "")
                           color: appTheme.textSecondary
                            font.pixelSize: appTheme.fontSmall
                        }
                    }
                }
            }
            Rectangle { width: parent.width; height: 8; color: appTheme.background }
        }
    }

    StatusView {
        anchors.fill: parent
        visible: root.loadState !== 1
        status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
       errorText: root.errorText
        emptyText: S.S0167
       onRetry: load()
    }

    Connections {
        target: api
        onUserProfileReady: {
            if (error !== "") {
                root.loadState = 3
                root.errorText = error
            } else {
                root.user = profile.user ? profile.user : ({})
                root.posts = profile.posts ? profile.posts : []
                root.loadState = 1
            }
        }
    }

    function load() {
        root.loadState = 0
        api.loadUserProfile(root.uid)
    }

    Component.onCompleted: load()
}
