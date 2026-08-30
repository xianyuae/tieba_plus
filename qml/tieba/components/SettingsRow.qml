import QtQuick 1.1

// Standard settings row: optional leading icon, title + optional subtitle,
// and a right slot (Switch, value text, ...). Set clickable: true together
// with onClicked to get press feedback; the MouseArea sits below the right
// slot so embedded controls keep their own input.
Item {
    id: root

    property string iconName: ""
    property string title: ""
    property string subtitle: ""
    property bool showDivider: false
    property bool clickable: false
    default property alias content: rightSlot.data
    signal clicked()

    width: parent ? parent.width : 0
    height: Math.max(56, txtCol.height + 18)

    SvgIcon {
        id: iconImg
        visible: root.iconName !== ""
        name: root.iconName
        width: 22
        height: 22
        anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
    }

    Rectangle {
        anchors.fill: parent
        color: appTheme.divider
        opacity: root.clickable && rowMouse.pressed ? 0.6 : 0
        visible: opacity > 0
    }

    Column {
        id: txtCol
        spacing: 2
        anchors {
            left: iconImg.visible ? iconImg.right : parent.left
            leftMargin: iconImg.visible ? 14 : 16
            right: rightSlot.left
            rightMargin: 12
            verticalCenter: parent.verticalCenter
        }

        Text {
            font.family: appTheme.fontFamily
            width: parent.width
            text: root.title
            color: appTheme.textPrimary
            font.pixelSize: appTheme.fontMedium
            elide: Text.ElideRight
        }
        Text {
            font.family: appTheme.fontFamily
            width: parent.width
            visible: root.subtitle !== ""
            text: root.subtitle
            color: appTheme.textTertiary
            font.pixelSize: appTheme.fontSmall
            wrapMode: Text.WordWrap
        }
    }

    Row {
        id: rightSlot
        spacing: 8
        anchors { right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
    }

    Rectangle {
        visible: root.showDivider
        anchors { left: parent.left; leftMargin: iconImg.visible ? 52 : 16; right: parent.right; bottom: parent.bottom }
        height: 1
        color: appTheme.divider
    }

    MouseArea {
        id: rowMouse
        anchors.fill: parent
        enabled: root.clickable
        onClicked: root.clicked()
    }
}
