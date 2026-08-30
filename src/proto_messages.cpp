#include "proto_messages.h"
#include "pbwire.h"
#include "content.h"
#include "clientinfo.h"
#include "accountmanager.h"
#include "signutil.h"
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QStringList>

namespace {

using namespace Proto;

static QString idStr(quint64 v) { return QString::number(v); }

// ---- shared parsers ----

QVariantMap parseError(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("errno"), qlonglong(r.varint()));
        else if (f == 2) m.insert(QLatin1String("errmsg"), r.string());
        else if (f == 3) m.insert(QLatin1String("usermsg"), r.string());
        else r.skipField();
    }
    return m;
}

QVariantMap parseUser(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 2: m.insert(QLatin1String("id"), idStr(r.varint())); break;
        case 3: m.insert(QLatin1String("name"), r.string()); break;
        case 4: m.insert(QLatin1String("nameShow"), r.string()); break;
        case 5: m.insert(QLatin1String("portrait"), r.string()); break;
        case 7: m.insert(QLatin1String("type"), (int)r.varint()); break;
        case 11: m.insert(QLatin1String("isManager"), (int)r.varint()); break;
        case 23: m.insert(QLatin1String("levelId"), (int)r.varint()); break;
        case 25: m.insert(QLatin1String("isBawu"), (int)r.varint()); break;
        case 26: m.insert(QLatin1String("bawuType"), r.string()); break;
        case 27: m.insert(QLatin1String("portraith"), r.string()); break;
        case 32: m.insert(QLatin1String("sex"), (int)r.varint()); break;
        case 34: m.insert(QLatin1String("intro"), r.string()); break;
        case 37: m.insert(QLatin1String("postNum"), (int)r.varint()); break;
        case 38: m.insert(QLatin1String("tbAge"), r.string()); break;
        case 87: m.insert(QLatin1String("threadNum"), (int)r.varint()); break;
        case 127: m.insert(QLatin1String("ip"), r.string()); break;
        case 125: m.insert(QLatin1String("levelName"), r.string()); break;
        default: r.skipField(); break;
        }
    }
    QString nameShow = m.value(QLatin1String("nameShow")).toString();
    QString name = m.value(QLatin1String("name")).toString();
    m.insert(QLatin1String("displayName"), nameShow.isEmpty() ? name : nameShow);
    m.insert(QLatin1String("avatar"), Content::avatarUrl(m.value(QLatin1String("portrait")).toString()));
    return m;
}

QVariantMap parsePbContent(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("type"), (int)r.varint()); break;
        case 2: m.insert(QLatin1String("text"), r.string()); break;
        case 3: m.insert(QLatin1String("link"), r.string()); break;
        case 4: m.insert(QLatin1String("src"), r.string()); break;
        case 5: m.insert(QLatin1String("bsize"), r.string()); break;
        case 6: m.insert(QLatin1String("bigSrc"), r.string()); break;
        case 7: m.insert(QLatin1String("bigSize"), r.string()); break;
        case 8: m.insert(QLatin1String("cdnSrc"), r.string()); break;
        case 9: m.insert(QLatin1String("bigCdnSrc"), r.string()); break;
        case 10: m.insert(QLatin1String("imgType"), r.string()); break;
        case 11: m.insert(QLatin1String("c"), r.string()); break;
        case 12: m.insert(QLatin1String("voiceMD5"), r.string()); break;
        case 13: m.insert(QLatin1String("duringTime"), (quint32)r.varint()); break;
        case 15: m.insert(QLatin1String("uid"), idStr(r.varint())); break;
        case 16: m.insert(QLatin1String("dynamic"), r.string()); break;
        case 17: m.insert(QLatin1String("static"), r.string()); break;
        case 18: m.insert(QLatin1String("width"), (quint32)r.varint()); break;
        case 19: m.insert(QLatin1String("height"), (quint32)r.varint()); break;
        case 25: m.insert(QLatin1String("originSrc"), r.string()); break;
        case 27: m.insert(QLatin1String("originSize"), (quint32)r.varint()); break;
        case 34: m.insert(QLatin1String("isLongPic"), (quint32)r.varint()); break;
        case 35: m.insert(QLatin1String("showOriginalBtn"), (quint32)r.varint()); break;
        case 36: m.insert(QLatin1String("cdnSrcActive"), r.string()); break;
        default: r.skipField(); break;
        }
    }
    Content::finalizeFragment(m);
    return m;
}

QVariantMap parseAgree(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("agreeNum"), qlonglong(r.varint()));
        else if (f == 2) m.insert(QLatin1String("hasAgree"), (int)r.varint());
        else if (f == 4) m.insert(QLatin1String("disagreeNum"), qlonglong(r.varint()));
        else if (f == 5) m.insert(QLatin1String("diffAgreeNum"), qlonglong(r.varint()));
        else r.skipField();
    }
    return m;
}

