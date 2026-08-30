#ifndef TIEBA_TIEBAAPI_H
#define TIEBA_TIEBAAPI_H

// Central network API: builds common params, signs requests, issues JSON
// (c.tieba.baidu.com / tieba.baidu.com) and protobuf (tiebac.baidu.com)
// requests, parses responses in C++, and emits QML-friendly signals.

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include "httpclient.h"
#include "signutil.h"

class TiebaApi : public QObject
{
    Q_OBJECT
public:
    static TiebaApi *instance();

    // Browse
    Q_INVOKABLE void loadFollowedForums();
    Q_INVOKABLE void loadPersonalized(int pn); // 个性化推荐 feed (cmd 309264)
    Q_INVOKABLE void loadFrsPage(const QString &kw, int pn, int sortType);
    Q_INVOKABLE void loadThreadPage(const QString &tid, int pn, bool seeLz, bool reverse, const QString &forumId);
    Q_INVOKABLE void loadSubFloor(const QString &tid, const QString &pid, int pn, const QString &forumId);

    // Search
    Q_INVOKABLE void searchThread(const QString &word, int pn);
    Q_INVOKABLE void searchForum(const QString &word);
    Q_INVOKABLE void searchUser(const QString &word);

    // Validate pasted BDUSS/STOKEN and populate UID/name/tbs from the server.
    Q_INVOKABLE void login(const QString &bduss, const QString &stoken,
                           const QString &tbs = QString());

    // Actions
    Q_INVOKABLE void likeForum(const QString &fid, const QString &kw);
    Q_INVOKABLE void unlikeForum(const QString &fid, const QString &kw);
    Q_INVOKABLE void agree(const QString &threadId, const QString &postId, bool cancel);
    Q_INVOKABLE void addStore(const QString &tid, const QString &pid);
    Q_INVOKABLE void removeStore(const QString &tid);
    Q_INVOKABLE void loadStoreList();
    Q_INVOKABLE void addPost(const QVariantMap &args);
    Q_INVOKABLE void uploadImages(const QVariantList &paths, const QString &forumName);
    Q_INVOKABLE void reportPost(const QString &postId); // returns report page url via actionFinished("report")

    // Feeds
    Q_INVOKABLE void loadReplyMe(int pn);
    Q_INVOKABLE void loadAtMe(int pn);
    Q_INVOKABLE void loadMsg();

    // User / forum
    Q_INVOKABLE void loadUserProfile(const QString &uid);
    Q_INVOKABLE void loadUserPost(const QString &uid, bool isThread, int pn);
    Q_INVOKABLE void loadForumDetail(const QString &forumId);

    // Misc
    Q_INVOKABLE void cancelAll();

signals:
    void followedForumsReady(const QVariantList &forums, const QString &error);
    void personalizedReady(const QVariantList &threads, bool hasMore, const QString &error);
    void frsPageReady(const QVariantMap &data, const QString &error);
    void threadPageReady(const QVariantMap &data, const QString &error);
    void subFloorReady(const QVariantMap &data, const QString &error);
    void searchThreadsReady(const QVariantList &items, bool hasMore, const QString &error);
    void searchForumsReady(const QVariantList &items, const QString &error);
    void searchUsersReady(const QVariantList &items, const QString &error);
    void storeListReady(const QVariantList &items, const QString &error);
    void replyMeReady(const QVariantList &items, const QString &error);
    void atMeReady(const QVariantList &items, const QString &error);
    void msgReady(const QVariantMap &counts, const QString &error);
    void userProfileReady(const QVariantMap &profile, const QString &error);
    void userPostReady(const QVariantList &posts, bool hasMore, const QString &error);
    void forumDetailReady(const QVariantMap &forum, const QString &error);
    void actionFinished(const QString &action, bool ok, const QString &message, const QVariantMap &data);
    void loginExpired();
    void uploadProgress(int current, int total, const QString &path);
    void uploadDone(const QVariantList &results, const QString &error); // results: [{picId,width,height}]

private slots:
    void onHttpFinished(int requestId, bool ok, int httpStatus, const QByteArray &data,
                        const QString &errString);

private:
    enum RequestType {
        ReqFollowedForums = 1, ReqFrsPage, ReqThreadPage, ReqSubFloor,
        ReqSearchThread, ReqSearchForum, ReqSearchUser,
        ReqLikeForum, ReqUnlikeForum, ReqAgree,
        ReqAddStore, ReqRemoveStore, ReqStoreList,
        ReqReplyMe, ReqAtMe, ReqMsg,
        ReqUserProfile, ReqUserPost, ReqForumDetail,
        ReqAddPost, ReqUploadPic, ReqReport, ReqLogin,
        ReqPersonalized
    };

    struct Pending { int type; QVariantMap ctx; };

    explicit TiebaApi(QObject *parent = 0);
    static TiebaApi *s_instance;
    HttpClient *m_http;
    QHash<int, Pending> m_pending;
    // Set once a secure protobuf request fails at transport level; all later
    // proto calls go over plain HTTP (see sendProto).
    bool m_preferInsecure;

    // image upload state (sequential chunked multipart to /c/s/uploadPicture)
    QVariantList m_uploadPaths, m_uploadResults;
    QString m_uploadForum;
    int m_uploadIndex, m_uploadChunkNo, m_uploadTotalChunks;
    qint64 m_uploadFileLen;
    int m_uploadWidth, m_uploadHeight;
    QByteArray m_uploadFileData, m_uploadFileMd5Hex;

    void startUploadFile();
    void startUploadChunk();
    void handleUploadResponse(bool ok, int httpStatus, const QByteArray &data);
    QByteArray buildUploadMultipart(int chunkNo, bool isFinish, const QByteArray &chunk) const;
    QString uploadCookie() const;

    HttpParams officialCommonParams() const;
    HttpParams miniCommonParams() const;
    HttpParams newCommonParams() const;
    HttpClient::Headers officialHeaders() const;
    HttpClient::Headers headersWithUserAgent(const QString &userAgent) const;
    HttpClient::Headers protoHeaders(const QString &forumName = QString()) const;
    HttpClient::Headers webHeaders() const;

    void sendSignedForm(const QString &url, const HttpParams &common, const HttpParams &business,
                        const HttpClient::Headers &headers, int type, const QVariantMap &ctx);
    void sendProto(const QString &path, const QByteArray &body, bool needStoken,
                   const QString &forumName, int type, const QVariantMap &ctx);
    void retryProtoOverHttp(int type, const QVariantMap &ctx);
    void handleResponse(int type, const QVariantMap &ctx, bool ok, int status,
                        const QByteArray &data, const QString &errString);
    bool isError(const QVariantMap &m, QString *msg);

    // offline page cache helpers (frs/pb/floor)
    QString cacheKey(const QString &prefix, const QVariantMap &ctx) const;
    void cachePageData(const QString &prefix, const QVariantMap &ctx, const QVariantMap &data);
    QVariantMap loadCachedPage(const QString &prefix, const QVariantMap &ctx) const;
};

#endif // TIEBA_TIEBAAPI_H
