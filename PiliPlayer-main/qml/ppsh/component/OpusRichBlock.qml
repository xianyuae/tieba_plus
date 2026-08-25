import QtQuick 1.1
import "../../js/util.js" as Util

// Renders a single article/opus content block.
//
// Performance note: previously every block instantiated a Column holding one
// child per possible block kind (13 heavy sub-trees), and only toggled their
// `visible` flag. For a long article that meant hundreds of unused QML items
// being created and laid out. We now use a Loader so exactly ONE lightweight
// component is instantiated per block, selected by `block.kind`.
Item {
	id: root;

	property variant block: ({});
	signal openUrl(string url);
	signal openImage(string url, string caption);

	function __IsImageUrl(u)
	{
		var s = String(u || "").toLowerCase();
		if(s === "")
			return false;
		if(s.indexOf("hdslb.com") !== -1)
			return true;
		if(/\.(jpg|jpeg|png|gif|webp|bmp)(\?|@|$)/.test(s))
			return true;
		return false;
	}

	function __BlockComponent()
	{
		var k = String(root.block.kind || "");
		switch(k)
		{
			case "title":    return titleComp;
			case "author":   return authorComp;
			case "stats":    return statsComp;
			case "topvideo": return topvideoComp;
			case "heading":  return headingComp;
			case "text":     return textComp;
			case "image":    return imageComp;
			case "divider":  return dividerComp;
			case "quote":    return quoteComp;
			case "list":     return listComp;
			case "code":     return codeComp;
			case "card":     return cardComp;
			case "unknown":  return unknownComp;
			default:         return emptyComp;
		}
	}

	width: parent.width;
	height: loader.height;

	Loader {
		id: loader;
		anchors.left: parent.left;
		anchors.right: parent.right;
		sourceComponent: root.__BlockComponent();
	}

	Component {
		id: emptyComp;
		Item {
			width: parent.width;
			height: 0;
		}
	}

	Component {
		id: titleComp;
		Text {
			width: parent.width;
			height: implicitHeight + 8;
			text: root.block.text || "";
			textFormat: Text.PlainText;
			wrapMode: Text.WrapAnywhere;
			color: constants._cTextPrimary;
			font.pixelSize: constants._iFontXXXL;
			font.bold: true;
			onLinkActivated: root.openUrl(link);
		}
	}

	Component {
		id: authorComp;
		Item {
			width: parent.width;
			height: constants._iSizeXXL;

			Row {
				width: parent.width;
				height: parent.height;
				spacing: constants._iSpacingLarge;

				Image {
					id: authorface;
					width: constants._iSizeXL;
					height: width;
					source: root.block.face || "";
					fillMode: Image.PreserveAspectCrop;
					clip: true;
					sourceSize: Qt.size(width, height);
					MouseArea {
						anchors.fill: parent;
						onClicked: {
							if(root.block.mid)
								root.openUrl("https://space.bilibili.com/" + root.block.mid);
						}
					}
				}

				Column {
					width: parent.width - authorface.width - parent.spacing;
					height: parent.height;
					Text {
						width: parent.width;
						text: root.block.name || "";
						font.pixelSize: constants._iFontLarge;
						font.bold: true;
						elide: Text.ElideRight;
						color: constants._cTextPrimary;
					}
					Text {
						width: parent.width;
						text: (root.block.time || "") !== "" ? root.block.time : (root.block.ts > 0 ? Util.FormatDateTime(root.block.ts, "DATETIME") : "");
						font.pixelSize: constants._iFontMedium;
						elide: Text.ElideRight;
						color: constants._cTextSecondary;
					}
				}
			}

			MouseArea {
				anchors.fill: parent;
				onClicked: {
					if(root.block.mid)
						root.openUrl("https://space.bilibili.com/" + root.block.mid);
				}
			}
		}
	}

	Component {
		id: statsComp;
		Row {
			width: parent.width;
			height: constants._iSizeLarge;
			spacing: constants._iSpacingXL;

			Text {
				height: parent.height;
				verticalAlignment: Text.AlignVCenter;
				text: root.block.statsText || "";
				font.pixelSize: constants._iFontLarge;
				color: constants._cTextSecondary;
			}
		}
	}

	Component {
		id: topvideoComp;
		Rectangle {
			width: parent.width;
			height: constants._iSizeBig + constants._iSpacingLarge * 2;
			radius: constants._iRadiusMedium;
			color: constants._cCardColor;
			border.width: 1;
			border.color: constants._cDividerColor;

			Row {
				id: topvideoRow;
				anchors.top: parent.top;
				anchors.left: parent.left;
				anchors.right: parent.right;
				anchors.topMargin: constants._iSpacingLarge;
				anchors.leftMargin: constants._iSpacingLarge;
				anchors.rightMargin: constants._iSpacingLarge;
				height: constants._iSizeBig;
				spacing: constants._iSpacingLarge;

				Image {
					id: topcover;
					width: constants._iSizeBig;
					height: parent.height;
					source: root.__IsImageUrl(root.block.cover) ? root.block.cover : "";
					fillMode: Image.PreserveAspectCrop;
					clip: true;
					MouseArea {
						anchors.fill: parent;
						onClicked: root.openImage(root.block.cover, "");
					}
				}

				Column {
					width: parent.width - topcover.width - parent.spacing;
					height: parent.height;
					spacing: constants._iSpacingSmall;
					Text {
						width: parent.width;
						text: root.block.title || (root.block.bvid || (root.block.aid ? "av" + root.block.aid : ""));
						font.pixelSize: constants._iFontLarge;
						font.bold: true;
						elide: Text.ElideRight;
						maximumLineCount: 2;
						wrapMode: Text.WrapAnywhere;
						color: constants._cTextPrimary;
					}
					Text {
						width: parent.width;
						text: root.block.bvid || (root.block.aid ? "av" + root.block.aid : "");
						font.pixelSize: constants._iFontMedium;
						elide: Text.ElideRight;
						color: constants._cTextSecondary;
					}
					Text {
						width: parent.width;
						text: root.block.duration || qsTr("Video");
						font.pixelSize: constants._iFontMedium;
						color: constants._cTextHint;
					}
				}
			}

			MouseArea {
				anchors.fill: parent;
				onClicked: {
					var u = root.block.bvid
						? "https://www.bilibili.com/video/" + root.block.bvid
						: (root.block.aid ? "https://www.bilibili.com/video/av" + root.block.aid : "");
					if(u !== "")
						root.openUrl(u);
				}
			}
		}
	}

	Component {
		id: headingComp;
		Text {
			width: parent.width;
			height: implicitHeight + 8;
			text: root.block.html || "";
			textFormat: Text.RichText;
			wrapMode: Text.WrapAnywhere;
			color: constants._cTextPrimary;
			font.pixelSize: root.block.level === 1 ? constants._iFontXXXL : (root.block.level === 2 ? constants._iFontXXL : constants._iFontXL);
			font.bold: true;
			onLinkActivated: root.openUrl(link);
		}
	}

	Component {
		id: textComp;
		Text {
			width: parent.width - (root.block.indent ? root.block.indent * 20 : 0) - (root.block.firstLineIndent ? root.block.firstLineIndent * 20 : 0);
			x: (root.block.indent ? root.block.indent * 20 : 0) + (root.block.firstLineIndent ? root.block.firstLineIndent * 20 : 0);
			height: implicitHeight + 8;
			text: root.block.html || "";
			textFormat: Text.RichText;
			wrapMode: Text.WrapAnywhere;
			horizontalAlignment: root.block.align === 1 ? Text.AlignHCenter : (root.block.align === 2 ? Text.AlignRight : Text.AlignLeft);
			color: constants._cTextPrimary;
			onLinkActivated: root.openUrl(link);
		}
	}

	Component {
		id: imageComp;
		Image {
			width: parent.width;
			height: (root.block.width > 0 && root.block.height > 0
				? Math.min(parent.width * root.block.height / root.block.width, 520)
				: parent.width * 0.62);
			source: root.__IsImageUrl(root.block.url) ? root.block.url : "";
			onStatusChanged: {
				if(status === Image.Error)
					source = "";
			}
			fillMode: Image.PreserveAspectFit;
			clip: true;
			cache: true;
			sourceSize.width: width;
			MouseArea {
				anchors.fill: parent;
				onClicked: root.openImage(root.block.url, root.block.comment);
			}
		}
	}

	Component {
		id: dividerComp;
		Rectangle {
			width: parent.width;
			height: 1;
			color: constants._cDividerColor;
		}
	}

	Component {
		id: quoteComp;
		Rectangle {
			width: parent.width;
			height: quoteedit.height + constants._iSpacingBig * 2;
			radius: constants._iRadiusSmall;
			color: constants._cLighterColor;
			border.width: 1;
			border.color: constants._cDividerColor;

			Text {
				id: quoteedit;
				anchors.top: parent.top;
				anchors.left: parent.left;
				anchors.right: parent.right;
				anchors.topMargin: constants._iSpacingLarge;
				anchors.leftMargin: constants._iSpacingLarge;
				anchors.rightMargin: constants._iSpacingLarge;
				height: implicitHeight + 8;
				text: root.block.html || "";
				textFormat: Text.RichText;
				wrapMode: Text.WrapAnywhere;
				color: constants._cTextSecondary;
				onLinkActivated: root.openUrl(link);
			}
		}
	}

	Component {
		id: listComp;
		Text {
			width: parent.width;
			height: implicitHeight + 8;
			text: root.block.html || "";
			textFormat: Text.RichText;
			wrapMode: Text.WrapAnywhere;
			color: constants._cTextPrimary;
			onLinkActivated: root.openUrl(link);
		}
	}

	Component {
		id: codeComp;
		Rectangle {
			width: parent.width;
			height: codeheader.height + coderow.height + constants._iSpacingMedium * 2;
			radius: constants._iRadiusSmall;
			color: constants._bInverted ? "#1c1d22" : "#f2f3f5";
			border.width: 1;
			border.color: constants._cDividerColor;

			Column {
				anchors.top: parent.top;
				anchors.left: parent.left;
				anchors.right: parent.right;
				anchors.topMargin: constants._iSpacingMedium;
				anchors.leftMargin: constants._iSpacingMedium;
				anchors.rightMargin: constants._iSpacingMedium;
				Row {
					id: codeheader;
					width: parent.width;
					height: constants._iSizeSmall;
					Text {
						width: parent.width - copycode.width;
						height: parent.height;
						verticalAlignment: Text.AlignVCenter;
						text: root.block.lang || "code";
						font.pixelSize: constants._iFontMedium;
						color: constants._cTextSecondary;
					}
					Text {
						id: copycode;
						height: parent.height;
						verticalAlignment: Text.AlignVCenter;
						text: qsTr("Copy");
						font.pixelSize: constants._iFontMedium;
						color: constants._cGlobalColor;
						MouseArea {
							anchors.fill: parent;
							onClicked: controller._CopyToClipboard(root.block.content);
						}
					}
				}
				Row {
					id: coderow;
					width: parent.width;
					height: Math.max(linenumbers.height, codebody.implicitHeight + 8);
					Text {
						id: linenumbers;
						width: 34;
						height: codebody.implicitHeight + 8;
						text: root.block.lines || "";
						font.family: "monospace";
						font.pixelSize: constants._iFontMedium;
						horizontalAlignment: Text.AlignRight;
						color: constants._cTextHint;
					}
					Text {
						id: codebody;
						width: parent.width - linenumbers.width - constants._iSpacingMedium;
						x: constants._iSpacingMedium;
						height: implicitHeight + 8;
						text: root.block.content || "";
						textFormat: Text.PlainText;
						wrapMode: Text.WrapAnywhere;
						font.family: "monospace";
						font.pixelSize: constants._iFontMedium;
						color: constants._cTextPrimary;
					}
				}
			}
		}
	}

	Component {
		id: cardComp;
		Rectangle {
			width: parent.width;
			height: cardlayout.height + constants._iSpacingBig * 2;
			radius: constants._iRadiusMedium;
			color: constants._cCardColor;
			border.width: 1;
			border.color: constants._cDividerColor;

			Column {
				id: cardlayout;
				anchors.top: parent.top;
				anchors.left: parent.left;
				anchors.right: parent.right;
				anchors.topMargin: constants._iSpacingLarge;
				anchors.leftMargin: constants._iSpacingLarge;
				anchors.rightMargin: constants._iSpacingLarge;
				height: implicitHeight;
				spacing: constants._iSpacingSmall;

				Row {
					visible: (root.block.cover || "") !== "";
					width: parent.width;
					height: visible ? constants._iSizeBig : 0;
					Image {
						id: cardcover;
						width: parent.width;
						height: parent.height;
						source: root.__IsImageUrl(root.block.cover) ? root.block.cover : "";
						onStatusChanged: {
							if(status === Image.Error)
								source = "";
						}
						fillMode: Image.PreserveAspectCrop;
						clip: true;
						MouseArea {
							anchors.fill: parent;
							onClicked: root.openImage(root.block.cover, "");
						}
					}
				}

				Column {
					width: parent.width;
					spacing: constants._iSpacingTiny;
					Text {
						width: parent.width;
						text: root.block.title || "";
						font.pixelSize: constants._iFontLarge;
						font.bold: true;
						wrapMode: Text.WrapAnywhere;
						maximumLineCount: 3;
						elide: Text.ElideRight;
						color: constants._cTextPrimary;
					}
					Text {
						width: parent.width;
						visible: (root.block.desc || "") !== "";
						text: root.block.desc || "";
						font.pixelSize: constants._iFontMedium;
						wrapMode: Text.WrapAnywhere;
						maximumLineCount: 3;
						elide: Text.ElideRight;
						color: constants._cTextSecondary;
					}
				}

				Text {
					visible: (root.block.contentHtml || "") !== "";
					width: parent.width;
					height: visible ? implicitHeight + 8 : 0;
					text: root.block.contentHtml || "";
					textFormat: Text.RichText;
					wrapMode: Text.WrapAnywhere;
					color: constants._cTextPrimary;
					onLinkActivated: root.openUrl(link);
				}

				Row {
					visible: (root.block.author || "") !== "" || (root.block.tag || "") !== "" || (root.block.duration || "") !== "";
					width: parent.width;
					spacing: constants._iSpacingXL;
					Text {
						text: root.block.tag || "";
						visible: (root.block.tag || "") !== "";
						font.pixelSize: constants._iFontMedium;
						color: constants._cGlobalColor;
					}
					Text {
						text: root.block.author || "";
						visible: (root.block.author || "") !== "";
						width: parent.width * 0.5;
						elide: Text.ElideRight;
						font.pixelSize: constants._iFontMedium;
						color: constants._cTextSecondary;
					}
					Text {
						text: root.block.duration || "";
						visible: (root.block.duration || "") !== "";
						font.pixelSize: constants._iFontMedium;
						color: constants._cTextHint;
					}
				}

				Row {
					visible: (root.block.stats || []).length > 0;
					width: parent.width;
					Text {
						width: parent.width;
						text: root.block.statsText || "";
						font.pixelSize: constants._iFontMedium;
						wrapMode: Text.WrapAnywhere;
						color: constants._cTextSecondary;
					}
				}
			}

			MouseArea {
				anchors.fill: parent;
				onClicked: {
					if((root.block.url || "") !== "")
						root.openUrl(root.block.url);
				}
			}
		}
	}

	Component {
		id: unknownComp;
		Rectangle {
			width: parent.width;
			height: constants._iSizeXL;
			radius: constants._iRadiusSmall;
			color: constants._cLighterColor;
			border.width: 1;
			border.color: constants._cDividerColor;
			Text {
				anchors.fill: parent;
				anchors.margins: constants._iSpacingMedium;
				horizontalAlignment: Text.AlignHCenter;
				verticalAlignment: Text.AlignVCenter;
				text: root.block.title || qsTr("Unsupported content");
				font.pixelSize: constants._iFontLarge;
				wrapMode: Text.WrapAnywhere;
				color: constants._cTextHint;
			}
		}
	}
}