QVariantMap parseMedia(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("type"), (int)r.varint()); break;
        case 3: m.insert(QLatin1String("bigPic"), r.string()); break;
        case 8: m.insert(QLatin1String("srcPic"), r.string()); break;
        case 10: m.insert(QLatin1String("width"), (quint32)r.varint()); break;
        case 11: m.insert(QLatin1String("height"), (quint32)r.varint()); break;
        case 15: m.insert(QLatin1String("originPic"), r.string()); break;
        case 18: m.insert(QLatin1String("dynamicPic"), r.string()); break;
        case 19: m.insert(QLatin1String("isLongPic"), (quint32)r.varint()); break;
        case 20: m.insert(QLatin1String("showOriginalBtn"), (quint32)r.varint()); break;
        default: r.skipField(); break;
        }
    }
    QStringList thumbs;
    thumbs << m.value(QLatin1String("bigPic")).toString()
           << m.value(QLatin1String("dynamicPic")).toString()
           << m.value(QLatin1String("srcPic")).toString()
           << m.value(QLatin1String("originPic")).toString();
    QString thumb;
    for (int i = 0; i < thumbs.size(); ++i)
        if (!thumbs.at(i).isEmpty()) { thumb = thumbs.at(i); break; }
    m.insert(QLatin1String("thumb"), thumb);
    m.insert(QLatin1String("original"), m.value(QLatin1String("originPic")).toString());
    return m;
}

QVariantMap parseVideoInfo(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("videoMD5"), r.string());
        else if (f == 2) m.insert(QLatin1String("videoUrl"), r.string());
        else if (f == 3) m.insert(QLatin1String("duration"), (quint32)r.varint());
        else if (f == 4) m.insert(QLatin1String("width"), (quint32)r.varint());
        else if (f == 5) m.insert(QLatin1String("height"), (quint32)r.varint());
        else if (f == 6) m.insert(QLatin1String("thumbnailUrl"), r.string());
        else if (f == 7) m.insert(QLatin1String("thumbnailWidth"), (quint32)r.varint());
        else if (f == 8) m.insert(QLatin1String("thumbnailHeight"), (quint32)r.varint());
        else r.skipField();
    }
    return m;
}

QVariantMap parseAbstract(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("type"), (int)r.varint());
        else if (f == 2) m.insert(QLatin1String("text"), r.string());
        else if (f == 3) m.insert(QLatin1String("link"), r.string());
        else if (f == 4) m.insert(QLatin1String("src"), r.string());
        else if (f == 5) m.insert(QLatin1String("un"), r.string());
        else if (f == 6) m.insert(QLatin1String("duringTime"), r.string());
        else if (f == 7) m.insert(QLatin1String("voiceMD5"), r.string());
        else r.skipField();
    }
    return m;
}

QVariantMap parseOriginThreadInfo(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList media, abstract, content;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("title"), r.string()); break;
        case 2: media << parseMedia(r.message()); break;
        case 3: abstract << parseAbstract(r.message()); break;
        case 4: m.insert(QLatin1String("fname"), r.string()); break;
        case 5: m.insert(QLatin1String("tid"), idStr(r.varint())); break;
        case 7: m.insert(QLatin1String("fid"), idStr(r.varint())); break;
        case 13: m.insert(QLatin1String("video"), parseVideoInfo(r.message())); break;
        case 14: content << parsePbContent(r.message()); break;
        case 16: m.insert(QLatin1String("replyNum"), (int)r.varint()); break;
        case 18: m.insert(QLatin1String("author"), parseUser(r.message())); break;
        case 19: m.insert(QLatin1String("agree"), parseAgree(r.message())); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("media"), media);
    m.insert(QLatin1String("abstract"), Content::joinAbstract(abstract));
    m.insert(QLatin1String("content"), content);
    return m;
}

QVariantMap parseSimpleForum(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("id"), idStr(r.varint()));
        else if (f == 2) m.insert(QLatin1String("name"), r.string());
        else if (f == 4) m.insert(QLatin1String("avatar"), r.string());
        else if (f == 12) m.insert(QLatin1String("memberNum"), (int)r.varint());
        else if (f == 13) m.insert(QLatin1String("postNum"), (int)r.varint());
        else r.skipField();
    }
    return m;
}

