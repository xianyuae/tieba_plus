import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "components/blacklist_helper.js" as BlacklistFilter
import "strings.js" as S

                  
Page {
    id: root
    property string kw: ""
    property string fid: ""

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "search"; onClicked: pageStack.push(Qt.createComponent("SearchPage.qml"), { forumName: root.kw }) }
        ToolButton { icon: "edit"; onClicked: replySheet.open() }
        ToolButton {
            icon: "menu"
            onClicked: (sortMenu.status === DialogStatus.Closed) ? sortMenu.open() : sortMenu.close()
        }
    }

    Menu {
        id: sortMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: S.S0037
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: reload(0)
            }
            MenuItem {
                text: S.S0038
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: reload(1)
            }
        }
    }

    property int loadState: 0
    property int pn: 1
    property int sortType: 0
    property bool hasMore: false
    property bool loadingMore: false
    property bool hadLoaded: false
    property variant threads: []
    property variant forum: ({})
    property string errorText: S.S0039
    property bool offlineNotified: false
    property int likeState: -1
    property bool forumActionPending: false
    property variant blockedUsers: []
    property variant blockedKeywords: []

    // Safety net: returning from a pushed page should never lose the list.
    onStatusChanged: {
        if (status === PageStatus.Activating && root.hadLoaded && root.threads.length === 0) {
            console.debug("FORUM re-activated with empty list, reloading")
            reload(root.sortType)
        }
    }

    Rectangle { anchors.fill: parent; color: appTheme.background }

                   
    Rectangle {
        id: forumHeader
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 68
        color: appTheme.cardBackground

        Rectangle {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: 2
            color: appTheme.accent
        }

        Avatar {
            anchors { left: parent.left; leftMargin: 14; verticalCenter: parent.verticalCenter }
            size: 48
            source: forum.avatar ? forum.avatar : ""
        }
        Column {
            anchors { left: parent.left; leftMargin: 70; right: parent.right; rightMargin: 64; verticalCenter: parent.verticalCenter }
            spacing: 3
            Text {
                font.family: appTheme.fontFamily
                width: parent.width
                text: forum.name ? forum.name : root.kw
                color: appTheme.textPrimary
                font.pixelSize: appTheme.fontLarge
                font.bold: true
                elide: Text.ElideRight
            }
            Text {
                font.family: appTheme.fontFamily
                width: parent.width
                text: S.S0040 + (forum.threadNum !== undefined ? forum.threadNum : "") +
                      S.S0041 + (forum.memberNum !== undefined ? forum.memberNum : "")
                color: appTheme.textTertiary
                font.pixelSize: appTheme.fontSmall
                elide: Text.ElideRight
            }
        }
        // Compact SVG icon button instead of a full-size Meego Button:
        // 40x40 touch target, no text layout cost on every header repaint.
        Item {
            anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
            width: 40
            height: 40
            enabled: !root.forumActionPending
            opacity: enabled ? 1.0 : 0.4

            Rectangle {
                anchors.fill: parent
                radius: appTheme.radius
                color: likeArea.pressed ? appTheme.divider : "transparent"
            }
            SvgIcon {
                anchors.centerIn: parent
                width: 22
                height: 22
                name: root.likeState === 1 ? "check" : "plus"
            }
            MouseArea {
                id: likeArea
                anchors.fill: parent
                onClicked: {
                    root.forumActionPending = true
                    if (root.likeState === 1) api.unlikeForum(root.fid, root.kw)
                    else api.likeForum(root.fid, root.kw)
                }
            }
        }
    }

    ListView {
        id: list
        anchors { top: forumHeader.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: threads
        spacing: appTheme.spacingMedium
        // Reassigning a JS-array model resets the view to the top; remember
        // the offset across a load-more append and restore it when it lands.
        property real pendingY: -1
        onCountChanged: {
            console.debug("FRS list count=" + count)
            if (pendingY >= 0) { contentY = pendingY; pendingY = -1 }
        }
        delegate: ThreadCard {
            thread: modelData
            onClicked: {
                console.debug("FORUM tap thread tid=" + modelData.tid + " forumId=" + modelData.forumId)
                var comp = Qt.createComponent("ThreadPage.qml")
                if (comp.status === Component.Error) {
                    console.debug("ThreadPage create FAILED: " + comp.errorString())
                    return
                }
                pageStack.push(comp,
                              { tid: modelData.tid, title: modelData.title, forumId: modelData.forumId, forumName: root.kw })
            }
        }
        footer: Item {
            width: list.width
            height: 48
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

    StatusView {
        anchors.fill: parent
        visible: root.loadState !== 1
        status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
        errorText: root.errorText
        emptyText: S.S0045
        onRetry: reload(root.sortType)
    }

    ReplySheet {
        id: replySheet
        fid: root.fid
        kw: root.kw
        tid: ""
        title: S.S0046
        isNewThread: true
    }

    Connections {
        target: api
        onActionFinished: {
            if ((action !== "like" && action !== "unlike") || data.fid !== root.fid) return
            root.forumActionPending = false
            if (ok) {
                root.likeState = action === "like" ? 1 : 0
                notifier.notify(root.likeState === 1 ? S.S0047 : S.S0048, "")
            } else {
                notifier.notify(S.S0049, message)
            }
        }
        onFrsPageReady: {
            if (error !== "") {
                root.loadState = 3
                root.errorText = error
            } else {
                if (data.offline === 1 && !root.offlineNotified) {
                    root.offlineNotified = true
                    notifier.notify(S.S0050, S.S0051)
                }
                root.forum = data.forum ? data.forum : ({})
                if (root.fid === "" && root.forum.id !== undefined)
                    root.fid = root.forum.id
                root.likeState = root.forum.isLike === 1 ? 1 : 0
                var incoming = data.threads ? data.threads : []
                var kept = []
                for (var ti = 0; ti < incoming.length; ti++) {
                    if (!BlacklistFilter.isBlacklisted(incoming[ti], root.blockedUsers, root.blockedKeywords))
                        kept.push(incoming[ti])
                }
                // Assign once; mutating a variant list property in place is
                // unreliable on Qt4's QML engine.
                // Arm the scroll restore before the append (model reset snaps to top).
                if (root.pn > 1) list.pendingY = list.contentY
                root.threads = (root.pn === 1) ? kept : root.threads.concat(kept)
                console.debug("FRS ui: got " + incoming.length + ", kept " + kept.length +
                              ", total " + root.threads.length +
                              ", first=" + (root.threads.length > 0 ? root.threads[0].title : "-"))
                var page = data.page ? data.page : ({})
                root.hasMore = page.hasMore === 1
                root.loadState = root.threads.length > 0 ? 1 : 2
                if (root.loadState === 1) root.hadLoaded = true
            }
            root.loadingMore = false
        }
    }

    function reload(sort) {
        root.sortType = sort
        root.pn = 1
        root.loadState = 0
        root.threads = []
        root.forum = ({})
        root.likeState = -1
        root.forumActionPending = false
        root.offlineNotified = false
        api.loadFrsPage(root.kw, 1, sort)
    }

    function loadMore() {
        root.loadingMore = true
        root.pn += 1
        api.loadFrsPage(root.kw, root.pn, root.sortType)
    }

    Component.onCompleted: { reloadBlacklist(); reload(0) }

    function reloadBlacklist() {
        var us = db.blacklistUsers()
        var ks = db.blacklistKeywords()
        var uArr = []
        for (var i = 0; i < us.length; i++) uArr.push(us[i])
        var kArr = []
        for (var j = 0; j < ks.length; j++) kArr.push(ks[j])
        root.blockedUsers = uArr
        root.blockedKeywords = kArr
    }
}
