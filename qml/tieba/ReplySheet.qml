import QtQuick 1.1
import com.nokia.meego 1.0
import "strings.js" as S

                                  
Sheet {
    id: sheet
    property string fid: ""
    property string kw: ""
    property string tid: ""
    property string postId: ""
    property string subPostId: ""
    property string replyUid: ""
    property string title: ""
    property string initialText: ""
    property variant initialImages: []
    property int draftId: -1
    property bool isNewThread: false
    property variant imagePaths: []
    property bool uploading: false
    property bool sending: false
    property bool sessionActive: false
    property bool committed: false

    content: Column {
        id: contentCol
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: 8

        Text {
           font.family: appTheme.fontFamily
            text: sheet.isNewThread ? (S.S0096 + sheet.kw) : (sheet.title !== "" ? sheet.title : S.S0088)
           color: appTheme.textPrimary
            font.pixelSize: appTheme.fontMedium
            font.bold: true
        }

        TextArea {
            id: input
            platformStyle: TextAreaStyle { textFont.family: appTheme.fontFamily }
            anchors { left: parent.left; right: parent.right }
            height: 140
            placeholderText: S.S0097
        }

                                                            
        Flickable {
            id: thumbFlick
            anchors { left: parent.left; right: parent.right }
            height: 62
            contentWidth: thumbRow.width
            clip: true
            visible: sheet.imagePaths.length > 0
            Row {
                id: thumbRow
                spacing: 6
                Repeater {
                    model: sheet.imagePaths
                    delegate: Item {
                        width: 56; height: 56
                        Image {
                            anchors.fill: parent
                            source: "file://" + modelData
                            fillMode: Image.PreserveAspectCrop
                            clip: true
                        }
                        Rectangle {
                            anchors { top: parent.top; right: parent.right }
                            width: 18; height: 18
                            color: "#cc0000"
                            Text {
                                font.family: appTheme.fontFamily
                                anchors.centerIn: parent
                                text: S.S0034
                                color: "white"
                                font.pixelSize: 10
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: removeImage(index)
                        }
                    }
                }
            }
        }
    }

    buttons: Row {
        spacing: 8
        Button { text: S.S0023; font.family: appTheme.fontFamily; enabled: !sheet.uploading && !sheet.sending; onClicked: sheet.close() }
        Button { text: S.S0098; font.family: appTheme.fontFamily; enabled: !sheet.uploading && !sheet.sending; onClicked: doDraft() }
        Button { text: S.S0099; font.family: appTheme.fontFamily; enabled: !sheet.uploading && !sheet.sending; onClicked: pick() }
       Button {
            text: sheet.uploading ? S.S0100 : S.S0101
           font.family: appTheme.fontFamily
            enabled: !sheet.uploading && !sheet.sending && (input.text !== "" || sheet.imagePaths.length > 0)
            onClicked: doSend()
        }
    }

    function pick() {
        var p = util.pickImage()
        if (p !== "") {
            var arr = sheet.imagePaths.slice()
            arr.push(p)
            sheet.imagePaths = arr
        }
    }

    function removeImage(index) {
        var arr = sheet.imagePaths.slice()
        arr.splice(index, 1)
        sheet.imagePaths = arr
    }

    function doDraft() {
        if (input.text === "" && sheet.imagePaths.length === 0) { sheet.close(); return }
       saveCurrentDraft()
       sheet.committed = true
       sheet.close()
        notifier.notify(S.S0102, "")
   }

    function saveCurrentDraft() {
        if (sheet.draftId >= 0) db.removeDraft(sheet.draftId)
        sheet.draftId = db.saveDraft({ content: input.text, forum_name: sheet.kw, forum_id: sheet.fid,
                                       thread_id: sheet.tid, thread_title: sheet.title,
                                       floor: sheet.postId, images: sheet.imagePaths.join(",") })
    }

    function doSend() {
        if (input.text === "" && sheet.imagePaths.length === 0) return
        if (sheet.imagePaths.length === 0) { sendText(input.text); return }
        sheet.sending = true
        sheet.uploading = true
        api.uploadImages(sheet.imagePaths, sheet.kw)
    }

    function sendText(text) {
        sheet.sending = true
        var args = {
            content: text,
            fid: sheet.fid,
            kw: sheet.kw,
            tid: sheet.tid,
            postId: sheet.postId,
            subPostId: sheet.subPostId,
            replyUid: sheet.replyUid,
            nameShow: accounts.name,
            tbs: accounts.tbs
        }
        api.addPost(args)
    }

    Connections {
        target: api
        onUploadDone: {
            if (!sheet.uploading) return
           sheet.uploading = false
            if (error !== "") { sheet.sending = false; notifier.notify(S.S0103, error); return }
            var lines = []
            for (var i = 0; i < results.length; i++) {
                var r = results[i]
                lines.push("#(pic," + r.picId + "," + r.width + "," + r.height + ")")
            }
            var text = input.text
            if (lines.length > 0 && text !== "") text += "\n"
            text += lines.join("\n")
            sendText(text)
        }
        onActionFinished: {
            if (action === "addPost" && sheet.sending) {
                sheet.sending = false
                if (ok) {
                    sheet.committed = true
                   if (sheet.draftId >= 0) { db.removeDraft(sheet.draftId); sheet.draftId = -1 }
                   sheet.close()
                    notifier.notify(S.S0104, "")
               } else {
                    notifier.notify(S.S0105, message)
               }
            }
        }
    }

    function beginSession() {
        input.text = sheet.initialText
        sheet.imagePaths = sheet.initialImages.slice()
        sheet.initialText = ""
        sheet.initialImages = []
        sheet.sessionActive = true
        sheet.committed = false
        input.forceActiveFocus()
    }

    function finishSession() {
        if (sheet.sessionActive && !sheet.committed &&
                (input.text !== "" || sheet.imagePaths.length > 0))
            saveCurrentDraft()
        sheet.sessionActive = false
        sheet.sending = false
        sheet.uploading = false
    }

    onStatusChanged: {
        if (status === DialogStatus.Open)
            beginSession()
        else if (status === DialogStatus.Closed)
            finishSession()
    }
}
