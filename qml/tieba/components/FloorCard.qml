import QtQuick 1.1
import com.nokia.meego 1.0
import "../strings.js" as S

// Post ("floor") card: full-bleed flat block (no gaps between floors) with
// author header, rich content, sub-post preview and an icon action row.
Item {
    id: root
    property variant post: ({})
    property string threadAuthorId: ""
    property string tid: ""
    // Optimistic-agree patch from the page: {has: 0/1, count: int} or undefined.
    property variant agreeOverride: undefined

    signal replyClicked()
    signal authorClicked()
    signal agreeClicked()
    signal storeClicked()
    signal subFloorClicked()
    signal imageClicked(variant images, int index)

    width: parent.width
    height: card.height

    Rectangle {
        id: card
        anchors { left: parent.left; right: parent.right; top: parent.top }
        height: content.height + 22
        color: appTheme.cardBackground
    }

    // Author tap target: declared before content so the action-row and
    // sub-post MouseAreas stack above it. Anchors to sibling `card` only.
    MouseArea {
        anchors.fill: card
        onClicked: root.authorClicked()
    }

    Column {
        id: content
        anchors {
            left: parent.left; leftMargin: 14
            right: parent.right; rightMargin: 14
            top: card.top; topMargin: 11
        }
        spacing: 8

        Row {
            id: headerRow
            width: parent.width
            spacing: 10
            Avatar { source: post.author ? post.author.avatar : ""; size: 40 }
            Column {
                width: parent.width - 50
                spacing: 3
                Row {
                    spacing: 6
                    Text {
                        font.family: appTheme.fontFamily
                        text: post.author ? post.author.displayName : ""
                        color: appTheme.textPrimary
                        font.pixelSize: appTheme.fontMedium
                        font.bold: true
                    }
                    Rectangle {
                       visible: !!post.author && post.author.isBawu === 1
                       width: 34; height: 16; radius: 3; color: appTheme.accent
                        Text { font.family: appTheme.fontFamily; anchors.centerIn: parent; text: S.S0169; color: "white"; font.pixelSize: 10 }
                   }
                    Rectangle {
                       visible: !!post.author && threadAuthorId !== "" && post.author.id === threadAuthorId
                       width: 34; height: 16; radius: 3; color: "#ff9500"
                        Text { font.family: appTheme.fontFamily; anchors.centerIn: parent; text: S.S0170; color: "white"; font.pixelSize: 10 }
                   }
                }
                Text {
                    font.family: appTheme.fontFamily
                   text: {
                       var s = util.timeAgo(post.time * 1000)
                        if (post.floor > 1) s += S.S0033 + S.S0171 + post.floor + S.S0172
                       if (post.author && post.author.ip) s += S.S0033 + post.author.ip
                        return s
                    }
                    color: appTheme.textTertiary
                    font.pixelSize: appTheme.fontSmall
                }
            }
        }

        RichContent {
            anchors { left: parent.left; right: parent.right }
            fragments: post.content ? post.content : []
            onImageClicked: root.imageClicked(images, index)
        }

        Rectangle {
            visible: !!post.originThread && post.originThread.title !== undefined && post.originThread.title !== ""
            width: parent.width
            height: 60
            radius: appTheme.radius
            color: appTheme.dark ? "#1f2125" : "#f5f6f8"
            Column {
                anchors { fill: parent; margins: 10 }
                spacing: 4
                Text {
                    font.family: appTheme.fontFamily
                    width: parent.width
                    text: post.originThread ? post.originThread.title : ""
                    color: appTheme.textPrimary
                    font.pixelSize: appTheme.fontSmall
                    elide: Text.ElideRight
                }
                Text {
                    font.family: appTheme.fontFamily
                    width: parent.width
                    text: post.originThread ? (post.originThread.abstract + S.S0033 + post.originThread.fname) : ""
                    color: appTheme.textTertiary
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (post.originThread && post.originThread.tid)
                        pageStack.push(Qt.createComponent("../ThreadPage.qml"), { tid: post.originThread.tid, title: post.originThread.title })
                }
            }
        }

        Column {
            id: subPreview
            visible: subModel.length > 0
            width: parent.width
            spacing: 6
            Rectangle { width: parent.width; height: 1; color: appTheme.divider }
            Repeater {
                model: subModel
                delegate: Text {
                    font.family: appTheme.fontFamily
                    width: subPreview.width
                    text: {
                        var name = modelData.author ? modelData.author.displayName : ""
                        var txt = ""
                        for (var i = 0; i < modelData.content.length; i++) {
                            var c = modelData.content[i]
                            if (c.type === 0 || c.type === 9 || c.type === 27) txt += c.text
                            else if (c.type === 2) txt += "[" + c.text + "]"
                        }
                        return name + S.S0177 + txt
                    }
                    color: appTheme.textSecondary
                    font.pixelSize: appTheme.fontSmall
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    MouseArea { anchors.fill: parent; onClicked: root.subFloorClicked() }
                }
            }
           Text {
               font.family: appTheme.fontFamily
               visible: post.subPostNumber > subModel.length
                text: S.S0173 + post.subPostNumber + S.S0174
               color: appTheme.accent
                font.pixelSize: appTheme.fontSmall
                MouseArea { anchors.fill: parent; onClicked: root.subFloorClicked() }
            }
        }

        Rectangle { width: parent.width; height: 1; color: appTheme.divider }

        Row {
            width: parent.width
            height: 28
            spacing: 24

            Item {
                width: actionAgree.width + 20; height: parent.height
                MouseArea { anchors.fill: parent; onClicked: root.agreeClicked() }
                Row {
                    id: actionAgree
                    spacing: 5
                    anchors.verticalCenter: parent.verticalCenter
                    SvgIcon {
                        name: "agree"
                        width: 15; height: 15
                        dark: root.agreedNow ? !appTheme.dark : appTheme.dark
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        font.family: appTheme.fontFamily
                        text: agreeText
                        color: root.agreedNow ? appTheme.accent : appTheme.textSecondary
                        font.pixelSize: appTheme.fontSmall
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            Item {
                width: actionReply.width + 20; height: parent.height
                MouseArea { anchors.fill: parent; onClicked: root.replyClicked() }
                Row {
                    id: actionReply
                    spacing: 5
                    anchors.verticalCenter: parent.verticalCenter
                    SvgIcon { name: "reply"; width: 15; height: 15; anchors.verticalCenter: parent.verticalCenter }
                    Text {
                        font.family: appTheme.fontFamily
                        text: S.S0088
                        color: appTheme.textSecondary
                        font.pixelSize: appTheme.fontSmall
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            Item {
                width: actionStore.width + 20; height: parent.height
                MouseArea { anchors.fill: parent; onClicked: root.storeClicked() }
                Row {
                    id: actionStore
                    spacing: 5
                    anchors.verticalCenter: parent.verticalCenter
                    SvgIcon { name: "bookmark"; width: 15; height: 15; anchors.verticalCenter: parent.verticalCenter }
                    Text {
                        font.family: appTheme.fontFamily
                        text: S.S0140
                        color: appTheme.textSecondary
                        font.pixelSize: appTheme.fontSmall
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }

   property bool agreedNow: agreeOverride !== undefined
                             ? agreeOverride.has === 1
                             : (!!post.agree && post.agree.hasAgree === 1)
    property int baseAgreeCount: post.agree ? (post.agree.diffAgreeNum || post.agree.agreeNum || 0) : 0
    property int agreeCount: agreeOverride !== undefined ? agreeOverride.count : baseAgreeCount
    property string agreeText: agreeCount > 0 ? (S.S0175 + agreeCount) : S.S0176

    property variant subModel: post.subPostList ? post.subPostList.slice(0, 2) : []
}
