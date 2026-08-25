#ifndef TIEBA_PROTO_MESSAGES_H
#define TIEBA_PROTO_MESSAGES_H

// Hand-written protobuf message builders/parsers for the Tieba Plus API
// (tiebac.baidu.com). Requests are serialized to their outer *Request bytes;
// responses are parsed from raw protobuf bytes into QVariantMap trees consumed
// by QML. Field numbers follow docs/REF-protobuf.md.

#include <QByteArray>
#include <QString>
#include <QVariantMap>
#include <QVariantList>

namespace Proto
{
    // ---- Requests (return the serialized outer *Request message) ----
    QByteArray frsPageRequest(const QString &kw, int pn, int sortType,
                              const QString &cid, bool isGood);
    QByteArray pbPageRequest(const QString &tid, int pn, int seeLz, int sort,
                             qint64 lastPid, qint64 forumId);
    QByteArray pbFloorRequest(const QString &tid, const QString &pid, int pn, qint64 forumId);
    // args keys: content, fid, kw, tid, postId, subPostId, replyUid, nameShow, tbs
    QByteArray addPostRequest(const QVariantMap &args);
    QByteArray profileRequest(const QString &uid, bool isSelf, int pn);
    QByteArray forumDetailRequest(qint64 forumId);
    QByteArray personalizedRequest(int pn, int loadType);

    // ---- Responses ----
    QVariantMap frsPageResponse(const QByteArray &data);
    QVariantMap pbPageResponse(const QByteArray &data);
    QVariantMap pbFloorResponse(const QByteArray &data);
    QVariantMap addPostResponse(const QByteArray &data);
    QVariantMap profileResponse(const QByteArray &data);
    QVariantMap forumDetailResponse(const QByteArray &data);
    QVariantMap personalizedResponse(const QByteArray &data);
}

#endif // TIEBA_PROTO_MESSAGES_H
