import QtQuick 1.1
import "../util.js" as U
import "../strings.js" as S

                                                                                
Item {
    id: root
    property variant fragments: []
    signal imageClicked(variant images, int index)

    height: column.height
    clip: true

    Column {
        id: column
        anchors { left: parent.left; right: parent.right }
        spacing: 8

        Text {
            font.family: appTheme.fontFamily
            id: inlineText
            width: parent.width
            visible: text !== ""
            text: U.toHtml(root.fragments, appTheme.accent)
            textFormat: Text.StyledText
            color: appTheme.textPrimary
            font.pixelSize: appTheme.fontMedium
            wrapMode: Text.WordWrap
            onLinkActivated: util.openUrl(link)
        }

        Repeater {
            model: U.extractImages(root.fragments)
            delegate: Item {
                width: column.width
                height: image.height
                CachedImage {
                    id: image
                    anchors { left: parent.left; right: parent.right }
                    height: parent.width * U.imageAspect(modelData)
                    source: modelData.thumb
                    fillMode: 1
                    radius: appTheme.radius
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.imageClicked(U.extractImages(root.fragments), index)
                }
            }
        }

        Repeater {
            model: U.extractVideos(root.fragments)
            delegate: Item {
                width: column.width
                height: video.height + 0
                Rectangle {
                    id: video
                    anchors { left: parent.left; right: parent.right }
                    height: parent.width * U.imageAspect(modelData)
                    radius: appTheme.radius
                    color: appTheme.dark ? "#101114" : "#1a1a1a"
                    CachedImage {
                        anchors.fill: parent
                        source: modelData.src
                        fillMode: 2
                    }
                   Text {
                       font.family: appTheme.fontFamily
                       anchors.centerIn: parent
                        text: S.S0178
                       color: "white"
                        font.pixelSize: 40
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (modelData.link !== "")
                                util.openUrl(modelData.link)
                            else if (modelData.text !== "")
                                util.openUrl(modelData.text)
                        }
                    }
                }
            }
        }

        Repeater {
            model: U.extractVoices(root.fragments)
            delegate: Rectangle {
                width: column.width
                height: 44
                radius: appTheme.radius
                color: appTheme.cardBackground
                border.color: appTheme.divider
                border.width: 1
                Row {
                    anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                   spacing: 8
                    Text { font.family: appTheme.fontFamily; text: S.S0179; font.pixelSize: appTheme.fontMedium; anchors.verticalCenter: parent.verticalCenter }
                   Text {
                       font.family: appTheme.fontFamily
                        text: S.S0180 + (modelData.duringTime ? modelData.duringTime + "s" : "")
                       color: appTheme.textPrimary
                        font.pixelSize: appTheme.fontSmall
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }
    }
}
