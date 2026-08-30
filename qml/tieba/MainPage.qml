import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "strings.js" as S

Page {
    id: mainPage
    property int currentIndex: 0

    // Single bottom navigation: the four main tabs live in the native Meego
    // toolbar (leftmost group); the app menu stays right-aligned as usual.
    tools: ToolBarLayout {
        ToolButton {
            icon: "home"
            opacity: mainPage.currentIndex === 0 ? 1.0 : 0.55
            onClicked: mainPage.currentIndex = 0
        }
        ToolButton {
            icon: "forum"
            opacity: mainPage.currentIndex === 1 ? 1.0 : 0.55
            onClicked: mainPage.currentIndex = 1
        }
        ToolButton {
            icon: "notify"
            opacity: mainPage.currentIndex === 2 ? 1.0 : 0.55
            onClicked: mainPage.currentIndex = 2
        }
        ToolButton {
            icon: "person"
            opacity: mainPage.currentIndex === 3 ? 1.0 : 0.55
            onClicked: mainPage.currentIndex = 3
        }
        ToolButton {
            icon: "menu"
            onClicked: (myMenu.status === DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }

    Menu {
        id: myMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: S.S0081
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: pageStack.push(Qt.createComponent("SearchPage.qml"), {})
            }
            MenuItem {
                text: S.S0082
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: pageStack.push(Qt.createComponent("LoginPage.qml"), {})
            }
            MenuItem {
                text: S.S0083
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: pageStack.push(Qt.createComponent("SettingsPage.qml"), {})
            }
            MenuItem {
                text: S.S0084
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: pageStack.push(Qt.createComponent("AboutPage.qml"), {})
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: appTheme.background
    }

    Item {
        id: contentArea
        anchors { top: parent.top; left: parent.left; right: parent.right; bottom: parent.bottom }
        HomePage { anchors.fill: parent; stack: pageStack; visible: mainPage.currentIndex === 0 }
        ExplorePage { anchors.fill: parent; stack: pageStack; visible: mainPage.currentIndex === 1 }
        NotificationsPage { anchors.fill: parent; stack: pageStack; visible: mainPage.currentIndex === 2 }
        UserPage { anchors.fill: parent; stack: pageStack; visible: mainPage.currentIndex === 3 }
    }
}
