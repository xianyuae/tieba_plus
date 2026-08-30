import QtQuick 1.1
import "../strings.js" as S

                                                                     
Item {
    id: root
    property variant media: []                        
    property int thumbSize: 90

    height: media.length > 0 ? thumbSize : 0
    clip: true

    Row {
        id: row
        spacing: 4
        Repeater {
            model: Math.min(root.media.length, 3)
            delegate: CachedImage {
                width: root.thumbSize
                height: root.thumbSize
                source: root.media[index].thumb
                fillMode: 2
                radius: appTheme.radius
            }
        }
    }

    Rectangle {
        visible: root.media.length > 3
        anchors { right: row.right; bottom: row.bottom }
        width: 40; height: 40
        radius: 20
        color: "#88000000"
        Text {
            font.family: appTheme.fontFamily
            anchors.centerIn: parent
            text: "+" + (root.media.length - 3)
            color: "white"
            font.pixelSize: appTheme.fontSmall
        }
    }
}
