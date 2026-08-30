import QtQuick 1.1

// Small section label shown above a SettingsCard.
Item {
    id: root
    property string title: ""

    width: parent ? parent.width : 0
    height: 30

    Text {
        font.family: appTheme.fontFamily
        anchors { left: parent.left; leftMargin: 18; bottom: parent.bottom; bottomMargin: 4 }
        text: root.title
        color: appTheme.textTertiary
        font.pixelSize: appTheme.fontSmall
        font.bold: true
    }
}