QVariantMap parseSubPostList(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList content;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("id"), idStr(r.varint())); break;
        case 2: content << parsePbContent(r.message()); break;
        case 3: m.insert(QLatin1String("time"), qlonglong(r.varint())); break;
        case 4: m.insert(QLatin1String("authorId"), idStr(r.varint())); break;
        case 5: m.insert(QLatin1String("title"), r.string()); break;
        case 6: m.insert(QLatin1String("floor"), (quint32)r.varint()); break;
        case 7: m.insert(QLatin1String("author"), parseUser(r.message())); break;
        case 9: m.insert(QLatin1String("agree"), parseAgree(r.message())); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("content"), content);
    return m;
}

QVariantMap parsePost(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList content;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("id"), idStr(r.varint())); break;
        case 2: m.insert(QLatin1String("title"), r.string()); break;
        case 3: m.insert(QLatin1String("floor"), (quint32)r.varint()); break;
        case 4: m.insert(QLatin1String("time"), qlonglong(r.varint())); break;
        case 5: content << parsePbContent(r.message()); break;
        case 9: m.insert(QLatin1String("isVoice"), (quint32)r.varint()); break;
        case 10: m.insert(QLatin1String("isNTitle"), (quint32)r.varint()); break;
        case 13: m.insert(QLatin1String("subPostNumber"), (quint32)r.varint()); break;
        case 15: {
            PbReader sr(r.message());
            QVariantList subs;
            int sf;
            while ((sf = sr.next()) != 0) {
                if (sf == 2) subs << parseSubPostList(sr.message());
                else sr.skipField();
            }
            m.insert(QLatin1String("subPostList"), subs);
            break;
        }
        case 19: m.insert(QLatin1String("authorId"), idStr(r.varint())); break;
        case 23: m.insert(QLatin1String("author"), parseUser(r.message())); break;
        case 24: r.skipField(); break; // Zan (message) — like info comes from agree (37)
        case 29: m.insert(QLatin1String("video"), parseVideoInfo(r.message())); break;
        case 37: m.insert(QLatin1String("agree"), parseAgree(r.message())); break;
        case 38: m.insert(QLatin1String("fromForum"), parseSimpleForum(r.message())); break;
        case 42: m.insert(QLatin1String("originThread"), parseOriginThreadInfo(r.message())); break;
        case 46: m.insert(QLatin1String("tid"), idStr(r.varint())); break;
        case 50: m.insert(QLatin1String("quoteId"), r.string()); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("content"), content);
    return m;
}

QVariantMap parseThreadInfo(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList abstract, media;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 2: m.insert(QLatin1String("tid"), idStr(r.varint())); break;
        case 3: m.insert(QLatin1String("title"), r.string()); break;
        case 4: m.insert(QLatin1String("replyNum"), (int)r.varint()); break;
        case 5: m.insert(QLatin1String("viewNum"), (int)r.varint()); break;
        case 7: m.insert(QLatin1String("lastTimeInt"), (int)r.varint()); break;
        case 8: m.insert(QLatin1String("threadTypes"), (int)r.varint()); break;
        case 9: m.insert(QLatin1String("isTop"), (int)r.varint()); break;
        case 10: m.insert(QLatin1String("isGood"), (int)r.varint()); break;
        case 18: m.insert(QLatin1String("author"), parseUser(r.message())); break;
        case 21: abstract << parseAbstract(r.message()); break;
        case 22: media << parseMedia(r.message()); break;
        case 27: m.insert(QLatin1String("forumId"), idStr(r.varint())); break;
        case 28: m.insert(QLatin1String("forumName"), r.string()); break;
        case 38: m.insert(QLatin1String("isNoTitle"), (int)r.varint()); break;
        case 45: m.insert(QLatin1String("createTime"), (int)r.varint()); break;
        case 79: m.insert(QLatin1String("video"), parseVideoInfo(r.message())); break;
        case 124: m.insert(QLatin1String("agreeNum"), (int)r.varint()); break;
        case 135: m.insert(QLatin1String("shareNum"), (int)r.varint()); break;
        case 141: m.insert(QLatin1String("originThread"), parseOriginThreadInfo(r.message())); break;
        case 155: m.insert(QLatin1String("forumInfo"), parseSimpleForum(r.message())); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("abstract"), Content::joinAbstract(abstract));
    m.insert(QLatin1String("media"), media);
    return m;
}

QVariantMap parsePage(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("pageSize"), (int)r.varint());
        else if (f == 2) m.insert(QLatin1String("offset"), (int)r.varint());
        else if (f == 3) m.insert(QLatin1String("currentPage"), (int)r.varint());
        else if (f == 4) m.insert(QLatin1String("totalCount"), (int)r.varint());
        else if (f == 5) m.insert(QLatin1String("totalPage"), (int)r.varint());
        else if (f == 6) m.insert(QLatin1String("hasMore"), (int)r.varint());
        else if (f == 7) m.insert(QLatin1String("hasPrev"), (int)r.varint());
        else r.skipField();
    }
    return m;
}

