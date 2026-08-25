.pragma library

function __R_EscapeHtml(s)
{
	return String(s === undefined || s === null ? "" : s)
		.replace(/&/g, "&amp;")
		.replace(/</g, "&lt;")
		.replace(/>/g, "&gt;")
		.replace(/"/g, "&quot;");
}

function __R_Attr(s)
{
	return __R_EscapeHtml(s).replace(/'/g, "&#39;");
}

function __R_NormalizeUrl(u)
{
	var s = String(u || "");
	if(s.indexOf("//") === 0)
		return "https:" + s;
	if(s.indexOf("http://") !== 0 && s.indexOf("https://") !== 0)
		return "";
	return s;
}

function __R_IsImageUrl(u)
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

function __R_FixImageSrc(u)
{
	var s = String(u || "");
	if(s.indexOf("//") === 0)
		return "https:" + s;
	return s;
}

// Ask bilibili's image CDN for a width-limited variant so article images decode
// and download much faster. Only applies to clean /bfs/ URLs; keeps the original
// format and aspect ratio, and skips animated GIFs (resizing would flatten them).
function __R_ThumbUrl(u)
{
	var s = String(u || "");
	if(s.indexOf("@") !== -1 || s.indexOf("?") !== -1)
		return s;
	if(s.indexOf("hdslb.com/bfs/") === -1 && s.indexOf("archive.biliimg.com/bfs/") === -1)
		return s;
	if(/\.gif$/i.test(s))
		return s;
	return s + "@1280w";
}

function __R_PrepareHtmlContent(html)
{
	var s = String(html || "");
	if(s.indexOf("<") === -1)
		return __R_EscapeHtml(s).replace(/\n/g, "<br/>");
	s = s.replace(/data-src\s*=\s*(["'])(.*?)\1/g, function(m, q, u){
		return 'src="' + __R_Attr(__R_FixImageSrc(u)) + '"';
	});
	s = s.replace(/src\s*=\s*(["'])(.*?)\1/g, function(m, q, u){
		return 'src="' + __R_Attr(__R_FixImageSrc(u)) + '"';
	});
	return s;
}

function __R_CountText(c)
{
	var n = Number(c || 0);
	if(n >= 100000000)
		return (n / 100000000).toFixed(1) + "亿";
	if(n >= 10000)
		return (n / 10000).toFixed(1) + "万";
	return String(n);
}

function __R_WordHtml(w)
{
	if(!w)
		return "";
	var s = __R_EscapeHtml(w.words !== undefined ? w.words : w.raw_text);
	if(s === "")
		return "";

	var st = w.style || {};
	var pre = "";
	var post = "";
	if(st.bold)
	{
		pre += "<b>";
		post = "</b>" + post;
	}
	if(st.italic)
	{
		pre += "<i>";
		post = "</i>" + post;
	}
	if(st.underline)
	{
		pre += "<u>";
		post = "</u>" + post;
	}
	if(st.strikethrough)
	{
		pre += "<s>";
		post = "</s>" + post;
	}

	var bgStyleColor = w.bg_style && w.bg_style.color;
	var bg = bgStyleColor ? (bgStyleColor.color || bgStyleColor.dark_color || "") : "";
	if(bg === "" && st.background)
	{
		var bgMatch = String(st.background).match(/#[0-9a-fA-F]{3,8}/);
		if(bgMatch)
			bg = bgMatch[0];
	}
	if(bg && bg.charAt(0) === "#")
	{
		pre += '<span style="background-color:' + __R_Attr(bg) + '">';
		post = "</span>" + post;
	}

	var color = w.color || (w.dark_color || "");
	if(color && color.charAt(0) === "#")
	{
		pre += '<font color="' + __R_Attr(color) + '">';
		post = "</font>" + post;
	}

	var fs = Number(w.font_size || 0);
	if(fs > 0)
	{
		pre += '<span style="font-size:' + fs + 'px">';
		post = "</span>" + post;
	}

	return pre + s + post;
}

function __R_FormulaHtml(f)
{
	if(!f || !f.latex_content)
		return "";
	var src = "https://api.bilibili.com/x/web-frontend/mathjax/tex?formula="
		+ encodeURIComponent(f.latex_content);
	return '<img src="' + src + '" width="140" height="48"/>';
}

function __R_EmojiHtml(emoji)
{
	if(!emoji)
		return "";
	var url = __R_NormalizeUrl(emoji.icon_url || emoji.webp_url || emoji.gif_url || emoji.emote_url || "");
	if(url === "")
	{
		var raw = emoji.raw_text;
		if(raw && typeof(raw) === "object")
			raw = raw.words;
		return __R_EscapeHtml(raw || "");
	}
	var size = Number(emoji.size || emoji.emoji_size || emoji.emoji_size_width || 1);
	if(emoji.emote_width && emoji.emote_width.width)
		size = emoji.emote_width.width / 18;
	var w = Math.max(16, Math.min(64, Math.round(size * 20)));
	return '<img src="' + __R_Attr(url) + '" width="' + w + '" height="' + w + '"/>';
}

function __R_RichHtml(r)
{
	if(!r)
		return "";
	var text = String(r.text || r.orig_text || r.show_text || "");
	var url = __R_NormalizeUrl(r.jump_url || r.link || r.url || "");
	var type = String(r.type || "");

	if(type === "RICH_TEXT_NODE_TYPE_EMOJI")
		return __R_EmojiHtml(r.emoji);

	if(type === "RICH_TEXT_NODE_TYPE_AT" || type === "RICH_TEXT_NODE_TYPE_USER")
	{
		if(url === "")
			url = "https://space.bilibili.com/" + (r.rid || r.mid || "");
		if(text !== "" && text.charAt(0) !== "@")
			text = "@" + text;
	}
	else if(type === "RICH_TEXT_NODE_TYPE_TOPIC")
	{
		if(url === "")
			url = "https://t.bilibili.com/topic/name/" + encodeURIComponent(text);
	}
	else if(type === "RICH_TEXT_NODE_TYPE_CV" || type === "RICH_TEXT_NODE_TYPE_OPUS" || type === "RICH_TEXT_NODE_TYPE_COMMENT")
	{
		if(url === "")
		{
			if(type === "RICH_TEXT_NODE_TYPE_CV")
				url = "https://www.bilibili.com/read/cv" + (r.rid || "") + "/";
			else if(type === "RICH_TEXT_NODE_TYPE_OPUS")
				url = "https://www.bilibili.com/opus/" + (r.rid || "");
			else
				url = "https://t.bilibili.com/" + (r.rid || "");
		}
	}
	else if(type === "RICH_TEXT_NODE_TYPE_AV" || type === "RICH_TEXT_NODE_TYPE_BV")
	{
		if(url === "" && r.rid)
		{
			if(type === "RICH_TEXT_NODE_TYPE_BV")
				url = "https://www.bilibili.com/video/" + r.rid;
			else
				url = "https://www.bilibili.com/video/av" + r.rid;
		}
	}
	else if(type.indexOf("RICH_TEXT_NODE_TYPE_OGV") === 0)
	{
		if(url === "" && r.rid)
			url = "https://www.bilibili.com/bangumi/play/ep" + r.rid;
	}
	else if(type === "RICH_TEXT_NODE_TYPE_LIVE")
	{
		if(url === "" && r.rid)
			url = "https://live.bilibili.com/" + r.rid;
	}

	if(url !== "")
		return '<a href="' + __R_Attr(url) + '">' + __R_EscapeHtml(text || url) + "</a>";
	if(text !== "")
		return __R_EscapeHtml(text);
	return "";
}

function __R_UserHtml(u)
{
	if(!u)
		return "";
	var url = "https://space.bilibili.com/" + (u.mid || "");
	return '<a href="' + url + '">' + __R_EscapeHtml(u.name || "用户") + "</a>";
}

function __R_NodeHtml(node)
{
	if(!node)
		return "";
	var type = node.type || "";
	var nt = Number(node.node_type || 0);
	if(type === "TEXT_NODE_TYPE_WORD" || nt === 1)
		return __R_WordHtml(node.word);
	if(type === "TEXT_NODE_TYPE_RICH" || nt === 4)
		return __R_RichHtml(node.rich || node.link);
	if(type === "TEXT_NODE_TYPE_FORMULA" || nt === 5)
		return __R_FormulaHtml(node.formula);
	if(type === "TEXT_NODE_TYPE_USER")
		return __R_UserHtml(node.user);
	if(nt === 2 && node.emote)
		return __R_EmojiHtml(node.emote);
	if(node.rich)
		return __R_RichHtml(node.rich);
	if(node.link)
		return __R_RichHtml(node.link);
	if(node.formula)
		return __R_FormulaHtml(node.formula);
	if(node.word)
		return __R_WordHtml(node.word);
	if(node.user)
		return __R_UserHtml(node.user);
	if(node.raw_text !== undefined)
		return __R_EscapeHtml(node.raw_text);
	return "";
}

function __R_NodesHtml(nodes)
{
	if(!Array.isArray(nodes))
		return "";
	var parts = [];
	for(var i = 0; i < nodes.length; i++)
	{
		var h = __R_NodeHtml(nodes[i]);
		if(h !== "")
			parts.push(h);
	}
	return parts.join("");
}

function __R_ChildrenHtml(children)
{
	if(!Array.isArray(children))
		return "";
	var parts = [];
	for(var i = 0; i < children.length; i++)
	{
		var c = children[i];
		var h = "";
		if(c && c.text && c.text.nodes)
			h = __R_NodesHtml(c.text.nodes);
		else if(c && typeof(c) === "string")
			h = __R_EscapeHtml(c);
		if(h !== "")
			parts.push(h);
	}
	return parts.join("<br/>");
}

function __R_HeadingLevel(p)
{
	if(p && p.heading && Number(p.heading.level) > 0)
		return Number(p.heading.level);
	if(p && p.format && Number(p.format.heading_type) > 0)
		return Number(p.format.heading_type);
	if(p && p.para_type === 1 && p.text && Array.isArray(p.text.nodes) && p.text.nodes.length > 0)
	{
		var n = p.text.nodes[0];
		var w = n && n.word;
		if(w)
		{
			var lvl = String(w.font_level || "").toLowerCase();
			if(lvl === "xxlarge")
				return 1;
			if(lvl === "xlarge")
				return 2;
			if(lvl === "large")
				return 3;
			var fs = Number(w.font_size || 0);
			if(fs >= 24)
				return 1;
			if(fs >= 22)
				return 2;
			if(fs >= 20)
				return 3;
		}
	}
	return 0;
}

function __R_MakeList(p)
{
	var list = p && p.list;
	if(!list)
		return null;
	var style = Number(list.style || 0);
	var ordered = style === 1;
	var raw = (list.items && list.items.length > 0) ? list.items : (list.children || []);
	var items = [];
	for(var i = 0; i < raw.length; i++)
	{
		var e = raw[i];
		var item = {};
		if(e && typeof(e) === "object")
		{
			item.level = Number(e.level || 1);
			item.order = e.order !== undefined ? e.order : (ordered ? i + 1 : "");
			if(e.nodes)
				item.html = __R_NodesHtml(e.nodes);
			else if(e.text && e.text.nodes)
				item.html = __R_NodesHtml(e.text.nodes);
			else if(e.children)
				item.html = __R_ChildrenHtml(e.children);
		}
		else
		{
			item.level = 1;
			item.order = ordered ? i + 1 : "";
			item.html = __R_EscapeHtml(e);
		}
		items.push(item);
	}
	return {
		kind: "list",
		ordered: ordered,
		items: items,
		html: __R_ListHtml(items, ordered),
	};
}

function __R_ListHtml(items, ordered)
{
	if(!Array.isArray(items))
		return "";
	var parts = [];
	for(var i = 0; i < items.length; i++)
	{
		var item = items[i] || {};
		var prefix = ordered ? ((item.order !== undefined ? item.order : i + 1) + ". ") : "• ";
		var indent = "";
		var level = Number(item.level || 1);
		for(var j = 1; j < level; j++)
			indent += "&nbsp;&nbsp;&nbsp;";
		parts.push(indent + prefix + (item.html || ""));
	}
	return parts.join("<br/>");
}

function __R_Stats(ms)
{
	var arr = [];
	var names = {
		like: "点赞",
		comment: "评论",
		favorite: "收藏",
		forward: "转发",
		coin: "投币",
	};
	for(var k in names)
	{
		var s = ms && ms[k];
		if(s && !s.hidden)
		{
			arr.push({
				name: names[k],
				value: s.count,
			});
		}
	}
	return arr;
}

function __R_StatsText(stats)
{
	if(!Array.isArray(stats))
		return "";
	var parts = [];
	for(var i = 0; i < stats.length; i++)
	{
		var s = stats[i] || {};
		parts.push(s.text || (s.name ? s.name + " " + __R_CountText(s.value) : ""));
	}
	return parts.join("  ");
}

function __R_FmtStat(label, value)
{
	if(value === undefined || value === null || value === "")
		return "";
	if(typeof(value) === "number")
		return label + __R_CountText(value);
	var s = String(value);
	if(/^\d+$/.test(s))
		return label + __R_CountText(Number(s));
	return s;
}

function __R_NormalizeCard(card, defaultText)
{
	var fallback = String(defaultText || "查看原文");
	var r = {
		type: "unknown",
		title: fallback,
		desc: "",
		cover: "",
		author: "",
		tag: "",
		stats: [],
		url: "",
		oid: "",
		contentHtml: "",
		duration: "",
	};
	if(!card || typeof(card) !== "object")
		return r;

	var t = String(card.type || "");

	var applyVideo = function(v)
	{
		if(!v || typeof(v) !== "object")
			return;
		if(v.title !== undefined && v.title !== "")
			r.title = String(v.title);
		r.desc = String(v.desc || v.desc_second || v.summary || "");
		r.author = String(v.author_name || v.uname || (v.owner && v.owner.name) || "");
		r.cover = __R_NormalizeUrl(v.cover || v.pic
			|| (Array.isArray(v.pics) && v.pics[0] && (v.pics[0].src || v.pics[0].url)));
		if(r.cover !== "" && !__R_IsImageUrl(r.cover))
			r.cover = "";
		r.url = __R_NormalizeUrl(v.jump_url || v.link || v.url || v.short_link || v.short_link_v2);
		r.oid = String(v.bvid || v.aid || v.id_str || v.oid || card.oid || card.biz_id || "");
		r.duration = String(v.duration_text || v.duration || (v.video_ts && v.video_ts.duration_text) || "");
		if(v.badge && typeof(v.badge) === "string")
			r.tag = v.badge;
		else if(v.badge && v.badge.text)
			r.tag = v.badge.text;
		if(Array.isArray(v.stat_infos))
		{
			for(var si = 0; si < v.stat_infos.length; si++)
			{
				if(v.stat_infos[si] && v.stat_infos[si].text)
					r.stats.push({ text: String(v.stat_infos[si].text) });
			}
		}
		else
		{
			var vs = __R_FmtStat("观看 ", v.view);
			if(vs !== "") r.stats.push({ text: vs });
			var rs = __R_FmtStat("评论 ", v.reply);
			if(rs !== "") r.stats.push({ text: rs });
			if(v.stat)
			{
				var sv = __R_FmtStat("观看 ", v.stat.view);
				if(sv !== "") r.stats.push({ text: sv });
				var sd = __R_FmtStat("弹幕 ", v.stat.danmaku);
				if(sd !== "") r.stats.push({ text: sd });
				var sr = __R_FmtStat("评论 ", v.stat.reply);
				if(sr !== "") r.stats.push({ text: sr });
			}
		}
	};

	var applyOpus = function(o)
	{
		if(!o || typeof(o) !== "object")
			return;
		if(o.title !== undefined && o.title !== "")
			r.title = String(o.title);
		r.desc = String(o.desc || o.content || o.summary || o.subtitle || "");
		r.author = String(o.author_name || (o.author && o.author.name) || o.uname || "");
		r.cover = __R_NormalizeUrl(o.cover
			|| (Array.isArray(o.pics) && o.pics[0] && (o.pics[0].src || o.pics[0].url)));
		if(r.cover !== "" && !__R_IsImageUrl(r.cover))
			r.cover = "";
		r.url = __R_NormalizeUrl(o.jump_url || o.link || o.url);
		r.oid = String(o.opus_id || o.id || o.oid || card.oid || card.biz_id || "");
		if(o.badge !== undefined && o.badge !== "")
			r.tag = String(o.badge);
		var ov = __R_FmtStat("阅读 ", o.view);
		if(ov !== "") r.stats.push({ text: ov });
		var orp = __R_FmtStat("评论 ", o.reply);
		if(orp !== "") r.stats.push({ text: orp });
		if(o.stat)
		{
			var osv = __R_FmtStat("阅读 ", o.stat.view);
			if(osv !== "") r.stats.push({ text: osv });
			var osr = __R_FmtStat("评论 ", o.stat.reply);
			if(osr !== "") r.stats.push({ text: osr });
		}
	};

	if(t === "LINK_CARD_TYPE_EVA3_VIDEO" && card.eva3_video)
	{
		r.type = "video";
		applyVideo(card.eva3_video.info || card.eva3_video);
		return r;
	}

	if(t === "LINK_CARD_TYPE_EVA3_OPUS" && card.eva3_opus)
	{
		r.type = "opus";
		r.tag = "专栏";
		applyOpus(card.eva3_opus.info || card.eva3_opus);
		return r;
	}

	if(t === "LINK_CARD_TYPE_EVA3_COMMENT" && card.eva3_comment)
	{
		var c = card.eva3_comment;
		var ci = c.info || c;
		var cu = ci.user || {};
		r.type = "comment";
		r.title = c.link_show_text || ci.link_show_text || (cu.name ? cu.name + "的评论" : fallback);
		r.desc = ci.source && ci.source.title ? ci.source.title : "";
		r.author = cu.name || ci.uname || "";
		r.cover = __R_NormalizeUrl(c.screenshot_img && (c.screenshot_img.src || c.screenshot_img.url));
		if(r.cover === "")
			r.cover = __R_NormalizeUrl(ci.cover || (Array.isArray(ci.pics) && ci.pics[0] && (ci.pics[0].src || ci.pics[0].url)));
		r.url = __R_NormalizeUrl(ci.link || c.resource_url || cu.jump_uri || "");
		r.oid = String(ci.id || card.oid || cu.mid || "");
		if(Array.isArray(ci.content && ci.content.nodes))
			r.contentHtml = __R_NodesHtml(ci.content.nodes);
		if(ci.thumbup && ci.thumbup.text)
			r.stats.push({ text: "赞 " + ci.thumbup.text });
		return r;
	}

	if(t === "LINK_CARD_TYPE_EVA3_USER" && card.eva3_user)
	{
		var u = card.eva3_user.info || card.eva3_user;
		r.type = "user";
		r.title = u.name || u.uname || fallback;
		r.desc = u.sign || u.usign || u.handle || "";
		r.cover = __R_NormalizeUrl(u.face || u.avatar);
		r.url = __R_NormalizeUrl(u.jump_uri || "https://space.bilibili.com/" + (u.mid || ""));
		r.oid = String(u.mid || card.oid || "");
		return r;
	}

	if(t === "LINK_CARD_TYPE_EVA3_VOTE" && card.eva3_vote)
	{
		var vo = card.eva3_vote.info || card.eva3_vote;
		r.type = "vote";
		r.title = vo.title || vo.desc || fallback;
		r.desc = vo.desc || "";
		r.url = __R_NormalizeUrl(vo.jump_url || vo.url);
		r.oid = String(vo.vote_id || card.oid || "");
		return r;
	}

	if(t === "LINK_CARD_TYPE_UGC" || t === "LINK_CARD_TYPE_ARCHIVE" || t === "LINK_CARD_TYPE_VIDEO")
	{
		r.type = "video";
		applyVideo(card.ugc || card.archive);
		return r;
	}

	if(t === "LINK_CARD_TYPE_OPUS" || t === "LINK_CARD_TYPE_ARTICLE" || t === "LINK_CARD_TYPE_CV")
	{
		r.type = "opus";
		r.tag = "专栏";
		applyOpus(card.opus);
		return r;
	}

	if(t === "LINK_CARD_TYPE_COMMON" && card.common)
	{
		var cm = card.common;
		r.type = "common";
		if(cm.title !== undefined && cm.title !== "")
			r.title = String(cm.title);
		r.desc = String(cm.desc || cm.subtitle || "");
		r.cover = __R_NormalizeUrl(cm.cover || cm.pic || cm.image || "");
		r.url = __R_NormalizeUrl(cm.jump_url || cm.link || cm.url);
		r.oid = String(cm.oid || card.oid || "");
		return r;
	}

	if(t === "LINK_CARD_TYPE_GOODS" && card.goods)
	{
		var gi = card.goods;
		var item = (Array.isArray(gi.items) && gi.items[0]) || gi;
		r.type = "goods";
		if(item.title !== undefined && item.title !== "")
			r.title = String(item.title);
		else if(item.name !== undefined && item.name !== "")
			r.title = String(item.name);
		r.desc = String(item.brief || item.desc || item.price || "");
		r.cover = __R_NormalizeUrl(item.cover || item.pic || item.image || "");
		r.url = __R_NormalizeUrl(item.jump_url || item.link || gi.jump_url || "");
		r.oid = String(item.id || gi.id || card.oid || "");
		r.tag = String(gi.head_text || item.jump_desc || "");
		return r;
	}

	if(t === "LINK_CARD_TYPE_ITEM_NULL")
	{
		r.type = "unknown";
		r.title = (card.item_null && card.item_null.text) || fallback;
		r.url = __R_NormalizeUrl(card.link);
		r.oid = String(card.oid || "");
		return r;
	}

	var info = card.archive || card.ugc || card.opus || card.common
		|| card.music || card.live || card.goods || card.reserve
		|| card.upower_lottery || card.match || card.vote;
	if(info)
	{
		if(info.title !== undefined && info.title !== "")
			r.title = String(info.title);
		else if(Array.isArray(info.items) && info.items[0] && info.items[0].name)
			r.title = String(info.items[0].name);
		r.desc = String(info.desc || info.desc_second || info.summary || info.content || info.subtitle || "");
		r.author = String(info.author_name || info.uname || (info.author && info.author.name) || (info.owner && info.owner.name) || "");
		r.cover = __R_NormalizeUrl(info.cover || info.pic
			|| (Array.isArray(info.pics) && info.pics[0] && (info.pics[0].src || info.pics[0].url))
			|| (Array.isArray(info.items) && info.items[0] && (info.items[0].cover || info.items[0].pic)));
		if(r.cover !== "" && !__R_IsImageUrl(r.cover))
			r.cover = "";
		r.url = __R_NormalizeUrl(info.jump_url || info.link || info.url
			|| (Array.isArray(info.items) && info.items[0] && info.items[0].jump_url));
		r.oid = String(info.bvid || info.aid || info.id_str || info.opus_id || info.id || info.oid || card.oid || card.biz_id || "");
		r.duration = String(info.duration_text || info.duration || "");
		if(info.badge && typeof(info.badge) === "string")
			r.tag = info.badge;
		else if(info.badge && info.badge.text)
			r.tag = info.badge.text;
		if(Array.isArray(info.stat_infos))
		{
			for(var si2 = 0; si2 < info.stat_infos.length; si2++)
			{
				if(info.stat_infos[si2] && info.stat_infos[si2].text)
					r.stats.push({ text: String(info.stat_infos[si2].text) });
			}
		}
		else
		{
			if(info.view !== undefined)
				r.stats.push({ text: "观看 " + __R_CountText(info.view) });
			if(info.reply !== undefined)
				r.stats.push({ text: "评论 " + __R_CountText(info.reply) });
			if(info.stat)
			{
				if(info.stat.view !== undefined)
					r.stats.push({ text: "观看 " + __R_CountText(info.stat.view) });
				if(info.stat.danmaku !== undefined)
					r.stats.push({ text: "弹幕 " + __R_CountText(info.stat.danmaku) });
				if(info.stat.reply !== undefined)
					r.stats.push({ text: "评论 " + __R_CountText(info.stat.reply) });
			}
		}
		return r;
	}

	if(card.show_text || card.link || card.link_type !== undefined || card.biz_id)
	{
		var lt = Number(card.link_type || 0);
		r.title = card.show_text || card.title || fallback;
		r.url = __R_NormalizeUrl(card.link || card.jump_url || "");
		r.oid = String(card.biz_id || card.oid || "");
		if(Array.isArray(card.pics) && card.pics.length > 0)
			r.cover = __R_NormalizeUrl(card.pics[0].url || card.pics[0].src);
		if(lt === 41)
			r.type = "video";
		else if(lt === 39)
		{
			r.type = "opus";
			r.tag = "专栏";
		}
		else if(lt === 42)
			r.type = "comment";
		else if(lt === 44)
			r.type = "user";
		else if(lt === 16)
			r.type = "web";
		else
			r.type = "unknown";
		var cci = card.content_card && card.content_card.card_info;
		if(cci && cci.content_video)
		{
			r.cid = cci.content_video.cid;
			r.duration = cci.content_video.duration_text || "";
		}
		return r;
	}

	return r;
}

function __R_NormalizeParagraphs(paragraphs)
{
	if(!Array.isArray(paragraphs))
		return [];

	var blocks = [];
	var curList = null;
	var flushList = function(){
		if(curList)
		{
			curList.html = __R_ListHtml(curList.items, curList.ordered);
			blocks.push(curList);
			curList = null;
		}
	};

	for(var i = 0; i < paragraphs.length; i++)
	{
		var p = paragraphs[i];
		if(!p)
			continue;
		var t = Number(p.para_type || 0);

		if((t === 5 || t === 6) && !p.list && !p.link_card)
		{
			var ordered = t === 5;
			var lf = (p.format && p.format.list_format) || {};
			if(!curList || curList.ordered !== ordered)
			{
				flushList();
				curList = {
					kind: "list",
					ordered: ordered,
					items: [],
					combineHash: (p.format && p.format.combine_hash) || "",
				};
			}
			if(curList.combineHash === "" && p.format && p.format.combine_hash)
				curList.combineHash = p.format.combine_hash;
			curList.items.push({
				level: Number(lf.level || 1),
				order: lf.order !== undefined ? lf.order : (ordered ? curList.items.length + 1 : ""),
				html: __R_NodesHtml(p.text && p.text.nodes),
			});
			continue;
		}

		if(curList && p.format && p.format.combine_hash
			&& curList.combineHash !== "" && p.format.combine_hash === curList.combineHash)
		{
			var clf = (p.format && p.format.list_format) || {};
			curList.items.push({
				level: Number(clf.level || 1),
				order: curList.ordered ? (clf.order !== undefined ? clf.order : curList.items.length + 1) : "",
				html: __R_NodesHtml(p.text && p.text.nodes),
			});
			continue;
		}

		flushList();

		var b = null;
		var h = __R_HeadingLevel(p);
		if(h > 0)
		{
			b = {
				kind: "heading",
				level: h,
				html: __R_NodesHtml((p.heading && p.heading.nodes) || (p.text && p.text.nodes)),
			};
		}
		else if(t === 1)
		{
			var fmt = p.format || {};
			b = {
				kind: "text",
				html: __R_NodesHtml(p.text && p.text.nodes),
				align: fmt.align !== undefined ? Number(fmt.align) : Number(p.align || 0),
				indent: (fmt.indent && fmt.indent.indent) || 0,
				firstLineIndent: (fmt.indent && fmt.indent.first_line_indent) || 0,
			};
		}
		else if(t === 2 && p.pic && Array.isArray(p.pic.pics))
		{
			for(var pi = 0; pi < p.pic.pics.length; pi++)
			{
				var pic = p.pic.pics[pi];
				if(!pic)
					continue;
				var iurl = __R_NormalizeUrl(pic.url || "");
				if(!__R_IsImageUrl(iurl))
					iurl = __R_NormalizeUrl(pic.src || "");
				if(!__R_IsImageUrl(iurl))
					iurl = __R_NormalizeUrl(pic.live_url || "");
				if(!__R_IsImageUrl(iurl))
					continue;
				blocks.push({
					kind: "image",
					url: __R_ThumbUrl(iurl),
					width: Number(pic.width || 0),
					height: Number(pic.height || 0),
					comment: pic.comment || "",
				});
			}
			continue;
		}
		else if(t === 3)
		{
			b = { kind: "divider" };
		}
		else if(t === 4)
		{
			b = {
				kind: "quote",
				html: p.blockquote && Array.isArray(p.blockquote.children)
					? __R_ChildrenHtml(p.blockquote.children)
					: __R_NodesHtml(p.text && p.text.nodes),
			};
		}
		else if((t === 6 || t === 7) && p.link_card)
		{
			var cardData = __R_NormalizeCard(p.link_card.card, p.link_card.default_text);
			b = {
				kind: "card",
				type: cardData.type,
				title: cardData.title,
				desc: cardData.desc,
				cover: cardData.cover,
				author: cardData.author,
				tag: cardData.tag,
				stats: cardData.stats,
				statsText: __R_StatsText(cardData.stats),
				url: cardData.url,
				oid: cardData.oid,
				contentHtml: cardData.contentHtml,
				duration: cardData.duration,
			};
		}
		else if(t === 5 || t === 6)
		{
			b = __R_MakeList(p);
		}
		else if((t === 7 || t === 8) && p.code)
		{
			var content = String(p.code.content || "");
			var lines = [];
			var ls = content.split("\n");
			for(var li = 0; li < ls.length; li++)
				lines.push(String(li + 1));
			b = {
				kind: "code",
				lang: p.code.lang || "",
				content: content,
				lines: lines.join("\n"),
			};
		}
		else
		{
			b = {
				kind: "unknown",
				title: "Unsupported content",
			};
		}

		if(b)
			blocks.push(b);
	}

	flushList();
	return blocks;
}

function __R_OpusPage(item)
{
	var detail = item && item.detail ? item.detail : (item || {});
	var modules = detail.modules || [];
	var title = "";
	var authorName = "";
	var authorMid = "";
	var authorFace = "";
	var pubTime = "";
	var pubTs = 0;
	var stats = [];
	var topVideo = null;
	var paragraphs = [];
	var extendItems = [];

	for(var i = 0; i < modules.length; i++)
	{
		var m = modules[i];
		if(!m)
			continue;
		var mt = m.module_type || "";
		if(mt === "MODULE_TYPE_TITLE" && m.module_title)
			title = m.module_title.text || "";
		else if(mt === "MODULE_TYPE_AUTHOR" && m.module_author)
		{
			authorName = m.module_author.name || "";
			authorMid = m.module_author.mid !== undefined ? m.module_author.mid : "";
			authorFace = __R_NormalizeUrl(m.module_author.face);
			pubTime = m.module_author.pub_time || "";
			pubTs = Number(m.module_author.pub_ts || 0);
		}
		else if(mt === "MODULE_TYPE_TOP" && m.module_top)
		{
			var tv = m.module_top.display && m.module_top.display.video
				? m.module_top.display.video
				: m.module_top.video;
			if(tv)
			{
				topVideo = {
					kind: "topvideo",
					cover: __R_NormalizeUrl(tv.cover),
					aid: tv.aid || "",
					bvid: tv.bvid || "",
					duration: tv.duration_text || tv.duration || "",
					title: tv.title || "",
				};
			}
		}
		else if(mt === "MODULE_TYPE_CONTENT" && m.module_content && Array.isArray(m.module_content.paragraphs))
			paragraphs = m.module_content.paragraphs;
		else if(mt === "MODULE_TYPE_STAT" && m.module_stat)
			stats = __R_Stats(m.module_stat);
		else if(mt === "MODULE_TYPE_EXTEND" && m.module_extend && Array.isArray(m.module_extend.items))
			extendItems = m.module_extend.items;
	}

	if(title === "" && detail.basic && detail.basic.title)
		title = String(detail.basic.title).replace(/\s*-\s*哔哩哔哩\s*$/, "");

	var id = item && (item.id_str || item.id) ? (item.id_str || item.id) : detail.id_str;
	var blocks = [];
	blocks.push({ kind: "title", text: title || "Untitled" });
	if(authorName !== "")
		blocks.push({ kind: "author", name: authorName, mid: authorMid, face: authorFace, time: pubTime, ts: pubTs });
	if(stats.length > 0)
		blocks.push({ kind: "stats", stats: stats, statsText: __R_StatsText(stats) });
	if(topVideo)
		blocks.push(topVideo);

	var contentBlocks = __R_NormalizeParagraphs(paragraphs);
	for(var j = 0; j < contentBlocks.length; j++)
		blocks.push(contentBlocks[j]);
	for(var k = 0; k < extendItems.length; k++)
	{
		var ei = extendItems[k] || {};
		var ecard = __R_NormalizeCard({
			type: "",
			show_text: ei.text || "扩展",
			link: ei.jump_url || "",
			biz_id: ei.biz_id || "",
		}, ei.text || "扩展");
		blocks.push({
			kind: "card",
			type: ecard.type,
			title: ecard.title,
			desc: ecard.desc,
			cover: ecard.cover,
			author: ecard.author,
			tag: ecard.tag,
			stats: ecard.stats,
			statsText: __R_StatsText(ecard.stats),
			url: ecard.url,
			oid: ecard.oid,
			contentHtml: ecard.contentHtml,
			duration: ecard.duration,
		});
	}

	return {
		type: "opus",
		title: title || "Untitled",
		commentId: detail.basic ? (detail.basic.comment_id_str || "") : "",
		url: "https://www.bilibili.com/opus/" + id,
		blocks: blocks,
	};
}

function __R_ReadPage(data)
{
	if(!data || typeof(data) !== "object")
		return {
			type: "read",
			title: "",
			commentId: "",
			url: "",
			blocks: [],
		};
	var ri = data && data.readInfo ? data.readInfo : (data || {});
	var opus = ri.opus || null;
	var author = ri.author || {};
	var stats = [];
	var rawStats = ri.stats || {};
	var statNames = {
		view: "观看",
		like: "点赞",
		reply: "评论",
		favorite: "收藏",
		coin: "投币",
		share: "分享",
	};
	for(var k in statNames)
	{
		if(rawStats[k] !== undefined && rawStats[k] !== null)
			stats.push({ name: statNames[k], value: rawStats[k] });
	}

	var topVideo = null;
	if(opus && opus.attachments && opus.attachments.top_content)
	{
		var top = opus.attachments.top_content;
		var tcv = top.top_content_video;
		if(tcv)
		{
			var cover = "";
			if(opus.article && Array.isArray(opus.article.cover) && opus.article.cover.length > 0)
				cover = __R_NormalizeUrl(opus.article.cover[0].url);
			if(cover === "" && Array.isArray(ri.origin_image_urls))
				cover = __R_NormalizeUrl(ri.origin_image_urls[0]);
			topVideo = {
				kind: "topvideo",
				cover: cover,
				aid: top.biz_id || ri.cover_avid || "",
				bvid: "",
				cid: tcv.cid || "",
				duration: tcv.duration_text || "",
				title: "",
			};
		}
	}

	var blocks = [];
	blocks.push({ kind: "title", text: ri.title || "Untitled" });
	if(author.name)
		blocks.push({ kind: "author", name: author.name, mid: author.mid, face: __R_NormalizeUrl(author.face), time: "", ts: Number(ri.publish_time || 0) });
	if(stats.length > 0)
		blocks.push({ kind: "stats", stats: stats, statsText: __R_StatsText(stats) });
	if(topVideo)
		blocks.push(topVideo);

	if(opus && opus.content && Array.isArray(opus.content.paragraphs))
	{
		var contentBlocks = __R_NormalizeParagraphs(opus.content.paragraphs);
		for(var j = 0; j < contentBlocks.length; j++)
			blocks.push(contentBlocks[j]);
	}
	else
	{
		blocks.push({ kind: "text", html: __R_PrepareHtmlContent(ri.content), align: 0, indent: 0, firstLineIndent: 0 });
	}

	return {
		type: "read",
		title: ri.title || "Untitled",
		commentId: ri.id !== undefined ? String(ri.id) : (data.cvid !== undefined ? String(data.cvid) : String(data.id || "")),
		url: "https://www.bilibili.com/read/cv" + (ri.id || data.cvid || data.id) + "/",
		blocks: blocks,
	};
}

function __R_ExtractInitialState(html)
{
	var s = String(html || "");
	var i = s.indexOf("__INITIAL_STATE__");
	if(i < 0)
		return null;
	i = s.indexOf("{", i);
	if(i < 0)
		return null;
	var depth = 0;
	var inStr = false;
	var esc = false;
	for(var j = i; j < s.length; j++)
	{
		var c = s.charAt(j);
		if(inStr)
		{
			if(esc)
				esc = false;
			else if(c === "\\")
				esc = true;
			else if(c === "\"")
				inStr = false;
			continue;
		}
		if(c === "\"")
			inStr = true;
		else if(c === "{")
			depth++;
		else if(c === "}")
		{
			depth--;
			if(depth === 0)
			{
				try
				{
					return JSON.parse(s.substring(i, j + 1));
				}
				catch(e)
				{
					return null;
				}
			}
		}
	}
	return null;
}

function __R_ExtractCanonicalCv(html)
{
	var s = String(html || "");
	var links = s.match(/<link[^>]+>/gi) || [];
	for(var i = 0; i < links.length; i++)
	{
		var tag = links[i];
		if(!/rel\s*=\s*["']canonical["']/i.test(tag))
			continue;
		if(!/data-vue-meta\s*=\s*["']true["']/i.test(tag))
			continue;
		var m = tag.match(/cv(\d+)/i);
		if(m)
			return m[1];
	}
	return "";
}

function __R_ParseArticleId(s)
{
	var str = String(s || "").trim();
	var m = str.match(/bilibili\.com\/opus\/(\d+)/i);
	if(m)
		return { kind: "opus", id: m[1], url: "https://www.bilibili.com/opus/" + m[1] };
	m = str.match(/bilibili\.com\/read\/cv(\d+)/i);
	if(m)
		return { kind: "read", id: m[1], url: "https://www.bilibili.com/read/cv" + m[1] + "/" };
	m = str.match(/^cv(\d+)$/i);
	if(m)
		return { kind: "read", id: m[1], url: "https://www.bilibili.com/read/cv" + m[1] + "/" };
	if(/^\d+$/.test(str))
		return { kind: "opus", id: str, url: "https://www.bilibili.com/opus/" + str };
	return null;
}

function ParseArticleId(s)
{
	return __R_ParseArticleId(s);
}