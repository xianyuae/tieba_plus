import QtQuick 1.1
import com.nokia.meego 1.0
import "components"
import "components/blacklist_helper.js" as BlacklistFilter
import "strings.js" as S

// 个性化推荐 (personalized feed, cmd 309264).
// Same card list as forum pages; pagination via a footer button because
// pull-to-refresh does not exist on Qt Quick 1.1.
Item {
    id: root
    property variant stack: null

    property int loadState: 0 // 0 loading, 1 ok, 2 empty, 3 error
    property int pn: 1
    property bool loadingMore: false
    property bool hasMore: false
    property variant threads: []
    property string errorText: S.S0039

    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 56
        color: appTheme.cardBackground
        Rectangle { anchors { left: parent.left; right: parent.right; bottom: parent.bottom } height: 2; color: appTheme.accent }

        Text {
            font.family: appTheme.fontFamily
            anchors { left: parent.left; leftMargin: 16; verticalCenter: parent.verticalCenter }
            text: S.S0035
            color: appTheme.textPrimary
            font.pixelSize: appTheme.fontLarge
            font.bold: true
        }

        Item {
            anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
            width: 40
            height: 40
            opacity: refreshArea.pressed ? 0.55 : 1.0

            SvgIcon {
                anchors.centerIn: parent
                width: 22
                height: 22
                name: "refresh"
            }
            MouseArea {
                id: refreshArea
                anchors.fill: parent
                onClicked: reload()
            }
        }
    }

    ListView {
        id: list
        anchors { top: header.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
        clip: true
        model: root.threads

        // Reassigning a JS-array model resets the view to the top; remember
        // the offset across a load-more append and restore it when it lands.
        property real pendingY: -1
        onCountChanged: if (pendingY >= 0) { contentY = pendingY; pendingY = -1 }
        spacing: appTheme.spacingMedium

        delegate: ThreadCard {
            thread: modelData
            onClicked: {
                var comp = Qt.createComponent("ThreadPage.qml")
                if (comp.status === Component.Error) {
                    console.debug("ThreadPage create FAILED: " + comp.errorString())
                    return
                }
                root.stack.push(comp,
                              { tid: modelData.tid, title: modelData.title,
                                forumId: modelData.forumId, forumName: modelData.forumName })
            }
        }

        footer: Item {
            width: list.width
            height: 52
            Button {
                anchors.centerIn: parent
                visible: root.loadState === 1 && root.hasMore && !root.loadingMore
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
        emptyText: S.S0197
        onRetry: reload()
    }

    Connections {
        target: api
        onPersonalizedReady: {
            root.loadingMore = false
            if (error !== "") {
                // First page failed -> error state; load-more failure keeps the list.
                if (root.pn === 1) {
                    root.loadState = 3
                    root.errorText = error
                } else {
                    root.pn -= 1
                    notifier.notify(S.S0131, error)
                }
                return
            }
            var incoming = []
            var seen = {}
            for (var i = 0; i < root.threads.length; i++)
                seen[root.threads[i].tid] = 1
            for (var j = 0; j < threads.length; j++) {
                var t = threads[j]
                if (!t.tid || seen[t.tid]) continue
                if (BlacklistFilter.isBlacklisted(t, [], [])) continue
                seen[t.tid] = 1
                incoming.push(t)
            }
            // Assign once; in-place mutation is unreliable on Qt4's QML engine.
            // Arm the scroll restore before the append (model reset snaps to top).
            if (root.pn > 1) list.pendingY = list.contentY
            root.threads = (root.pn === 1) ? incoming : root.threads.concat(incoming)
            root.hasMore = hasMore || incoming.length > 0
            root.loadState = root.threads.length > 0 ? 1 : 2
        }
    }

    function reload() {
        root.pn = 1
        root.loadState = 0
        root.threads = []
        root.hasMore = false
        api.loadPersonalized(1)
    }

    function loadMore() {
        root.loadingMore = true
        root.pn += 1
        api.loadPersonalized(root.pn)
    }

    Component.onCompleted: reload()
}