QVariantMap parseAnti(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("tbs"), r.string());
        else r.skipField();
    }
    return m;
}

namespace {

// Server-side user objects often omit the embedded User message; authors are
// then only referenced by id and must be joined from the response's user_list.
QVariantMap joinAuthor(const QVariantMap &item, const QVariantList &userList)
{
    QVariantMap m = item;
    const QVariantMap author = m.value(QLatin1String("author")).toMap();
    if (!author.isEmpty() && !author.value(QLatin1String("id")).toString().isEmpty())
        return m;
    const QString want = m.value(QLatin1String("authorId")).toString();
    if (want.isEmpty())
        return m;
    for (int i = 0; i < userList.size(); ++i) {
        const QVariantMap u = userList.at(i).toMap();
        if (u.value(QLatin1String("id")).toString() == want) {
            m.insert(QLatin1String("author"), u);
            return m;
        }
    }
    return m;
}

} // namespace

// FrsPage ForumInfo (package tieba.frsPage)
QVariantMap parseFrsForumInfo(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("id"), idStr(r.varint())); break;
        case 2: m.insert(QLatin1String("name"), r.string()); break;
        case 6: m.insert(QLatin1String("isLike"), (int)r.varint()); break;
        case 7: m.insert(QLatin1String("userLevel"), (int)r.varint()); break;
        case 8: m.insert(QLatin1String("levelName"), r.string()); break;
        case 9: m.insert(QLatin1String("memberNum"), (int)r.varint()); break;
        case 10: m.insert(QLatin1String("threadNum"), (int)r.varint()); break;
        case 11: m.insert(QLatin1String("postNum"), (int)r.varint()); break;
        case 24: m.insert(QLatin1String("avatar"), r.string()); break;
        case 25: m.insert(QLatin1String("slogan"), r.string()); break;
        default: r.skipField(); break;
        }
    }
    return m;
}

QVariantMap parseFrsPageData(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList threads;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("user"), parseUser(r.message())); break;
        case 2: m.insert(QLatin1String("forum"), parseFrsForumInfo(r.message())); break;
        case 4: m.insert(QLatin1String("page"), parsePage(r.message())); break;
        case 5: m.insert(QLatin1String("anti"), parseAnti(r.message())); break;
        case 7: threads << parseThreadInfo(r.message()); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("threads"), threads);
    return m;
}

QVariantMap parsePbPageData(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList posts;
    QVariantList userList;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("user"), parseUser(r.message())); break;
        case 2: m.insert(QLatin1String("forum"), parseSimpleForum(r.message())); break;
        case 3: m.insert(QLatin1String("page"), parsePage(r.message())); break;
        case 4: m.insert(QLatin1String("anti"), parseAnti(r.message())); break;
        case 6: posts << parsePost(r.message()); break;
        case 7: m.insert(QLatin1String("hasFloor"), (int)r.varint()); break;
        case 8: m.insert(QLatin1String("thread"), parseThreadInfo(r.message())); break;
        case 13: userList << parseUser(r.message()); break; // repeated User user_list
        case 39: m.insert(QLatin1String("displayForum"), parseSimpleForum(r.message())); break;
        default: r.skipField(); break;
        }
    }
    for (int i = 0; i < posts.size(); ++i)
        posts[i] = joinAuthor(posts.at(i).toMap(), userList);
    // Join sub-post authors as well.
    for (int i = 0; i < posts.size(); ++i) {
        QVariantMap p = posts.at(i).toMap();
        const QVariantList subs = p.value(QLatin1String("subPostList")).toList();
        if (subs.isEmpty()) continue;
        QVariantList subsOut;
        for (int k = 0; k < subs.size(); ++k) {
            QVariantMap s = subs.at(k).toMap();
            const QVariantList inner = s.value(QLatin1String("subPostList")).toList();
            QVariantList innerOut;
            for (int j = 0; j < inner.size(); ++j)
                innerOut << joinAuthor(inner.at(j).toMap(), userList);
            if (!inner.isEmpty()) s.insert(QLatin1String("subPostList"), innerOut);
            subsOut << joinAuthor(s, userList);
        }
        p.insert(QLatin1String("subPostList"), subsOut);
        posts[i] = p;
    }
    m.insert(QLatin1String("posts"), posts);
    return m;
}

QVariantMap parsePbFloorData(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList subs;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("page"), parsePage(r.message())); break;
        case 3: m.insert(QLatin1String("post"), parsePost(r.message())); break;
        case 4: subs << parseSubPostList(r.message()); break;
        case 5: m.insert(QLatin1String("thread"), parseThreadInfo(r.message())); break;
        case 6: m.insert(QLatin1String("forum"), parseSimpleForum(r.message())); break;
        case 8: m.insert(QLatin1String("displayForum"), parseSimpleForum(r.message())); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("subposts"), subs);
    return m;
}

