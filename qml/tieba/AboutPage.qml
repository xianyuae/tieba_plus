import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

Page {
    id: root

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
    }

    Rectangle { anchors.fill: parent; color: appTheme.background }

    PageHeader { id: header; title: S.S0084 }

    Flickable {
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        contentWidth: parent.width
        contentHeight: contentCol.height + 24
        clip: true

        Column {
            id: contentCol
            anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 12 }
            spacing: 6

            Item {
                width: parent.width
                height: 126

                Rectangle {
                    anchors { left: parent.left; leftMargin: 22; verticalCenter: parent.verticalCenter }
                    width: 80
                    height: 80
                    radius: 18
                    color: appTheme.accent
                    Text {
                        anchors.centerIn: parent
                        text: "贴"
                        color: "white"
                        font.family: appTheme.fontFamily
                        font.pixelSize: 42
                        font.bold: true
                    }
                }

                Column {
                    anchors { left: parent.left; leftMargin: 122; right: parent.right; rightMargin: 18; verticalCenter: parent.verticalCenter }
                    spacing: 5
                    Text {
                        font.family: appTheme.fontFamily
                        text: "百度贴吧+"
                        color: appTheme.textPrimary
                        font.pixelSize: appTheme.fontLarge * 1.35
                        font.bold: true
                    }
                    Text {
                        width: parent.width
                        font.family: appTheme.fontFamily
                        text: S.S0014
                        color: appTheme.textTertiary
                        font.pixelSize: appTheme.fontSmall
                        wrapMode: Text.WordWrap
                    }
                }
            }

            SettingsSectionLabel { title: S.S0202 }
            SettingsCard {
                Column {
                    width: parent.width
                    SettingsRow {
                        iconName: "info"
                        title: S.S0203
                        subtitle: S.S0204
                        showDivider: true
                    }
                    SettingsRow {
                        iconName: "history"
                        title: S.S0003
                        clickable: true
                        onClicked: pageStack.push(Qt.createComponent("LogPage.qml"), {})
                    }
                }
            }
            Item { width: 1; height: 10 }

            SettingsSectionLabel { title: S.S0211 }
            Rectangle {
                width: parent.width
                height: aiNoticeCol.height + 20
                radius: appTheme.radius
                color: "#fff3b0"
                clip: true

                Column {
                    id: aiNoticeCol
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }
                    spacing: 4

                    Text {
                        width: parent.width
                        text: S.S0211
                        color: "#6b4f00"
                        font.family: appTheme.fontFamily
                        font.pixelSize: appTheme.fontMedium
                        font.bold: true
                    }
                    Text {
                        width: parent.width
                        text: S.S0212
                        color: "#6b4f00"
                        font.family: appTheme.fontFamily
                        font.pixelSize: appTheme.fontSmall
                        wrapMode: Text.WordWrap
                    }
                }
            }
            Item { width: 1; height: 10 }

            SettingsSectionLabel { title: S.S0004 }
            SettingsCard {
                Item {
                    width: parent.width
                    height: bodyText.height + 20
                    Text {
                        id: bodyText
                        font.family: appTheme.fontFamily
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }
                        text: S.S0005
                        color: appTheme.textSecondary
                        font.pixelSize: appTheme.fontSmall
                        wrapMode: Text.WordWrap
                    }
                }
            }
            Item { width: 1; height: 8 }

            SettingsSectionLabel { title: S.S0006 }
            SettingsCard {
                Item {
                    width: parent.width
                    height: thanksText.height + 20
                    Text {
                        id: thanksText
                        font.family: appTheme.fontFamily
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 10 }
                        text: S.S0007
                        color: appTheme.textSecondary
                        font.pixelSize: appTheme.fontSmall
                        wrapMode: Text.WordWrap
                    }
                }
            }
            Item { width: 1; height: 8 }

            Text {
                width: parent.width - 36
                x: 18
                text: S.S0008
                color: appTheme.textTertiary
                font.family: appTheme.fontFamily
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
        }
    }

}
