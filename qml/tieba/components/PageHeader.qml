import QtQuick 1.1

// Unified page header: accent-tinted title bar with an optional subtitle.
// Pages stack their content below it.
Rectangle {
    id: root
    property string title: ""
    property string subtitle: ""
    default property alias content: rightSlot.data

    height: Math.max(56, headerCol.height + 20)
    color: appTheme.cardBackground

    Rectangle {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 2
        color: appTheme.accent
    }

    Column {
        id: headerCol
        anchors {
            left: parent.left; leftMargin: 16
            right: rightSlot.left; rightMargin: 8
            verticalCenter: parent.verticalCenter
        }
        spacing: 2

        Text {
            font.family: appTheme.fontFamily
            width: parent.width
            text: root.title
            color: appTheme.textPrimary
            font.pixelSize: appTheme.fontLarge
            font.bold: true
            elide: Text.ElideRight
        }

        Text {
            font.family: appTheme.fontFamily
            width: parent.width
            visible: root.subtitle !== ""
            text: root.subtitle
            color: appTheme.textTertiary
            font.pixelSize: appTheme.fontSmall
            elide: Text.ElideRight
        }
    }

    Row {
        id: rightSlot
        anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
        spacing: 8
    }
}