QVariantMap parseRecommendForumInfo(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("avatar"), r.string()); break;
        case 2: m.insert(QLatin1String("id"), idStr(r.varint())); break;
        case 3: m.insert(QLatin1String("name"), r.string()); break;
        case 4: m.insert(QLatin1String("isLike"), (quint32)r.varint()); break;
        case 5: m.insert(QLatin1String("memberNum"), (quint32)r.varint()); break;
        case 6: m.insert(QLatin1String("threadNum"), (quint32)r.varint()); break;
        case 7: m.insert(QLatin1String("slogan"), r.string()); break;
        case 12: m.insert(QLatin1String("isBrandForum"), (quint32)r.varint()); break;
        case 18: m.insert(QLatin1String("lv1Name"), r.string()); break;
        case 19: m.insert(QLatin1String("lv2Name"), r.string()); break;
        default: r.skipField(); break;
        }
    }
    return m;
}

QVariantMap parsePostInfoList(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList content, media;
    int f;
    while ((f = r.next()) != 0) {
        switch (f) {
        case 1: m.insert(QLatin1String("forumId"), idStr(r.varint())); break;
        case 2: m.insert(QLatin1String("tid"), idStr(r.varint())); break;
        case 3: m.insert(QLatin1String("pid"), idStr(r.varint())); break;
        case 4: m.insert(QLatin1String("isThread"), (quint32)r.varint()); break;
        case 5: m.insert(QLatin1String("createTime"), (quint32)r.varint()); break;
        case 6: m.insert(QLatin1String("forumName"), r.string()); break;
        case 7: m.insert(QLatin1String("title"), r.string()); break;
        case 8: {
            // repeated PostInfoContent; each has field 1 = repeated Abstract
            PbReader cr(r.message());
            QVariantList ab;
            int cf;
            while ((cf = cr.next()) != 0) {
                if (cf == 1) ab << parseAbstract(cr.message());
                else cr.skipField();
            }
            m.insert(QLatin1String("abstract"), Content::joinAbstract(ab));
            break;
        }
        case 10: m.insert(QLatin1String("userName"), r.string()); break;
        case 14: m.insert(QLatin1String("abstractText"), r.string()); break;
        case 16: media << parseMedia(r.message()); break;
        case 17: m.insert(QLatin1String("replyNum"), (quint32)r.varint()); break;
        case 18: m.insert(QLatin1String("uid"), idStr(r.varint())); break;
        case 19: m.insert(QLatin1String("portrait"), r.string()); break;
        case 35: m.insert(QLatin1String("nameShow"), r.string()); break;
        case 37: m.insert(QLatin1String("agreeNum"), (int)r.varint()); break;
        case 38: m.insert(QLatin1String("viewNum"), (int)r.varint()); break;
        case 40: m.insert(QLatin1String("agree"), parseAgree(r.message())); break;
        case 42: m.insert(QLatin1String("originThread"), parseOriginThreadInfo(r.message())); break;
        default: r.skipField(); break;
        }
    }
    m.insert(QLatin1String("media"), media);
    m.insert(QLatin1String("avatar"), Content::avatarUrl(m.value(QLatin1String("portrait")).toString()));
    return m;
}

QVariantMap parseProfileData(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    QVariantList posts;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("user"), parseUser(r.message()));
        else if (f == 4) posts << parsePostInfoList(r.message());
        else if (f == 14) { // UserAgreeInfo
            PbReader ar(r.message());
            int af;
            while ((af = ar.next()) != 0) {
                if (af == 1) m.insert(QLatin1String("totalAgree"), qlonglong(ar.varint()));
                else ar.skipField();
            }
        }
        else r.skipField();
    }
    m.insert(QLatin1String("posts"), posts);
    return m;
}

QVariantMap parseForumDetailData(const QByteArray &p)
{
    PbReader r(p);
    QVariantMap m;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) m.insert(QLatin1String("forum"), parseRecommendForumInfo(r.message()));
        else r.skipField();
    }
    return m;
}

// ---- request builders ----

