import QtQuick 1.1

// Horizontal chip selector (single choice). Replaces rows of Meego Buttons:
// cheaper to lay out, accent-tinted selection, scales with the font setting.
Row {
    id: root

    property variant items: []
    property int selectedIndex: 0
    signal selected(int index)

    spacing: 8

    Repeater {
        model: root.items

        delegate: Rectangle {
            property bool active: index === root.selectedIndex

            width: chipLabel.width + 30
            height: 36
            radius: 18
            color: active ? appTheme.accent : appTheme.cardBackground
            border.color: active ? appTheme.accentPressed : appTheme.divider
            border.width: 1

            Text {
                id: chipLabel
                anchors.centerIn: parent
                font.family: appTheme.fontFamily
                text: modelData
                color: parent.active ? "white" : appTheme.textSecondary
                font.pixelSize: appTheme.fontSmall
                font.bold: parent.active
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (!parent.active) {
                        root.selectedIndex = index
                        root.selected(index)
                    }
                }
            }
        }
    }
}
