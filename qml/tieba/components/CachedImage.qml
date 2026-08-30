import QtQuick 1.1
import "../strings.js" as S

                                                                                
Item {
    id: root
    property string source: ""
    property int fillMode: 2                                       
    property color placeholderColor: appTheme.dark ? "#2a2c30" : "#e4e6ea"
    property bool rounded: false
    property int radius: 0
    property bool loadFailed: false
    property int decodeBoost: 1 // >1 decodes a larger bitmap (sharper pinch zoom)

    Rectangle {
        anchors.fill: parent
        color: placeholderColor
        visible: image.status !== Image.Ready
        radius: root.rounded ? root.radius : 0
        clip: true
    }

    Image {
        id: image
        anchors.fill: parent
        fillMode: root.fillMode === 0 ? Image.Stretch :
                  (root.fillMode === 2 ? Image.PreserveAspectCrop : Image.PreserveAspectFit)
        smooth: true
        cache: true
        clip: root.rounded
        sourceSize.width: root.width > 0 ? Math.ceil(root.width * root.decodeBoost) : 0
    }

    function reload() {
        root.loadFailed = false
        if (root.source === "") { image.source = ""; return }
        var p = img.cachedPath(root.source)
        if (p !== "")
            image.source = img.fileUrl(p)
        else
            img.load(root.source)
    }

    onSourceChanged: reload()
    Component.onCompleted: reload()

    Connections {
        target: img
        onLoaded: {
            if (url === root.source) {
                root.loadFailed = false
                image.source = img.fileUrl(path)
            }
        }
        onFailed: if (url === root.source) root.loadFailed = true
    }
}