QByteArray buildCommonRequest(bool isPost)
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    const QString version = isPost ? QLatin1String("12.35.1.0") : QLatin1String("12.52.1.0");
    PbWriter w;
    w.varint(1, 2);                                        // _client_type
    w.string(2, version);                                   // _client_version
    w.string(3, ci->clientId());
    w.string(5, ci->imei());
    w.string(6, QLatin1String("1020031h"));                // from (V12)
    w.string(7, ci->cuid());
    w.varint(8, (quint64)QDateTime::currentMSecsSinceEpoch());
    w.string(9, ci->model());
    if (!ac->bduss().isEmpty()) w.string(10, ac->bduss());
    if (isPost && !ac->tbs().isEmpty()) w.string(11, ac->tbs());
    w.varint(12, 1);                                       // net_type
    w.string(24, QLatin1String("1.0.3"));                  // pversion
    w.string(25, ci->osVersion());
    w.string(26, ci->brand());
    w.string(28, QLatin1String("3.0.0"));                  // lego_lib_version
    if (!ac->stoken().isEmpty()) w.string(30, ac->stoken());
    w.string(32, ci->cuid());
    w.string(33, QLatin1String(""));
    w.string(35, ci->c3Aid());
    w.string(36, ci->sampleId());
    w.varint(37, 480);                                     // scr_w
    w.varint(38, 854);                                     // scr_h
    w.fixed64(39, 0x4000000000000000ULL);                  // scr_dip = 2.0 (double bits)
    w.varint(41, 0);                                       // is_teenager
    w.string(42, QLatin1String("2.34.0"));                 // sdk_ver
    w.string(43, QLatin1String("3340042"));                // framework_ver
    w.string(44, QLatin1String("1038000"));                // swan_game_ver
    w.varint(49, (quint64)ci->activeTimestamp());
    w.varint(50, (quint64)ci->firstInstallTime());
    w.varint(51, (quint64)ci->lastUpdateTime());
    w.string(53, QDate::currentDate().toString(QLatin1String("yyyyMdd")));
    w.string(54, ci->androidId());
    w.varint(55, 1);                                       // cmode
    w.varint(57, 1);                                       // start_type
    w.string(62, QLatin1String("tieba/") + version);       // user_agent
    w.varint(63, 1);                                       // personalized_rec_switch
    return w.data();
}

QByteArray encodeAdParam(int loadCount, int refreshCount, const QString &yoga)
{
    PbWriter w;
    if (loadCount) w.varint(1, (quint64)loadCount);
    if (refreshCount) w.varint(2, (quint64)refreshCount);
    if (!yoga.isEmpty()) w.string(3, yoga);
    return w.data();
}

QByteArray encodeEmptyMessage() { return QByteArray(); }

} // namespace

