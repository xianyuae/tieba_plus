// Shared display helpers for TiebaLite QML (Qt Quick 1.1).
// Content parsing itself happens in C++; these only build display strings.

function escapeHtml(s) {
    if (!s) return "";
    s = s.replace(/&/g, "&amp;");
    s = s.replace(/</g, "&lt;");
    s = s.replace(/>/g, "&gt;");
    s = s.replace(/\n/g, "<br/>");
    return s;
}

function escapeAttr(s) {
    if (!s) return "";
    return String(s).replace(/&/g, "&amp;").replace(/"/g, "&quot;")
                    .replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

// Builds StyledText HTML for inline fragments (text/link/@/emoticon-as-name).
// Block images / video / voice are extracted separately.
function toHtml(frags, accent) {
    var out = "";
    for (var i = 0; i < frags.length; i++) {
        var f = frags[i];
        var t = f.type;
        if (t === 0 || t === 9 || t === 27) {
            out += escapeHtml(f.text);
        } else if (t === 1) {
            out += '<a href="' + escapeAttr(f.link) + '">' + escapeHtml(f.text) + '</a>';
        } else if (t === 2) {
            out += '<font color="' + accent + '">[' + escapeHtml(f.text) + ']</font>';
        } else if (t === 4) {
            out += '<font color="' + accent + '">' + escapeHtml(f.text) + '</font>';
        }
    }
    return out;
}

function extractImages(frags) {
    var out = [];
    for (var i = 0; i < frags.length; i++) {
        var t = frags[i].type;
        if (t === 3 || t === 20) out.push(frags[i]);
    }
    return out;
}

function extractVideos(frags) {
    var out = [];
    for (var i = 0; i < frags.length; i++) {
        if (frags[i].type === 5) out.push(frags[i]);
    }
    return out;
}

function extractVoices(frags) {
    var out = [];
    for (var i = 0; i < frags.length; i++) {
        if (frags[i].type === 10) out.push(frags[i]);
    }
    return out;
}

// Height/width ratio of an image fragment (for aspect-ratio layout).
function imageAspect(f) {    var w = 0, h = 0;
    if (f.width && f.height) { w = f.width; h = f.height; }
    else if (f.bsize) {
        var parts = f.bsize.split(",");
        if (parts.length === 2) { w = parseInt(parts[0], 10); h = parseInt(parts[1], 10); }
    }
    if (w > 0 && h > 0) return h / w;
    return 0.62;
}

// Unified thread-list time in seconds (proto vs JSON shapes).
function threadTimeSeconds(t) {
    if (t.lastTimeInt) return t.lastTimeInt;
    if (t.createTime) return t.createTime;
    if (t.time) return parseInt(t.time, 10);
    return 0;
}

// Resolve a portrait hash (or passthrough an absolute URL).
function avatarUrl(p) {
    if (!p) return "";
    if (p.indexOf("http") === 0) return p;
    return "http://tb.himg.baidu.com/sys/portrait/item/" + p;
}
