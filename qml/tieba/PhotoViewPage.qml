import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

                                                     
Page {
    id: root
    property variant images: []                                  
    property int index: 0

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "download"; onClicked: saveCurrent() }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"

        ListView {
            id: viewer
            anchors.fill: parent
            orientation: ListView.Horizontal
            snapMode: ListView.SnapOneItem
            model: root.images
            delegate: Item {
                id: page
                width: viewer.width
                height: viewer.height

                property bool active: viewer.currentIndex === index
                property real curScale: 1.0
                property real offsetX: 0
                property real offsetY: 0
                property real startScale: 1.0
                property real startOffsetX: 0
                property real startOffsetY: 0

                // Leave the page -> drop any zoom so swiping back later is clean.
                onActiveChanged: {
                    if (!active) {
                        curScale = 1.0
                        offsetX = 0
                        offsetY = 0
                        photo.decodeBoost = 2
                    }
                    page.updateListInteraction()
                }
                onCurScaleChanged: page.updateListInteraction()

                // While zoomed the pager must not swallow drags: they pan the image.
                function updateListInteraction() {
                    if (page.active) viewer.interactive = page.curScale <= 1.0
                }

                function clampOffsets() {
                    var halfW = page.width * (curScale - 1) / 2
                    var halfH = page.height * (curScale - 1) / 2
                    if (offsetX > halfW) offsetX = halfW
                    else if (offsetX < -halfW) offsetX = -halfW
                    if (offsetY > halfH) offsetY = halfH
                    else if (offsetY < -halfH) offsetY = -halfH
                }

                CachedImage {
                    id: photo
                    anchors.centerIn: parent
                    width: page.width
                    height: page.height
                    fillMode: 1 // PreserveAspectFit
                    decodeBoost: 2 // decode at 2x so 2x zoom stays sharp
                    source: modelData.original ? modelData.original : (modelData.thumb ? modelData.thumb : "")
                    transform: [
                        Scale {
                            origin.x: page.width / 2
                            origin.y: page.height / 2
                            xScale: page.curScale
                            yScale: page.curScale
                        },
                        Translate { x: page.offsetX; y: page.offsetY }
                    ]
                }

                PinchArea {
                    anchors.fill: parent
                    enabled: page.active

                    onPinchStarted: {
                        page.startScale = page.curScale
                        page.startOffsetX = page.offsetX
                        page.startOffsetY = page.offsetY
                        viewer.interactive = false
                    }
                    onPinchUpdated: {
                        page.curScale = page.startScale * pinch.scale
                        if (page.curScale < 1.0) page.curScale = 1.0
                        else if (page.curScale > 4.0) page.curScale = 4.0
                        if (page.curScale > 1.0) {
                            page.offsetX = page.startOffsetX + (pinch.center.x - pinch.previousCenter.x)
                            page.offsetY = page.startOffsetY + (pinch.center.y - pinch.previousCenter.y)
                            page.clampOffsets()
                            // Progressive re-decode: past 2.2x fetch a larger
                            // bitmap so the picture stays readable (one-time cost).
                            if (page.curScale > 2.2 && photo.decodeBoost < 3)
                                photo.decodeBoost = 3
                        } else {
                            page.offsetX = 0
                            page.offsetY = 0
                        }
                    }
                    onPinchFinished: {
                        if (page.curScale <= 1.05) {
                            // snap back to identity
                            page.curScale = 1.0
                            page.offsetX = 0
                            page.offsetY = 0
                            photo.decodeBoost = 2
                        }
                        viewer.interactive = page.curScale <= 1.0
                    }
                }

                MouseArea {
                    id: panArea
                    anchors.fill: parent
                    enabled: page.active
                    property real lastX: 0
                    property real lastY: 0

                    // Single-finger pan while zoomed. At 1x this stays inert:
                    // the Flickable steals the drag and pages normally.
                    onPressed: {
                        lastX = mouse.x
                        lastY = mouse.y
                    }
                    onPositionChanged: {
                        if (pressed && page.curScale > 1.0) {
                            page.offsetX += mouse.x - lastX
                            page.offsetY += mouse.y - lastY
                            lastX = mouse.x
                            lastY = mouse.y
                            page.clampOffsets()
                        }
                    }

                    onDoubleClicked: {
                        if (page.curScale > 1.0) {
                            page.curScale = 1.0
                            page.offsetX = 0
                            page.offsetY = 0
                            photo.decodeBoost = 2
                        } else {
                            // Zoom to 2x keeping the tapped point under the finger:
                            // p = c + s*(u - c) + t  =>  t' = p - c - (s'/s)*(p - c - t)
                            var px = mouse.x, py = mouse.y
                            var cx = page.width / 2, cy = page.height / 2
                            var ratio = 2.0 / page.curScale
                            page.curScale = 2.0
                            page.offsetX = px - cx - ratio * (px - cx - page.offsetX)
                            page.offsetY = py - cy - ratio * (py - cy - page.offsetY)
                            page.clampOffsets()
                        }
                        viewer.interactive = page.curScale <= 1.0
                    }
                }
            }
            currentIndex: root.index
            Component.onCompleted: positionViewAtIndex(root.index, ListView.Beginning)
        }

        Text {
            font.family: appTheme.fontFamily
            anchors { right: parent.right; bottom: parent.bottom; rightMargin: 12; bottomMargin: 12 }
            text: (viewer.currentIndex + 1) + " / " + root.images.length
            color: "white"
            font.pixelSize: appTheme.fontMedium
        }
    }

    function saveCurrent() {
        var item = root.images[viewer.currentIndex]
        var url = item ? (item.original ? item.original : (item.thumb ? item.thumb : "")) : ""
        if (url === "") return
        var p = img.cachedPath(url)
        if (p !== "") util.saveImageToGallery(p)
        else { img.load(url); pendingSave = url }
    }

    property string pendingSave: ""

    Connections {
        target: img
        onLoaded: {
            if (url === root.pendingSave) {
                root.pendingSave = ""
                util.saveImageToGallery(path)
            }
        }
        onFailed: {
            if (url === root.pendingSave) {
                root.pendingSave = ""
                notifier.notify(S.S0094, S.S0095)
            }
        }
    }
}
