import QtQuick 1.1

// Card container for settings/about sections. Children are laid out in a
// full-width Column; rows inside set width: parent.width.
Rectangle {
    id: root

    default property alias content: col.data

    width: parent ? parent.width : 0
    height: col.height + 12
    radius: appTheme.radius
    color: appTheme.cardBackground
    clip: true

    Column {
        id: col
        anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 6 }
        spacing: 0
    }
}