namespace Proto {

QByteArray frsPageRequest(const QString &kw, int pn, int sortType, const QString &cid, bool isGood)
{
    PbWriter data;
    data.string(1, SignUtil::urlEncode(kw));               // kw (url-encoded)
    data.varint(2, 90);                                    // rn
    data.varint(3, 30);                                    // rn_need
    if (isGood && !cid.isEmpty()) {
        data.varint(4, 1);                                 // is_good
        data.varint(5, (quint64)cid.toLongLong());         // cid
    }
    data.varint(8, 1);                                     // with_group
    data.varint(11, 480);                                  // scr_w
    data.varint(12, 854);                                  // scr_h
    data.fixed64(13, 0x4000000000000000ULL);               // scr_dip
    data.varint(14, 2);                                    // q_type
    data.varint(15, (quint64)pn);                          // pn
    data.string(16, QLatin1String("recom_flist"));         // st_type
    data.message(39, buildCommonRequest(false));           // common
    if (sortType != 0) data.varint(47, (quint64)sortType); // sort_type
    data.varint(49, (quint64)(pn == 1 ? 1 : 2));           // load_type
    data.message(50, encodeEmptyMessage());                // app_pos
    data.message(51, encodeAdParam(0, 4, QLatin1String("1.0"))); // ad_param

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

QByteArray pbPageRequest(const QString &tid, int pn, int seeLz, int sort, qint64 lastPid, qint64 forumId)
{
    PbWriter data;
    data.varint(4, (quint64)tid.toLongLong());             // kz
    if (seeLz) data.varint(5, 1);                          // lz
    if (sort) data.varint(6, 1);                           // r
    if (lastPid > 0) data.varint(7, (quint64)lastPid);     // pid (jump)
    data.varint(8, 1);                                     // with_floor
    data.varint(9, 4);                                     // floor_rn
    data.varint(13, 15);                                   // rn
    data.varint(14, 480);                                  // scr_w
    data.varint(15, 854);                                  // scr_h
    data.fixed64(16, 0x4000000000000000ULL);               // scr_dip
    data.varint(17, 2);                                    // q_type
    data.varint(18, (quint64)pn);                          // pn
    data.string(19, QLatin1String("tb_frslist"));          // st_type
    data.message(25, buildCommonRequest(false));           // common
    if (forumId > 0) data.varint(56, (quint64)forumId);    // forum_id
    data.varint(74, 1);                                    // floor_sort_type
    data.varint(75, 2);                                    // source_type

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

QByteArray pbFloorRequest(const QString &tid, const QString &pid, int pn, qint64 forumId)
{
    PbWriter data;
    data.varint(1, (quint64)tid.toLongLong());             // kz
    if (!pid.isEmpty()) data.varint(2, (quint64)pid.toLongLong()); // pid
    data.varint(4, (quint64)pn);                           // pn
    data.varint(5, 480);                                   // scr_w
    data.varint(6, 854);                                   // scr_h
    data.fixed64(7, 0x4000000000000000ULL);                // scr_dip
    data.message(9, buildCommonRequest(false));            // common
    if (forumId > 0) data.varint(11, (quint64)forumId);    // forum_id

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

QByteArray addPostRequest(const QVariantMap &args)
{
    const QString content = args.value(QLatin1String("content")).toString();
    const QString fid = args.value(QLatin1String("fid")).toString();
    const QString kw = args.value(QLatin1String("kw")).toString();
    const QString tid = args.value(QLatin1String("tid")).toString();
    const QString postId = args.value(QLatin1String("postId")).toString();
    const QString subPostId = args.value(QLatin1String("subPostId")).toString();
    const QString replyUid = args.value(QLatin1String("replyUid")).toString();
    const QString nameShow = args.value(QLatin1String("nameShow")).toString();

    const bool hasPost = !postId.isEmpty();
    const bool hasSub = !subPostId.isEmpty();

    PbWriter data;
    data.message(1, buildCommonRequest(true));             // common (V12_POST)
    data.string(6, QLatin1String("1"));                    // anonymous
    data.string(7, QLatin1String("0"));                    // can_no_forum
    data.string(8, QLatin1String("0"));                    // is_feedback
    data.string(9, QLatin1String("0"));                    // takephoto_num
    data.string(10, QLatin1String("0"));                   // entrance_type
    if (!hasPost) data.string(32, QLatin1String("0"));     // barrage_time
    data.string(16, QLatin1String("12"));                  // vcode_tag
    data.string(18, QLatin1String("1"));                   // new_vcode
    if (!content.isEmpty()) data.string(19, content);      // content
    if (hasPost && !replyUid.isEmpty()) data.string(20, replyUid); // reply_uid
    data.string(26, fid);                                  // fid
    if (!hasPost) {
        data.string(28, QLatin1String(""));               // v_fid
        data.string(29, QLatin1String(""));               // v_fname
    }
    data.string(30, kw);                                   // kw
    data.string(31, QLatin1String("0"));                   // is_barrage
    data.string(45, tid);                                  // tid
    if (hasPost) data.string(46, postId);                  // quote_id
    data.string(48, QLatin1String("0"));                   // floor_num
    if (hasPost) data.string(49, postId);                  // repostid
    if (hasSub) data.string(50, subPostId);                // sub_post_id
    data.string(51, QLatin1String("0"));                   // is_ad
    data.string(52, QLatin1String("0"));                   // is_addition
    data.string(53, QLatin1String("0"));                   // is_giftpost
    data.string(47, QLatin1String("0"));                   // is_twzhibo_thread
    if (!hasSub) data.string(55, hasPost ? QLatin1String("0") : QLatin1String("13")); // post_from
    data.string(58, nameShow);                             // name_show
    data.string(60, QLatin1String("0"));                   // is_pictxt
    data.varint(67, 0);                                    // is_show_bless

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

QByteArray profileRequest(const QString &uid, bool isSelf, int pn)
{
    PbWriter data;
    if (isSelf) data.varint(1, (quint64)uid.toLongLong());
    else data.varint(3, (quint64)uid.toLongLong());        // friend_uid
    data.varint(2, 1);                                     // need_post_count
    data.varint(4, (quint64)(isSelf ? 0 : 1));             // is_guest
    data.varint(6, (quint64)pn);                           // pn
    data.varint(7, 20);                                    // rn
    data.varint(8, 1);                                     // has_plist
    data.message(9, buildCommonRequest(false));            // common
    data.varint(10, 480);                                  // scr_w
    data.varint(11, 854);                                  // scr_h
    data.varint(12, 0);                                    // q_type
    data.fixed64(13, 0x4000000000000000ULL);               // scr_dip
    data.varint(14, 1);                                    // is_from_usercenter
    data.varint(15, 1);                                    // page

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

QByteArray forumDetailRequest(qint64 forumId)
{
    PbWriter data;
    data.varint(1, (quint64)forumId);                      // forum_id
    data.message(2, buildCommonRequest(false));            // common

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

// PersonalizedRequestData (docs/REF-api-endpoints.md A2, Personalized.proto):
// common=1, tag_code=2, need_tags=3, load_type=4, page_thread_count=5, pn=6,
// sug_count=7, q_type=11, need_forumlist=22, new_net_type=23, new_install=27,
// request_times=28.
QByteArray personalizedRequest(int pn, int loadType)
{
    PbWriter data;
    data.message(1, buildCommonRequest(false));            // common (V12)
    data.varint(2, 0);                                     // tag_code
    data.varint(3, 0);                                     // need_tags
    data.varint(4, (quint64)loadType);                     // load_type: 1 refresh / 2 load more
    data.varint(5, 11);                                    // page_thread_count
    data.varint(6, (quint64)pn);                           // pn
    data.varint(7, 0);                                     // sug_count
    data.varint(11, 1);                                    // q_type
    data.varint(22, 0);                                    // need_forumlist
    data.varint(23, 1);                                    // new_net_type
    data.varint(27, 0);                                    // new_install
    data.varint(28, 0);                                    // request_times

    PbWriter req;
    req.message(1, data.data());
    return req.data();
}

QVariantMap frsPageResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) {
            QVariantMap d = parseFrsPageData(r.message());
            result.insert(QLatin1String("forum"), d.value(QLatin1String("forum")));
            result.insert(QLatin1String("page"), d.value(QLatin1String("page")));
            result.insert(QLatin1String("threads"), d.value(QLatin1String("threads")));
            result.insert(QLatin1String("anti"), d.value(QLatin1String("anti")));
            result.insert(QLatin1String("user"), d.value(QLatin1String("user")));
        } else r.skipField();
    }
    return result;
}

QVariantMap pbPageResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) {
            QVariantMap d = parsePbPageData(r.message());
            result.insert(QLatin1String("forum"), d.value(QLatin1String("forum")));
            result.insert(QLatin1String("page"), d.value(QLatin1String("page")));
            result.insert(QLatin1String("posts"), d.value(QLatin1String("posts")));
            result.insert(QLatin1String("anti"), d.value(QLatin1String("anti")));
            result.insert(QLatin1String("thread"), d.value(QLatin1String("thread")));
            result.insert(QLatin1String("hasFloor"), d.value(QLatin1String("hasFloor")));
            result.insert(QLatin1String("user"), d.value(QLatin1String("user")));
        } else r.skipField();
    }
    return result;
}

QVariantMap pbFloorResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) {
            QVariantMap d = parsePbFloorData(r.message());
            result.insert(QLatin1String("page"), d.value(QLatin1String("page")));
            result.insert(QLatin1String("post"), d.value(QLatin1String("post")));
            result.insert(QLatin1String("subposts"), d.value(QLatin1String("subposts")));
            result.insert(QLatin1String("thread"), d.value(QLatin1String("thread")));
            result.insert(QLatin1String("forum"), d.value(QLatin1String("forum")));
            result.insert(QLatin1String("displayForum"), d.value(QLatin1String("displayForum")));
        } else r.skipField();
    }
    return result;
}

