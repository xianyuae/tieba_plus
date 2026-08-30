// Shared blacklist filtering helper for TiebaLite QML (Qt Quick 1.1).
// Returns true when an item's author (by id or display name) or its
// title/abstract text matches any entry in the user or keyword blocklists.
function isBlacklisted(item, blockedUsers, blockedKeywords) {
    if (!item) return false
    var authorId = ""
    var authorName = ""
    if (item.author) {
        authorId = item.author.id || ""
        authorName = item.author.displayName || item.author.nameShow || item.author.name || ""
    }
    var j, k, u, kw
    for (j = 0; j < blockedUsers.length; j++) {
        u = blockedUsers[j]
        if (u.uid && u.uid !== "" && u.uid === authorId) return true
        if (u.name && u.name !== "" && u.name === authorName) return true
    }
    var title = item.title ? String(item.title) : ""
    var abstract = item.abstract ? String(item.abstract) : ""
    for (k = 0; k < blockedKeywords.length; k++) {
        kw = blockedKeywords[k].keyword
        if (!kw || kw === "") continue
        kw = String(kw)
        if (title.indexOf(kw) >= 0) return true
        if (abstract.indexOf(kw) >= 0) return true
        if (authorName && authorName.indexOf(kw) >= 0) return true
    }
    return false
}
