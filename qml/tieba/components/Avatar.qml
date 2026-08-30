import QtQuick 1.1
import "../strings.js" as S

                                   
Item {
    id: root
    property string source: ""
    property int size: 40

    width: size
    height: size

    // Square avatar with slightly rounded corners (cheap on Qt4 raster:
    // no circular opacity mask, plain clip is enough).
    Rectangle {
        anchors.fill: parent
        radius: appTheme.radius
        clip: true
        color: appTheme.dark ? "#2a2c30" : "#e4e6ea"

        CachedImage {
            anchors.fill: parent
            source: root.source
            fillMode: 2
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: appTheme.radius
        border.color: appTheme.divider
        border.width: 1
        color: "transparent"
    }
}