QVariantMap addPostResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) {
            PbReader d(r.message());
            int df;
            while ((df = d.next()) != 0) {
                if (df == 2) result.insert(QLatin1String("tid"), d.string());
                else if (df == 3) result.insert(QLatin1String("pid"), d.string());
                else if (df == 5) result.insert(QLatin1String("msg"), d.string());
                else d.skipField();
            }
        } else r.skipField();
    }
    return result;
}

QVariantMap profileResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) {
            QVariantMap d = parseProfileData(r.message());
            result.insert(QLatin1String("user"), d.value(QLatin1String("user")));
            result.insert(QLatin1String("posts"), d.value(QLatin1String("posts")));
            result.insert(QLatin1String("totalAgree"), d.value(QLatin1String("totalAgree")));
        } else r.skipField();
    }
    return result;
}

QVariantMap forumDetailResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) result.insert(QLatin1String("forum"), parseForumDetailData(r.message()).value(QLatin1String("forum")));
        else r.skipField();
    }
    return result;
}

// PersonalizedResponse { error=1, data=2 }; PersonalizedResponseData holds
// repeated ThreadInfo thread_list at field 2 (thread_personalized=7, unused:
// we only need the plain thread cards). No Page block — pagination is driven
// by the request's pn alone.
QVariantMap personalizedResponse(const QByteArray &data)
{
    PbReader r(data);
    QVariantMap result;
    int f;
    while ((f = r.next()) != 0) {
        if (f == 1) result.insert(QLatin1String("error"), parseError(r.message()));
        else if (f == 2) {
            PbReader d(r.message());
            QVariantList threads;
            int df;
            while ((df = d.next()) != 0) {
                if (df == 2) threads << parseThreadInfo(d.message());
                else d.skipField();
            }
            result.insert(QLatin1String("threads"), threads);
        } else r.skipField();
    }
    return result;
}

} // namespace Proto
