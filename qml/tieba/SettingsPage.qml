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

    PageHeader { id: header; title: S.S0083 }

    Flickable {
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        contentWidth: parent.width
        contentHeight: contentCol.height + 20

        Column {
            id: contentCol
            anchors { left: parent.left; right: parent.right; top: parent.top; topMargin: 8 }
            spacing: 6

            // ---- 外观 ----
            SettingsSectionLabel { title: S.S0200 }
            SettingsCard {
                Column {
                    width: parent.width

                    SettingsRow {
                        iconName: "settings"
                        title: S.S0111
                        subtitle: (settings.theme === 0 ? S.S0112 : settings.theme === 1 ? S.S0113 : S.S0114)
                    }
                    Item { width: parent.width; height: 46
                        ChipSelector {
                            anchors { left: parent.left; leftMargin: 16; right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                            items: [S.S0112, S.S0113, S.S0114]
                            selectedIndex: settings.theme
                            onSelected: settings.theme = index
                        }
                    }

                    Rectangle { width: parent.width - 32; x: 16; height: 1; color: appTheme.divider }

                    Item {
                        width: parent.width
                        height: 40
                        Text {
                            font.family: appTheme.fontFamily
                            anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
                            text: S.S0115
                            color: appTheme.textPrimary
                            font.pixelSize: appTheme.fontMedium
                        }
                    }
                    Item { width: parent.width; height: 50
                        Row {
                            anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
                            spacing: 14
                            Repeater {
                                model: ["#00a2ff", "#ff5722", "#4caf50", "#9c27b0", "#ff9800"]
                                delegate: Item {
                                    width: 36; height: 36
                                    Rectangle {
                                        anchors.fill: parent
                                        radius: 18
                                        color: modelData
                                        border.color: appTheme.textPrimary
                                        border.width: settings.accent === modelData ? 3 : 0
                                    }
                                    Text {
                                        visible: settings.accent === modelData
                                        anchors.centerIn: parent
                                        text: "✓"
                                        color: "white"
                                        font.pixelSize: 15
                                        font.bold: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: settings.accent = modelData
                                    }
                                }
                            }
                        }
                    }
                }
            }
            Item { width: 1; height: 10 }

            // ---- 阅读 ----
            SettingsSectionLabel { title: S.S0201 }
            SettingsCard {
                Column {
                    width: parent.width

                    SettingsRow {
                        iconName: "sort"
                        title: S.S0116
                        subtitle: (settings.density === 0 ? S.S0117 : settings.density === 1 ? S.S0118 : S.S0119)
                    }
                    Item { width: parent.width; height: 46
                        ChipSelector {
                            anchors { left: parent.left; leftMargin: 16; right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                            items: [S.S0117, S.S0118, S.S0119]
                            selectedIndex: settings.density
                            onSelected: settings.density = index
                        }
                    }

                    Rectangle { width: parent.width - 32; x: 16; height: 1; color: appTheme.divider }

                    SettingsRow {
                        iconName: "eye"
                        title: S.S0120
                        subtitle: settings.fontScale + "%"
                    }
                    Item {
                        width: parent.width; height: 48
                        Row {
                            anchors { left: parent.left; leftMargin: 16; right: parent.right; rightMargin: 16; verticalCenter: parent.verticalCenter }
                            spacing: 12

                            Rectangle {
                                width: 72; height: 36; radius: appTheme.radius
                                color: fontDown.pressed ? appTheme.divider : appTheme.cardBackground
                                border.color: appTheme.divider
                                Text {
                                    anchors.centerIn: parent
                                    text: "A-"
                                    color: appTheme.textPrimary
                                    font.family: appTheme.fontFamily
                                    font.pixelSize: appTheme.fontMedium
                                }
                                MouseArea { id: fontDown; anchors.fill: parent; onClicked: if (settings.fontScale > 60) settings.fontScale = settings.fontScale - 10 }
                            }
                            Rectangle {
                                width: 72; height: 36; radius: appTheme.radius
                                color: fontUp.pressed ? appTheme.divider : appTheme.cardBackground
                                border.color: appTheme.divider
                                Text {
                                    anchors.centerIn: parent
                                    text: "A+"
                                    color: appTheme.textPrimary
                                    font.family: appTheme.fontFamily
                                    font.pixelSize: appTheme.fontMedium
                                }
                                MouseArea { id: fontUp; anchors.fill: parent; onClicked: if (settings.fontScale < 160) settings.fontScale = settings.fontScale + 10 }
                            }
                        }
                    }

                    Rectangle { width: parent.width - 32; x: 16; height: 1; color: appTheme.divider }

                    SettingsRow {
                        clickable: true
                        iconName: "photo"
                        title: S.S0199
                        showDivider: true
                        onClicked: settings.onlyWifiImages = !settings.onlyWifiImages
                        Switch {
                            checked: settings.onlyWifiImages
                            onCheckedChanged: settings.onlyWifiImages = checked
                        }
                    }
                }
            }
            Item { width: 1; height: 10 }

            // ---- 其他 ----
            SettingsSectionLabel { title: S.S0121 }
            SettingsCard {
                Column {
                    width: parent.width

                    SettingsRow {
                        iconName: "edit"
                        title: S.S0122
                        showDivider: true
                        Switch {
                            checked: settings.immersive
                            onCheckedChanged: settings.immersive = checked
                        }
                    }
                    SettingsRow {
                        iconName: "cloudoff"
                        title: S.S0206
                        subtitle: S.S0207
                        clickable: true
                        onClicked: clearCacheDialog.open()
                    }
                }
            }
        }
    }

    Dialog {
        id: clearCacheDialog
        title: S.S0208
        content: Column {
            width: parent.width
            spacing: 8
            Text {
                id: dialogText
                width: parent.width
                text: S.S0209
                color: appTheme.textSecondary
                font.family: appTheme.fontFamily
                font.pixelSize: appTheme.fontSmall
                wrapMode: Text.WordWrap
            }
        }
        buttons: Row {
            width: parent.width
            spacing: 8
            Button {
                width: (parent.width - parent.spacing) / 2
                text: S.S0023
                font.family: appTheme.fontFamily
                onClicked: clearCacheDialog.reject()
            }
            Button {
                width: (parent.width - parent.spacing) / 2
                text: S.S0147
                font.family: appTheme.fontFamily
                onClicked: clearCacheDialog.accept()
            }
        }
        onAccepted: {
            db.clearCache()
            img.clear()
            util.showToast(S.S0210)
        }
    }
}
