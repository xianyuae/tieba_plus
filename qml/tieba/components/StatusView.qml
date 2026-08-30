import QtQuick 1.1
import com.nokia.meego 1.0
import "../strings.js" as S

// Loading / empty / error placeholder with themed icon and retry action.
// status: 0 = loading, 1 = content (hides this view), 2 = empty, 3 = error.
Item {
    id: root
    property int status: 0
    property string emptyText: S.S0167
    property string errorText: S.S0039
    signal retry()

    anchors.fill: parent
    visible: status !== 1

    BusyIndicator {
        anchors.centerIn: parent
        running: root.status === 0
        visible: root.status === 0
    }

    Column {
        anchors.centerIn: parent
        visible: root.status !== 0
        spacing: 14
        width: parent.width - 60

        SvgIcon {
            anchors.horizontalCenter: parent.horizontalCenter
            name: root.status === 3 ? "error" : "empty"
            width: 44
            height: 44
        }

        Text {
            font.family: appTheme.fontFamily
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            color: appTheme.textSecondary
            font.pixelSize: appTheme.fontMedium
            wrapMode: Text.WordWrap
            text: root.status === 3 ? root.errorText : root.emptyText
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.status === 3
            text: S.S0181
            font.family: appTheme.fontFamily
            onClicked: root.retry()
        }
    }
}
