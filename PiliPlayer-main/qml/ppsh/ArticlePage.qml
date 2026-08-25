import QtQuick 1.1
import com.nokia.meego 1.1
import "component"
import "../js/main.js" as Script
import "../js/util.js" as Util

BasePage {
	id: root;

	sTitle: qsTr("Article");
	objectName: "idArticlePage";
	property string sError: "";
	property string sArticleUrl: "";
	property string previewSrc: "";

	function _Init(id)
	{
		obj._View(id);
	}

	QtObject {
		id: obj;
		property string input: "";
		property string articleId: "";
		property string commentId: "";
		property string order: typerow.vCurrentValue;
		property int pageNo: 1;
		property int pageSize: 20;
		property int pageCount: 0;
		property int totalCount: 0;
		property int num: -1;
		property variant blocks: [];
		property bool __loadImage: true;
		property bool __helper: false;

		function _View(id)
		{
			if(!id) return;
			var parsed = Script.ParseArticleId(id);
			if(!parsed)
			{
				root.sError = qsTr("Invalid article link");
				root.bBusy = false;
				return;
			}
			obj.input = id;
			obj.articleId = parsed.id;
			root.sArticleUrl = parsed.url;
			root.sError = "";
			root.previewSrc = "";
			root.bBusy = true;
			obj.blocks = [];
			view.model = obj.blocks;
			obj.num = -1;
			obj.pageNo = 1;
			obj.pageSize = 20;
			obj.pageCount = 0;
			obj.totalCount = 0;
			Util.ModelClear(commentview.model);
			flip._Toggle(false);

			var d = {
				aid: id,
			};
			var s = function(data){
				root.bBusy = false;
				if(data && data.redirect)
				{
					controller._OpenUrl(data.url, 0);
					pageStack.pop();
					return;
				}
				obj._Apply(data);
			};
			var f = function(err){
				root.bBusy = false;
				root.sError = err || qsTr("Load failed");
			};
			Script.GetArticleDetail(d, s, f);
		}

		function _Apply(data)
		{
			if(!data) return;
			obj.blocks = data.blocks || [];
			root.sTitle = data.title || qsTr("Article");
			obj.commentId = data.commentId || "";
			root.sArticleUrl = data.url || root.sArticleUrl;
			view.model = obj.blocks;
			view.positionViewAtBeginning();
		}

		function _GetComment(p)
		{
			if(commentId === "") return;

			root.bBusy = true;

			var pn;
			if(typeof(p) === "number") pn = p;
			else if(p === constants._sNextPage) pn = pageNo + 1;
			else if(p === constants._sPrevPage) pn = pageNo - 1;
			else if(p === constants._sThisPage) pn = pageNo;
			else pn = 1;
			var d = {
				aid: commentId,
				model: commentview.model,
				pageNo: pn,
				order: order,
				type: 12,
			};

			if(pn === 1) Util.ModelClear(commentview.model);

			var s = function(data){
				obj.pageNo = data.pageNo;
				obj.pageSize = data.pageSize;
				obj.pageCount = data.pageCount;
				obj.totalCount = data.totalCount;
				num = obj.totalCount;
				root.bBusy = false;
			};
			var f = function(err){
				root.bBusy = false;
				controller._ShowMessage(err);
			};

			Script.GetComment(d, s, f);
		}
	}

	Header {
		id: header;
		sText: root.sTitle;
		iTextMargin: back.width;
		onClicked: {
			obj._View(obj.input);
		}
		ToolBarLayout {
			anchors.fill: parent;
			LocalToolIcon {
				id: back;
				iconId: "toolbar-back";
				onClicked: pageStack.pop();
			}
			LocalToolIcon {
				iconId: "toolbar-view-menu";
				onClicked: mainmenu.open();
			}
		}
	}

	FlipableWidget {
		id: flip;
		anchors.top: header.bottom;
		anchors.left: parent.left;
		anchors.right: parent.right;
		anchors.bottom: parent.bottom;
		front: Item {
			id: articleitem;
			anchors.fill: parent;

			ListView {
				id: view;
				anchors.fill: parent;
				clip: true;
				model: [];
				// Larger buffer is now affordable: each delegate instantiates only
				// one lightweight block via OpusRichBlock's Loader, so pre-creating
				// a few more off-screen items smooths scrolling without the old
				// memory/CPU blow-up from the 13-way per-block Column.
				cacheBuffer: 800;
				delegate: Component {
					Item {
						width: ListView.view.width;
						height: content.height + constants._iSpacingLarge * 2;
						OpusRichBlock {
							id: content;
							anchors.top: parent.top;
							anchors.left: parent.left;
							anchors.right: parent.right;
							anchors.leftMargin: constants._iSpacingLarge;
							anchors.rightMargin: constants._iSpacingLarge;
							block: modelData;
							onOpenUrl: root.__OpenLink(url);
							onOpenImage: root.__OpenImage(url, caption);
						}
					}
				}
			}

			ScrollDecorator {
				flickableItem: view;
			}

			Rectangle {
				visible: root.sError !== "";
				anchors.fill: parent;
				color: constants._cLightestColor;
				z: 2;
				Column {
					anchors.centerIn: parent;
					width: parent.width - constants._iSpacingSuper * 4;
					spacing: constants._iSpacingBig;
					Text {
						width: parent.width;
						text: root.sError;
						font.pixelSize: constants._iFontXL;
						wrapMode: Text.WrapAnywhere;
						horizontalAlignment: Text.AlignHCenter;
						color: constants._cTextPrimary;
					}
					ButtonWidget {
						width: parent.width;
						sButtonText: qsTr("Retry");
						onClicked: obj._View(obj.input);
					}
				}
			}
		}
		back: Item {
			id: commentitem;
			anchors.fill: parent;
			TypeRowWidget {
				id: typerow;
				anchors.left: parent.left;
				anchors.right: parent.right;
				anchors.top: parent.top;
				sText: "sort";
				aOptions: [
					{
						name: qsTr("New"),
						value: "0",
					},
					{
						name: qsTr("Hot"),
						value: "2",
					},
				]
				vCurrentValue: "";
				onSelected: {
					obj.order = value;
					obj._GetComment();
				}
			}
			CommentListWidget {
				id: commentview;
				anchors.left: parent.left;
				anchors.right: parent.right;
				anchors.top: typerow.bottom;
				anchors.bottom: parent.bottom;
				bHasMore: obj.pageNo < obj.pageCount;
				aid: obj.commentId;
				type: "12";
				onRefresh: {
					obj._GetComment();
				}
				onMore: {
					obj._GetComment(constants._sNextPage);
				}
			}
		}
	}

	ContextMenu {
		id: mainmenu;
		MenuLayout {
			MenuItem {
				text: qsTr("Refresh");
				onClicked: {
					obj._View(obj.input);
				}
			}
			MenuItem {
				text: qsTr("Open original");
				enabled: root.sArticleUrl !== "";
				onClicked: {
					controller._OpenUrl(root.sArticleUrl, 1);
				}
			}
			MenuItem {
				text: qsTr("Copy url");
				enabled: root.sArticleUrl !== "";
				onClicked: {
					controller._CopyToClipboard(root.sArticleUrl);
				}
			}
			MenuItem {
				text: qsTr("Back");
				onClicked: {
					pageStack.pop();
				}
			}
		}
	}

	IconWidget {
		anchors.right: parent.right;
		anchors.bottom: parent.bottom;
		z: 1;
		opacity: 0.6;
		iconId: flip.bOpen ? "toolbar-gallery" : "toolbar-new-chat";
		inverted: true;
		enabled: obj.commentId !== "";
		visible: enabled;
		Text {
			anchors.fill: parent;
			color: constants._cDarkestColor;
			elide: Text.ElideRight;
			horizontalAlignment: Text.AlignHCenter;
			verticalAlignment: Text.AlignBottom;
			font.pixelSize: constants._iFontLarge;
			font.bold: true;
			visible: obj.num >= 0;
			text: Util.FormatCount(obj.num);
		}
		onClicked: {
			flip._Toggle();
			if(flip.bOpen)
			{
				if(obj.num < 0) obj._GetComment();
				else root.bBusy = false;
			}
		}
	}

	Rectangle {
		id: preview;
		anchors.fill: parent;
		z: constants._iMaxZ;
		color: "#cc000000";
		visible: root.previewSrc !== "";
		Image {
			anchors.fill: parent;
			anchors.margins: constants._iSpacingBig;
			fillMode: Image.PreserveAspectFit;
			source: root.previewSrc;
			clip: true;
		}
		Text {
			anchors.bottom: parent.bottom;
			anchors.bottomMargin: constants._iSpacingBig;
			anchors.horizontalCenter: parent.horizontalCenter;
			text: qsTr("Tap to close");
			color: "#ffffff";
			font.pixelSize: constants._iFontMedium;
		}
		MouseArea {
			anchors.fill: parent;
			onClicked: {
				root.previewSrc = "";
			}
		}
		LocalToolIcon {
			id: previewclose;
			anchors.top: parent.top;
			anchors.right: parent.right;
			iconId: "toolbar-close";
			inverted: true;
			onClicked: {
				root.previewSrc = "";
			}
		}
	}

	function __OpenLink(url)
	{
		var u = String(url || "");
		if(u.indexOf("ppsh://user/") === 0)
		{
			controller._OpenUserPage(u.substring(12));
			return;
		}
		if(u.indexOf("ppsh://article/") === 0)
		{
			controller._OpenArticlePage(u.substring(15));
			return;
		}
		if(/bilibili\.com\/opus\//i.test(u) || /bilibili\.com\/read\/cv/i.test(u))
		{
			controller._OpenArticlePage(u);
			return;
		}
		if(u.indexOf("bilibili.com/video/") !== -1)
		{
			var m = u.match(/BV[0-9A-Za-z]{10}/i);
			if(m)
			{
				controller._OpenDetailPage(m[0]);
				return;
			}
			var m3 = u.match(/\/av(\d+)/i);
			if(m3)
			{
				controller._OpenDetailPage(m3[1]);
				return;
			}
		}
		if(/^av\d+$/i.test(u))
		{
			controller._OpenDetailPage(u);
			return;
		}
		if(u.indexOf("space.bilibili.com/") !== -1)
		{
			var m2 = u.match(/(\d+)/);
			if(m2)
			{
				controller._OpenUserPage(m2[1]);
				return;
			}
		}
		controller._OpenUrl(u, 0);
	}

	function __OpenImage(url, caption)
	{
		if(url !== "")
			root.previewSrc = url;
	}
}
