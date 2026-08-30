import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "components/blacklist_helper.js" as BlacklistFilter
import "strings.js" as S

            
Page {
    id: root
    property string tid: ""
    property string title: ""
    property string forumId: ""
    property string forumName: ""

    tools: ToolBarLayout {
        ToolButton { icon: "back"; onClicked: pageStack.pop() }
        ToolButton { icon: "edit"; onClicked: replySheet.open() }
        ToolButton {
            icon: "menu"
            onClicked: (threadMenu.status === DialogStatus.Closed) ? threadMenu.open() : threadMenu.close()
        }
    }

    Menu {
        id: threadMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem {
                text: seeLz ? S.S0134 : S.S0135
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: toggleLz()
            }
            MenuItem {
                text: reverse ? S.S0136 : S.S0137
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: toggleReverse()
            }
            MenuItem {
                text: S.S0138
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: jumpDialog.open()
            }
            MenuItem {
                text: root.stored ? S.S0139 : S.S0140
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                enabled: !root.storePending
                onClicked: toggleStore("")
            }
            MenuItem {
                text: S.S0141
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: util.copyText("https://tieba.baidu.com/p/" + root.tid)
            }
            MenuItem {
                text: S.S0142
                platformStyle: MenuItemStyle { fontFamily: appTheme.fontFamily }
                onClicked: { if (root.posts.length > 0) api.reportPost(root.posts[0].id) }
            }
        }
    }

    property int loadState: 0
    property bool seeLz: false
    property bool reverse: false
    property string threadAuthorId: ""
    property string errorText: S.S0039
    property bool hadLoaded: false
    property variant posts: []
    property int currentPage: 1
    property int totalPage: 1
    property bool offlineNotified: false
    property bool stored: false
    property bool storePending: false
    property variant blockedUsers: []
    property variant blockedKeywords: []

    // Optimistic agree state per floor, keyed "k<pid>": {has: 0/1, count: int}.
    // Reassigning the map re-evaluates only the affected FloorCard bindings —
    // no model reset, no scroll jump.
    property variant agreeOverrides: ({})

    function applyAgreeOverride(pid, wasAgreed) {
        var key = "k" + pid
        var ov = {}
        for (var k in agreeOverrides) ov[k] = agreeOverrides[k]
        var item
        for (var i = 0; i < posts.length; ++i)
            if (String(posts[i].id) === String(pid)) { item = posts[i]; break }
        var cnt = item && item.agree ? (item.agree.diffAgreeNum || item.agree.agreeNum || 0) : 0
        var hasAgree = !!wasAgreed
        if (agreeOverrides[key] !== undefined) {
            cnt = agreeOverrides[key].count
            hasAgree = agreeOverrides[key].has === 1
        }
        ov[key] = { has: hasAgree ? 0 : 1, count: hasAgree ? cnt - 1 : cnt + 1 }
        agreeOverrides = ov
    }

    function clearAgreeOverride(pid) {
        var key = "k" + pid
        if (agreeOverrides[key] === undefined) return
        var ov = {}
        for (var k in agreeOverrides) if (k !== key) ov[k] = agreeOverrides[k]
        agreeOverrides = ov
    }

    // Safety net: returning from a pushed page should never lose the thread.
    onStatusChanged: {
        if (status === PageStatus.Activating && root.hadLoaded && root.posts.length === 0) {
            console.debug("THREAD re-activated with empty list, reloading")
            loadPage(root.currentPage)
        }
    }

    Rectangle { anchors.fill: parent; color: appTheme.background }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        model: posts
        header: Item {
            width: list.width
            height: titleText.height + 24
            visible: root.title !== ""
            Text {
                id: titleText
                font.family: appTheme.fontFamily
                anchors { left: parent.left; leftMargin: 14; right: parent.right; rightMargin: 14; top: parent.top; topMargin: 12 }
                text: root.title
                color: appTheme.textPrimary
                font.pixelSize: appTheme.fontLarge
                font.bold: true
                wrapMode: Text.WordWrap
            }
        }
        delegate: FloorCard {
            post: modelData
            threadAuthorId: root.threadAuthorId
            tid: root.tid
            agreeOverride: root.agreeOverrides["k" + modelData.id]
            onReplyClicked: {
                replySheet.postId = modelData.id
                replySheet.replyUid = modelData.author ? modelData.author.id : ""
                replySheet.open()
            }
            onSubFloorClicked: pageStack.push(Qt.createComponent("SubFloorPage.qml"),
                                              { tid: root.tid, pid: modelData.id, forumId: root.forumId,
                                                forumName: root.forumName })
            onAgreeClicked: {
                var has = modelData.agree && modelData.agree.hasAgree === 1
                api.agree(root.tid, modelData.id, has)
                // Optimistic UI patch — reloading the page here reset the
                // scroll position to the top (Qt4 QML array-model reset).
                root.applyAgreeOverride(modelData.id, has)
            }
            onStoreClicked: {
                toggleStore(modelData.id)
            }
            onAuthorClicked: {
                if (modelData.author && modelData.author.id)
                    pageStack.push(Qt.createComponent("UserProfilePage.qml"), { uid: modelData.author.id })
            }
            onImageClicked: pageStack.push(Qt.createComponent("PhotoViewPage.qml"), { images: images, index: index })
        }
    }

    StatusView {
        anchors.fill: parent
        visible: root.loadState !== 1
        status: root.loadState === 0 ? 0 : (root.loadState === 3 ? 2 : 1)
        errorText: root.errorText
        emptyText: S.S0143
        onRetry: load()
    }

    ReplySheet {
        id: replySheet
        fid: root.forumId
        kw: root.forumName
        tid: root.tid
        title: root.title
        isNewThread: false
    }

    Dialog {
        id: jumpDialog
        content: Column {
            anchors { left: parent.left; right: parent.right; top: parent.top }
            spacing: 8
            Text {
                font.family: appTheme.fontFamily
                anchors { left: parent.left; right: parent.right }
                text: S.S0144 + root.currentPage + S.S0145 + root.totalPage + S.S0146
                color: appTheme.textPrimary
                font.pixelSize: appTheme.fontSmall
                wrapMode: Text.WordWrap
            }
            TextField {
                id: pageInput
                platformStyle: TextFieldStyle { textFont.family: appTheme.fontFamily }
                anchors { left: parent.left; right: parent.right }
                inputMethodHints: Qt.ImhDigitsOnly
            }
        }
        buttons: Row {
            spacing: 8
            Button { text: S.S0023; font.family: appTheme.fontFamily; onClicked: jumpDialog.reject() }
            Button { text: S.S0147; font.family: appTheme.fontFamily; onClicked: jumpDialog.accept() }
        }
        onStatusChanged: {
            if (status === DialogStatus.Open) pageInput.text = ""
        }
        onAccepted: {
            var p = parseInt(pageInput.text, 10)
            if (isNaN(p) || p < 1) { notifier.notify(S.S0148, ""); return }
            root.loadPage(p)
        }
    }

    Connections {
        target: api
        onActionFinished: {
            if (action === "report") {
                if (ok && data.url !== "") util.openUrl(data.url)
                else notifier.notify(S.S0149, message)
            } else if (action === "store" && data.tid === root.tid) {
                root.storePending = false
                if (ok) {
                    root.stored = data.stored === 1
                    if (root.stored)
                        db.addFavorite({ tid: root.tid, title: root.title, forum_name: root.forumName,
                                         forum_id: root.forumId })
                    else
                        db.removeFavorite(root.tid)
                    notifier.notify(root.stored ? S.S0150 : S.S0151, "")
                } else {
                    notifier.notify(S.S0152, message)
                }
            } else if (action === "agree" && data.tid === root.tid) {
                if (!ok) {
                    // Revert the optimistic patch and tell the user.
                    root.clearAgreeOverride(data.pid)
                    notifier.notify(S.S0153, message)
                }
                // Success: the local override already shows the final state.
            }
        }
        onThreadPageReady: {
            if (error !== "") {
                root.loadState = 3
                root.errorText = error
            } else {
                if (data.offline === 1 && !root.offlineNotified) {
                    root.offlineNotified = true
                    notifier.notify(S.S0050, S.S0154)
                }
                var incoming = data.posts ? data.posts : []
                var kept = []
                for (var pi = 0; pi < incoming.length; pi++) {
                    if (!BlacklistFilter.isBlacklisted(incoming[pi], root.blockedUsers, root.blockedKeywords))
                        kept.push(incoming[pi])
                }
                // Assign once; mutating a variant list property in place is
                // unreliable on Qt4's QML engine.
                root.posts = kept
                console.debug("THREAD ui: got " + incoming.length + ", kept " + kept.length)
                if (data.page) {
                    root.currentPage = data.page.currentPage ? data.page.currentPage : 1
                    root.totalPage = data.page.totalPage ? data.page.totalPage : root.currentPage
                }
                if (data.thread && data.thread.author && data.thread.author.id)
                    root.threadAuthorId = data.thread.author.id
                else if (data.user && data.user.id)
                    root.threadAuthorId = data.user.id
                if (root.title === "" && data.thread && data.thread.title)
                    root.title = data.thread.title
                root.loadState = root.posts.length > 0 ? 1 : 2
                if (root.loadState === 1) root.hadLoaded = true
            }
        }
    }

    function load() {
        console.debug("THREAD load tid=" + root.tid + " pn=1 forumId=" + root.forumId)
        root.loadState = 0
        root.posts = []
        root.agreeOverrides = ({})
        root.offlineNotified = false
        root.stored = db.isFavorite(root.tid)
        db.addHistory({ type: "1", tid: root.tid, title: root.title, forum_name: root.forumName,
                        forum_id: root.forumId })
        api.loadThreadPage(root.tid, 1, root.seeLz, root.reverse, root.forumId)
    }

    function loadPage(p) {
        if (p < 1) return
        root.loadState = 0
        root.currentPage = p
        root.agreeOverrides = ({})
        root.offlineNotified = false
        api.loadThreadPage(root.tid, p, root.seeLz, root.reverse, root.forumId)
    }

    function toggleLz() {
        root.seeLz = !root.seeLz
        load()
    }

    function toggleReverse() {
        root.reverse = !root.reverse
        load()
    }

    function toggleStore(pid) {
        if (root.storePending) return
        root.storePending = true
        if (root.stored) api.removeStore(root.tid)
        else api.addStore(root.tid, pid)
    }

    Component.onCompleted: { reloadBlacklist(); load() }

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
