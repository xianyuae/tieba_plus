import QtQuick 1.1
import com.nokia.meego 1.0
import "util.js" as U
import "components"
import "strings.js" as S

                  
Page {
    id: root
    property int tab: 0                                 
    property int loadState: -1                                                           
    property variant results: []
    property bool hasMore: false
    property int page: 1
    property bool loadingMore: false
    property string errorText: S.S0106
    property variant historyList: []

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
    }

    Rectangle { anchors.fill: parent; color: appTheme.background }

                   
    Rectangle {
        id: searchBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 54
        color: appTheme.toolbarBackground
        Row {
            anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
            spacing: 8
            TextField {
                id: searchField
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                width: parent.width - 80
                anchors.verticalCenter: parent.verticalCenter
                placeholderText: S.S0107
                onAccepted: doSearch()
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                text: S.S0081
                font.family: appTheme.fontFamily
                enabled: !root.loadingMore && root.loadState !== 0
                onClicked: doSearch()
            }
        }
    }

           
    Row {
        id: tabs
        anchors { top: searchBar.bottom; left: parent.left; right: parent.right }
        height: 40
       Repeater {
            model: [ S.S0108, S.S0109, S.S0068 ]
           delegate: Item {
                width: tabs.width / 3
                height: tabs.height
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width; height: 2
                    color: index === root.tab ? appTheme.accent : "transparent"
                }
                Text {
                    font.family: appTheme.fontFamily
                    anchors.centerIn: parent
                    text: modelData
                    color: index === root.tab ? appTheme.accent : appTheme.textSecondary
                    font.pixelSize: appTheme.fontMedium
                }
                MouseArea { anchors.fill: parent; onClicked: { root.tab = index; if (searchField.text !== "") doSearch() } }
            }
        }
    }

                          
    ListView {
        id: historyView
        visible: root.loadState === -1
        anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: historyList
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: historyView.width; height: 44
            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                text: modelData.keyword
                color: appTheme.textPrimary
                font.pixelSize: appTheme.fontMedium
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea { anchors.fill: parent; onClicked: { searchField.text = modelData.keyword; doSearch() } }
        }
    }

                     
    ListView {
        id: threadList
        visible: root.loadState !== -1 && root.tab === 0
        anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: root.tab === 0 ? results : []
        spacing: appTheme.spacingMedium
        // Reassigning a JS-array model resets the view to the top; remember
        // the offset across a load-more append and restore it when it lands.
        property real pendingY: -1
        onCountChanged: if (pendingY >= 0) { contentY = pendingY; pendingY = -1 }
        delegate: ThreadCard {
            thread: modelData
            onClicked: pageStack.push(Qt.createComponent("ThreadPage.qml"),
                                      { tid: modelData.tid, title: modelData.title, forumId: modelData.forumId, forumName: modelData.forumName })
        }
        footer: Item {
            width: threadList.width
            height: root.tab === 0 && root.hasMore ? 48 : 0
            Button {
                anchors.centerIn: parent
                visible: root.hasMore && !root.loadingMore
                text: S.S0044
                font.family: appTheme.fontFamily
                onClicked: loadMore()
            }
            BusyIndicator {
                anchors.centerIn: parent
                visible: root.loadingMore
                running: root.loadingMore
            }
        }
    }

                    
    ListView {
        id: forumList
        visible: root.loadState !== -1 && root.tab === 1
        anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: root.tab === 1 ? results : []
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: forumList.width; height: 64
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Row {
                anchors { fill: parent; leftMargin: 12; rightMargin: 12 }
                spacing: 12
                Avatar { source: U.avatarUrl(modelData.avatar || ""); size: 44; anchors.verticalCenter: parent.verticalCenter }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 120
                    spacing: 2
                    Text { font.family: appTheme.fontFamily; text: modelData.name || ""; color: appTheme.textPrimary; font.pixelSize: appTheme.fontMedium; font.bold: true }
                    Text { font.family: appTheme.fontFamily; text: modelData.intro || ""; color: appTheme.textTertiary; font.pixelSize: appTheme.fontSmall; elide: Text.ElideRight }
                }
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: pageStack.push(Qt.createComponent("ForumPage.qml"), { kw: modelData.name, fid: modelData.fid })
            }
        }
    }

                   
    ListView {
        id: userList
        visible: root.loadState !== -1 && root.tab === 2
        anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: root.tab === 2 ? results : []
        spacing: appTheme.spacingMedium
        delegate: Item {
            width: userList.width; height: 64
            Rectangle { anchors.fill: parent; color: appTheme.cardBackground }
            Row {
                anchors { fill: parent; leftMargin: 12; rightMargin: 12 }
                spacing: 12
                Avatar { source: U.avatarUrl(modelData.portrait || ""); size: 44; anchors.verticalCenter: parent.verticalCenter }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 80
                    spacing: 2
                    Text { font.family: appTheme.fontFamily; text: modelData.name || ""; color: appTheme.textPrimary; font.pixelSize: appTheme.fontMedium; font.bold: true }
                    Text { font.family: appTheme.fontFamily; text: modelData.intro || ""; color: appTheme.textTertiary; font.pixelSize: appTheme.fontSmall; elide: Text.ElideRight }
                }
            }
            Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 1; color: appTheme.divider }
            MouseArea {
                anchors.fill: parent
                onClicked: pageStack.push(Qt.createComponent("UserProfilePage.qml"), { uid: modelData.uid })
            }
        }
    }

    StatusView {
        anchors { top: tabs.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        visible: root.loadState === 0 || root.loadState === 2 || root.loadState === 3
        status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
        emptyText: S.S0110
        errorText: root.errorText
        onRetry: doSearch()
    }

    Connections {
        target: api
        onSearchThreadsReady: {
            if (root.tab !== 0) return
            root.loadingMore = false
            if (error !== "") { root.errorText = error; root.loadState = 3; return }
            // Arm the scroll restore before the append (model reset snaps to top).
            if (root.page > 1) threadList.pendingY = threadList.contentY
            root.results = root.page === 1 ? items : root.results.concat(items)
            root.hasMore = hasMore
            root.loadState = root.results.length > 0 ? 1 : 2
        }
        onSearchForumsReady: {
            if (root.tab !== 1) return
            if (error !== "") { root.errorText = error; root.loadState = 3; return }
            root.results = items; root.loadState = items.length > 0 ? 1 : 2
        }
        onSearchUsersReady: {
            if (root.tab !== 2) return
            if (error !== "") { root.errorText = error; root.loadState = 3; return }
            root.results = items; root.loadState = items.length > 0 ? 1 : 2
        }
    }

    function doSearch() {
        if (searchField.text === "") return
        db.addSearch(searchField.text)
        root.loadState = 0
        root.results = []
        root.page = 1
        root.hasMore = false
        root.loadingMore = false
        root.errorText = S.S0106
        if (root.tab === 0) api.searchThread(searchField.text, root.page)
        else if (root.tab === 1) api.searchForum(searchField.text)
        else api.searchUser(searchField.text)
    }

    function loadMore() {
        if (root.loadingMore || !root.hasMore || root.tab !== 0) return
        root.loadingMore = true
        root.page += 1
        api.searchThread(searchField.text, root.page)
    }

    function loadHistory() {
        root.historyList = db.searchHistory()
    }

    Component.onCompleted: loadHistory()
}
