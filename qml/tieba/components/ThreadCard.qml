import QtQuick 1.1
import "../util.js" as U
import "../strings.js" as S

// Thread list card: rounded card, accent edge for pinned/quality threads,
// meta row with reply-count chip.
Item {
    id: root
    property variant thread: ({})
    signal clicked()

    width: parent.width
    height: card.height

    Rectangle {
        id: card
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 4 }
        height: content.height + 24
        radius: appTheme.radius
        color: appTheme.cardBackground

        Rectangle {
            visible: thread.isTop === 1
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
            width: 3
            radius: 2
            color: appTheme.badge
        }
    }

    Column {
        id: content
        anchors {
            left: parent.left; leftMargin: 16
            right: parent.right; rightMargin: 16
            top: card.top; topMargin: 12
        }
        spacing: 7

        Text {
            font.family: appTheme.fontFamily
            width: parent.width
            text: (thread.isTop === 1 ? "[" + S.S0182 + "] " : "") +
                  (thread.isGood === 1 ? "[" + S.S0183 + "] " : "") +
                  (thread.title ? thread.title : "")
            color: thread.isTop === 1 ? appTheme.badge : appTheme.textPrimary
            font.pixelSize: appTheme.fontMedium
            font.bold: true
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Text {
            font.family: appTheme.fontFamily
            visible: text !== ""
            width: parent.width
            text: thread.abstract ? thread.abstract : ""
            color: appTheme.textSecondary
            font.pixelSize: appTheme.fontSmall
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        ImageGrid {
            media: thread.media ? thread.media : []
        }

        Row {
            width: parent.width
            spacing: 12

            Text {
                font.family: appTheme.fontFamily
                text: thread.forumName ? thread.forumName : ""
                color: appTheme.accent
                font.pixelSize: appTheme.fontSmall
                elide: Text.ElideRight
                width: Math.min(implicitWidth + 2, parent.width / 2)
            }

            Row {
                spacing: 4
                SvgIcon { name: "reply"; width: 13; height: 13; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    font.family: appTheme.fontFamily
                    text: thread.replyNum !== undefined ? String(thread.replyNum) : "0"
                    color: appTheme.textTertiary
                    font.pixelSize: appTheme.fontSmall
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Text {
                font.family: appTheme.fontFamily
                text: util.timeAgo(U.threadTimeSeconds(thread) * 1000)
                color: appTheme.textTertiary
                font.pixelSize: appTheme.fontSmall
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    MouseArea {
        anchors.fill: card
        onClicked: root.clicked()
    }
}
