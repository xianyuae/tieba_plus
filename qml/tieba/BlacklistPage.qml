import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

                
Page {
    id: root

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "edit"; onClicked: addMenu.open() }
    }

    Menu {
        id: addMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: S.S0016
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: userDialog.open()
            }
            MenuItem {
                text: S.S0017
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: keywordDialog.open()
            }
        }
    }

    property variant users: []
    property variant keywords: []

   Rectangle { anchors.fill: parent; color: appTheme.background }

   ListView {
       id: list
       anchors.fill: parent
       clip: true
       model: combined
       delegate: Item {
           width: list.width
           height: 50
           Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
           Text {
               font.family: appTheme.fontFamily
               anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
               text: modelData.display
               color: appTheme.textPrimary
               font.pixelSize: appTheme.fontMedium
               elide: Text.ElideRight
           }
           Text {
               font.family: appTheme.fontFamily
               anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                text: S.S0018
               color: appTheme.accent
               font.pixelSize: appTheme.fontSmall
           }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (modelData.kind === "user") db.removeBlacklistUser(modelData.uid)
                    else db.removeBlacklistKeyword(modelData.keyword)
                    refresh()
                }
            }
       }
   }

                                                                         
    Dialog {
        id: userDialog
        title: S.S0016

        content: Column {
            width: parent.width
            spacing: 10
            Label {
                font.family: appTheme.fontFamily
                text: S.S0019
            }
            TextField {
                id: uidInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                width: parent.width
                placeholderText: S.S0020
            }
            Label {
                font.family: appTheme.fontFamily
                text: S.S0021
            }
            TextField {
                id: userNameInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                width: parent.width
                placeholderText: S.S0022
            }
        }

        buttons: Row {
            spacing: 8
            Button {
                text: S.S0023
                font.family: appTheme.fontFamily
                onClicked: userDialog.close()
            }
            Button {
                text: S.S0024
                font.family: appTheme.fontFamily
                enabled: uidInput.text !== ""
                onClicked: {
                    db.addBlacklistUser(uidInput.text, userNameInput.text !== "" ? userNameInput.text : uidInput.text)
                    uidInput.text = ""
                    userNameInput.text = ""
                    userDialog.close()
                    refresh()
                }
            }
        }
    }

                                                                   
    Dialog {
        id: keywordDialog
        title: S.S0017

        content: Column {
            width: parent.width
            spacing: 10
            Label {
                font.family: appTheme.fontFamily
                text: S.S0025
            }
            TextField {
                id: keywordInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                width: parent.width
                placeholderText: S.S0026
            }
        }

        buttons: Row {
            spacing: 8
            Button {
                text: S.S0023
                font.family: appTheme.fontFamily
                onClicked: keywordDialog.close()
            }
            Button {
                text: S.S0024
                font.family: appTheme.fontFamily
                enabled: keywordInput.text !== ""
                onClicked: {
                    db.addBlacklistKeyword(keywordInput.text)
                    keywordInput.text = ""
                    keywordDialog.close()
                    refresh()
                }
            }
        }
    }

    property variant combined: []

   function refresh() {
       root.users = db.blacklistUsers()
       root.keywords = db.blacklistKeywords()
       var out = []
       for (var i = 0; i < root.users.length; i++) {
            out.push({ kind: "user", display: S.S0027 + root.users[i].name, uid: root.users[i].uid })
       }
       for (var j = 0; j < root.keywords.length; j++) {
            out.push({ kind: "keyword", display: S.S0028 + root.keywords[j].keyword, keyword: root.keywords[j].keyword })
       }
       root.combined = out
   }

   StatusView {
       anchors.fill: parent
       visible: root.combined.length === 0
       status: 1
        emptyText: S.S0029
   }

   Component.onCompleted: refresh()
}
