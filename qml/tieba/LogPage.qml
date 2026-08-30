import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

                 
Page {
    id: root

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "refresh"; onClicked: refresh() }
        ToolButton { icon: "close"; onClicked: { logstore.clear(); refresh() } }
    }

    property variant lines: []

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: lines
        delegate: Item {
            width: list.width
            height: col.height + 12
            Column {
                id: col
                anchors { left: parent.left; leftMargin: 10; right: parent.right; rightMargin: 10; top: parent.top; topMargin: 6 }
                spacing: 2
                Text {
                    font.family: appTheme.fontFamily
                    width: parent.width
                    text: modelData[0] + "  " + modelData[1] + "[" + modelData[2] + "]"
                    color: modelData[1] === "http" ? appTheme.accent : appTheme.textTertiary
                    font.pixelSize: 11
                }
                Text {
                    font.family: appTheme.fontFamily
                    width: parent.width
                    text: modelData[3]
                    color: appTheme.textSecondary
                    font.pixelSize: appTheme.fontSmall
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    StatusView {
        anchors.fill: parent
       visible: root.lines.length === 0
       status: 1
        emptyText: S.S0066
   }

    function refresh() {
        root.lines = logstore.read()
    }

    Component.onCompleted: refresh()
}
