#include "tiebaapi.h"
#include "json.h"
#include "proto_messages.h"
#include "clientinfo.h"
#include "accountmanager.h"
#include "appsettings.h"
#include "content.h"
#include "db.h"
#include "logstore.h"
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QCryptographicHash>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QVariantList>
#include <QtGui/QImageReader>
#include <QtGui/QImage>
#include <QtCore/QBuffer>
#include <QtCore/QSize>
#include <QtCore/QtGlobal>

TiebaApi *TiebaApi::s_instance = 0;

namespace {

static QString js(const QVariantMap &m, const char *k) { return m.value(QLatin1String(k)).toString(); }

static const QByteArray kSecret = "tiebaclient!!!";

QByteArray buildMultipart(const QByteArray &data, bool addStoken)
{
    const QByteArray boundary = "--------7da3d81520810*";
    AccountManager *ac = AccountManager::instance();
    QByteArray body;
    if (addStoken && !ac->stoken().isEmpty()) {
        body += "--" + boundary + "\r\n";
        body += "Content-Disposition: form-data; name=\"stoken\"\r\n\r\n";
        body += ac->stoken().toUtf8() + "\r\n";
    }
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"data\"; filename=\"file\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";
    body += data;
    body += "\r\n--" + boundary + "--\r\n";
    return body;
}

static HttpParams merge(const HttpParams &common, const HttpParams &business)
{
    HttpParams p = common;
    QMap<QString, QString> b = business.map();
    QMap<QString, QString>::const_iterator it = b.constBegin();
    for (; it != b.constEnd(); ++it)
        p.add(it.key(), it.value());
    return p;
}

} // namespace

TiebaApi::TiebaApi(QObject *parent)
    : QObject(parent), m_preferInsecure(false)
{
    m_http = new HttpClient(this);
    connect(m_http, SIGNAL(finished(int, bool, int, QByteArray, QString)),
            this, SLOT(onHttpFinished(int, bool, int, QByteArray, QString)));
}

TiebaApi *TiebaApi::instance()
{
    if (!s_instance)
        s_instance = new TiebaApi();
    return s_instance;
}

// ---- common params ----

HttpParams TiebaApi::officialCommonParams() const
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    HttpParams p;
    p.add(QLatin1String("BDUSS"), ac->bduss());
    p.add(QLatin1String("_client_id"), ci->clientId());
    p.add(QLatin1String("_client_type"), QLatin1String("2"));
    p.add(QLatin1String("_os_version"), ci->osVersion());
    p.add(QLatin1String("model"), ci->model());
    p.add(QLatin1String("net_type"), QLatin1String("1"));
    p.add(QLatin1String("_phone_imei"), ci->imei());
    p.add(QLatin1String("timestamp"), SignUtil::currentTimestampMs());
    p.add(QLatin1String("active_timestamp"), QString::number(ci->activeTimestamp()));
    p.add(QLatin1String("android_id"), ci->androidId());
    p.add(QLatin1String("baiduid"), ci->baiduId());
    p.add(QLatin1String("brand"), ci->brand());
    p.add(QLatin1String("cmode"), QLatin1String("1"));
    p.add(QLatin1String("cuid"), ci->cuid());
    p.add(QLatin1String("cuid_galaxy2"), ci->cuid());
    p.add(QLatin1String("cuid_gid"), QLatin1String(""));
    p.add(QLatin1String("event_day"), QDate::currentDate().toString(QLatin1String("yyyyMdd")));
    p.add(QLatin1String("extra"), QLatin1String(""));
    p.add(QLatin1String("first_install_time"), QString::number(ci->firstInstallTime()));
    p.add(QLatin1String("framework_ver"), QLatin1String("3340042"));
    p.add(QLatin1String("from"), QLatin1String("tieba"));
    p.add(QLatin1String("is_teenager"), QLatin1String("0"));
    p.add(QLatin1String("last_update_time"), QString::number(ci->lastUpdateTime()));
    p.add(QLatin1String("mac"), QLatin1String("02:00:00:00:00:00"));
    p.add(QLatin1String("sample_id"), ci->sampleId());
    p.add(QLatin1String("sdk_ver"), QLatin1String("2.34.0"));
    p.add(QLatin1String("start_scheme"), QLatin1String(""));
    p.add(QLatin1String("start_type"), QLatin1String("1"));
    p.add(QLatin1String("swan_game_ver"), QLatin1String("1038000"));
    p.add(QLatin1String("_client_version"), QLatin1String("12.25.1.0"));
    p.add(QLatin1String("c3_aid"), ci->c3Aid());
    p.add(QLatin1String("oaid"), QLatin1String("{}"));
    return p;
}

HttpParams TiebaApi::miniCommonParams() const
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    HttpParams p;
    p.add(QLatin1String("BDUSS"), ac->bduss());
    p.add(QLatin1String("_client_id"), ci->clientId());
    p.add(QLatin1String("_client_type"), QLatin1String("2"));
    p.add(QLatin1String("_os_version"), ci->osVersion());
    p.add(QLatin1String("model"), ci->model());
    p.add(QLatin1String("net_type"), QLatin1String("1"));
    p.add(QLatin1String("_phone_imei"), ci->imei());
    p.add(QLatin1String("timestamp"), SignUtil::currentTimestampMs());
    p.add(QLatin1String("cuid"), ci->cuid());
    p.add(QLatin1String("cuid_galaxy2"), ci->cuid());
    p.add(QLatin1String("from"), QLatin1String("1021636m"));
    p.add(QLatin1String("_client_version"), QLatin1String("7.2.0.0"));
    p.add(QLatin1String("subapp_type"), QLatin1String("mini"));
    return p;
}

HttpParams TiebaApi::newCommonParams() const
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    HttpParams p;
    p.add(QLatin1String("BDUSS"), ac->bduss());
    p.add(QLatin1String("_client_id"), ci->clientId());
    p.add(QLatin1String("_client_type"), QLatin1String("2"));
    p.add(QLatin1String("_os_version"), ci->osVersion());
    p.add(QLatin1String("model"), ci->model());
    p.add(QLatin1String("net_type"), QLatin1String("1"));
    p.add(QLatin1String("_phone_imei"), ci->imei());
    p.add(QLatin1String("timestamp"), SignUtil::currentTimestampMs());
    p.add(QLatin1String("cuid"), ci->cuid());
    p.add(QLatin1String("from"), QLatin1String("baidu_appstore"));
    p.add(QLatin1String("_client_version"), QLatin1String("8.2.2"));
    return p;
}

HttpClient::Headers TiebaApi::officialHeaders() const
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    HttpClient::Headers h;
    h << qMakePair(QString(QLatin1String("User-Agent")), QString(QLatin1String("bdtb for Android 12.25.1.0")));
    h << qMakePair(QString(QLatin1String("Cookie")),
                   QString(QLatin1String("CUID=") + ci->cuid() + QLatin1String(";ka=open;TBBRAND=") + ci->model() +
                           QLatin1String(";BAIDUID=") + ci->baiduId() + QLatin1String(";")));
    h << qMakePair(QString(QLatin1String("CUID")), ci->cuid());
    h << qMakePair(QString(QLatin1String("CUID_GALAXY2")), ci->cuid());
    h << qMakePair(QString(QLatin1String("CUID_GID")), QString(QLatin1String("")));
    h << qMakePair(QString(QLatin1String("CUID_GALAXY3")), ci->c3Aid());
    h << qMakePair(QString(QLatin1String("client_type")), QString(QLatin1String("2")));
    h << qMakePair(QString(QLatin1String("Charset")), QString(QLatin1String("UTF-8")));
    if (!ac->uid().isEmpty())
        h << qMakePair(QString(QLatin1String("client_user_token")), ac->uid());
    return h;
}

HttpClient::Headers TiebaApi::headersWithUserAgent(const QString &userAgent) const
{
    HttpClient::Headers h = officialHeaders();
    for (int i = 0; i < h.size(); ++i) {
        if (h.at(i).first == QLatin1String("User-Agent")) {
            h[i].second = userAgent;
            break;
        }
    }
    return h;
}

HttpClient::Headers TiebaApi::protoHeaders(const QString &forumName) const
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    HttpClient::Headers h;
    h << qMakePair(QString(QLatin1String("Charset")), QString(QLatin1String("UTF-8")));
    h << qMakePair(QString(QLatin1String("client_type")), QString(QLatin1String("2")));
    h << qMakePair(QString(QLatin1String("Cookie")),
                   QString(QLatin1String("ka=open;CUID=") + ci->cuid() + QLatin1String(";TBBRAND=") + ci->model() + QLatin1String(";")));
    h << qMakePair(QString(QLatin1String("CUID")), ci->cuid());
    h << qMakePair(QString(QLatin1String("CUID_GALAXY2")), ci->cuid());
    h << qMakePair(QString(QLatin1String("CUID_GID")), QString(QLatin1String("")));
    h << qMakePair(QString(QLatin1String("CUID_GALAXY3")), ci->c3Aid());
    h << qMakePair(QString(QLatin1String("User-Agent")), QString(QLatin1String("tieba/12.52.1.0")));
    h << qMakePair(QString(QLatin1String("x_bd_data_type")), QString(QLatin1String("protobuf")));
    if (!ac->uid().isEmpty())
        h << qMakePair(QString(QLatin1String("client_user_token")), ac->uid());
    if (!forumName.isEmpty())
        h << qMakePair(QString(QLatin1String("forum_name")), SignUtil::urlEncode(forumName));
    return h;
}

HttpClient::Headers TiebaApi::webHeaders() const
{
    ClientInfo *ci = ClientInfo::instance();
    AccountManager *ac = AccountManager::instance();
    HttpClient::Headers h;
    h << qMakePair(QString(QLatin1String("User-Agent")), ci->webUserAgent());
    h << qMakePair(QString(QLatin1String("Host")), QString(QLatin1String("tieba.baidu.com")));
    h << qMakePair(QString(QLatin1String("Accept")), QString(QLatin1String("application/json, text/plain, */*")));
    QString cookie = QLatin1String("CUID=") + ci->cuid() +
                     QLatin1String(";TBBRAND=") + ci->model() +
                     QLatin1String(";cuid_galaxy2=") + ci->cuid() +
                     QLatin1String(";BDUSS=") + ac->bduss() +
                     QLatin1String(";STOKEN=") + ac->stoken() +
                     QLatin1String(";BAIDUID=") + ci->baiduId() +
                     QLatin1String(";BAIDUID_BFESS=") + ci->baiduId() +
                     QLatin1String(";mo_originid=2");
    h << qMakePair(QString(QLatin1String("Cookie")), cookie);
    return h;
}

// ---- request helpers ----

void TiebaApi::sendSignedForm(const QString &url, const HttpParams &common, const HttpParams &business,
                              const HttpClient::Headers &headers, int type, const QVariantMap &ctx)
{
    HttpParams params = merge(common, business);
    StParams::addTo(params);
    const QString sign = SignUtil::sign(params, QString::fromLatin1(kSecret));
    QByteArray body = params.formEncoded();
    body += "&sign=";
    body += sign.toLatin1();
    const int id = m_http->postForm(url, body, headers);
    Pending p; p.type = type; p.ctx = ctx;
    m_pending.insert(id, p);
}

void TiebaApi::sendProto(const QString &path, const QByteArray &body, bool needStoken,
                         const QString &forumName, int type, const QVariantMap &ctx)
{
    // tiebac.baidu.com requires modern TLS which the N9's OpenSSL 0.9.8 often
    // cannot negotiate. The very same protobuf endpoints also answer over
    // plain HTTP on c.tieba.baidu.com, so we degrade automatically once a
    // secure request fails at the transport layer.
    const QString host = m_preferInsecure
        ? QLatin1String("http://c.tieba.baidu.com")
        : QLatin1String("https://tiebac.baidu.com");
    const QString url = host + path;
    qDebug("API proto type=%d via %s", type, qPrintable(url));
    const QByteArray multipart = buildMultipart(body, needStoken);
    HttpClient::Headers headers = protoHeaders(forumName);
    const int id = m_http->postRaw(url, multipart,
        QByteArray("multipart/form-data; boundary=--------7da3d81520810*"), headers);
    Pending p; p.type = type; p.ctx = ctx;
    m_pending.insert(id, p);
}

// Re-dispatches a failed protobuf page request over the insecure endpoint.
void TiebaApi::retryProtoOverHttp(int type, const QVariantMap &ctx)
{
    qWarning("API falling back to HTTP for type=%d", type);
    m_preferInsecure = true;
    switch (type) {
    case ReqFrsPage:
        loadFrsPage(js(ctx, "kw"), ctx.value(QLatin1String("pn")).toInt(),
                    ctx.value(QLatin1String("sortType")).toInt());
        break;
    case ReqThreadPage:
        loadThreadPage(js(ctx, "tid"), ctx.value(QLatin1String("pn")).toInt(),
                       ctx.value(QLatin1String("seeLz")).toBool(),
                       ctx.value(QLatin1String("reverse")).toBool(), js(ctx, "forumId"));
        break;
    case ReqSubFloor:
        loadSubFloor(js(ctx, "tid"), js(ctx, "pid"),
                     ctx.value(QLatin1String("pn")).toInt(), js(ctx, "forumId"));
        break;
    case ReqPersonalized:
        loadPersonalized(ctx.value(QLatin1String("pn")).toInt());
        break;
    default:
        break;
    }
}

// ---- browse ----

void TiebaApi::loadFollowedForums()
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("BDUSS"), ac->bduss());
    b.add(QLatin1String("stoken"), ac->stoken());
    b.add(QLatin1String("user_id"), ac->uid());
    b.add(QLatin1String("_client_version"), QLatin1String("11.10.8.6"));
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/f/forum/getforumlist"), officialCommonParams(), b,
                   headersWithUserAgent(QLatin1String("bdtb for Android 11.10.8.6")), ReqFollowedForums, QVariantMap());
}

void TiebaApi::loadPersonalized(int pn)
{
    // 1 = refresh, 2 = load more (mirrors the official client).
    const QByteArray body = Proto::personalizedRequest(pn, pn <= 1 ? 1 : 2);
    QVariantMap ctx;
    ctx.insert(QLatin1String("pn"), pn);
    sendProto(QLatin1String("/c/f/excellent/personalized?cmd=309264"), body, true, QString(), ReqPersonalized, ctx);
}

void TiebaApi::loadFrsPage(const QString &kw, int pn, int sortType)
{
    QByteArray body = Proto::frsPageRequest(kw, pn, sortType, QString(), false);
    QVariantMap ctx;
    ctx.insert(QLatin1String("kw"), kw);
    ctx.insert(QLatin1String("pn"), pn);
    ctx.insert(QLatin1String("sortType"), sortType);
    sendProto(QLatin1String("/c/f/frs/page?cmd=301001"), body, true, kw, ReqFrsPage, ctx);
}

void TiebaApi::loadThreadPage(const QString &tid, int pn, bool seeLz, bool reverse, const QString &forumId)
{
    QByteArray body = Proto::pbPageRequest(tid, pn, seeLz ? 1 : 0, reverse ? 1 : 0, 0,
                                           forumId.toLongLong());
    QVariantMap ctx;
    ctx.insert(QLatin1String("tid"), tid);
    ctx.insert(QLatin1String("pn"), pn);
    ctx.insert(QLatin1String("forumId"), forumId);
    ctx.insert(QLatin1String("seeLz"), seeLz);
    ctx.insert(QLatin1String("reverse"), reverse);
    sendProto(QLatin1String("/c/f/pb/page?cmd=302001&format=protobuf"), body, true, QString(), ReqThreadPage, ctx);
}

void TiebaApi::loadSubFloor(const QString &tid, const QString &pid, int pn, const QString &forumId)
{
    QByteArray body = Proto::pbFloorRequest(tid, pid, pn, forumId.toLongLong());
    QVariantMap ctx;
    ctx.insert(QLatin1String("tid"), tid);
    ctx.insert(QLatin1String("pid"), pid);
    ctx.insert(QLatin1String("pn"), pn);
    ctx.insert(QLatin1String("forumId"), forumId);
    sendProto(QLatin1String("/c/f/pb/floor?cmd=302002&format=protobuf"), body, false, QString(), ReqSubFloor, ctx);
}

// ---- search ----

void TiebaApi::searchThread(const QString &word, int pn)
{
    QString url = QLatin1String("https://tieba.baidu.com/mo/q/search/thread?word=") + SignUtil::urlEncode(word) +
                  QLatin1String("&pn=") + QString::number(pn) +
                  QLatin1String("&st=1&tt=1&ct=2&is_use_zonghe=1&cv=99.9.101");
    HttpClient::Headers h = webHeaders();
    h << qMakePair(QString(QLatin1String("Referer")),
                   QString(QLatin1String("https://tieba.baidu.com/mo/q/hybrid/search?keyword=") + SignUtil::urlEncode(word)));
    QVariantMap ctx; ctx.insert(QLatin1String("word"), word);
    const int id = m_http->get(url, h);
    Pending p; p.type = ReqSearchThread; p.ctx = ctx;
    m_pending.insert(id, p);
}

void TiebaApi::searchForum(const QString &word)
{
    QString url = QLatin1String("https://tieba.baidu.com/mo/q/search/forum?word=") + SignUtil::urlEncode(word);
    HttpClient::Headers h = webHeaders();
    QVariantMap ctx; ctx.insert(QLatin1String("word"), word);
    const int id = m_http->get(url, h);
    Pending p; p.type = ReqSearchForum; p.ctx = ctx;
    m_pending.insert(id, p);
}

void TiebaApi::searchUser(const QString &word)
{
    QString url = QLatin1String("https://tieba.baidu.com/mo/q/search/user?word=") + SignUtil::urlEncode(word);
    HttpClient::Headers h = webHeaders();
    QVariantMap ctx; ctx.insert(QLatin1String("word"), word);
    const int id = m_http->get(url, h);
    Pending p; p.type = ReqSearchUser; p.ctx = ctx;
    m_pending.insert(id, p);
}

void TiebaApi::login(const QString &bduss, const QString &stoken, const QString &tbs)
{
    if (bduss.trimmed().isEmpty()) {
        emit actionFinished(QLatin1String("login"), false,
                            QString::fromUtf8("BDUSS 不能为空"), QVariantMap());
        return;
    }

    // /c/s/login intentionally does not receive the normal BDUSS common
    // parameter. It exchanges the pasted cookie for the canonical user id,
    // nickname, portrait and a fresh tbs token.
    HttpParams b;
    b.add(QLatin1String("bdusstoken"), bduss.trimmed() + QLatin1String("|null"));
    b.add(QLatin1String("stoken"), stoken.trimmed());
    b.add(QLatin1String("user_id"), QLatin1String(""));
    b.add(QLatin1String("channel_id"), QLatin1String(""));
    b.add(QLatin1String("channel_uid"), QLatin1String(""));
    b.add(QLatin1String("_client_version"), QLatin1String("11.10.8.6"));
    b.add(QLatin1String("authsid"), QLatin1String("null"));
    HttpClient::Headers h;
    h << qMakePair(QString(QLatin1String("User-Agent")),
                   QString(QLatin1String("bdtb for Android 11.10.8.6")));
    h << qMakePair(QString(QLatin1String("Cookie")), QString(QLatin1String("ka=open")));
    QVariantMap ctx;
    ctx.insert(QLatin1String("bduss"), bduss.trimmed());
    ctx.insert(QLatin1String("stoken"), stoken.trimmed());
    ctx.insert(QLatin1String("tbs"), tbs.trimmed());
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/s/login"),
                   HttpParams(), b, h, ReqLogin, ctx);
}

// ---- actions ----

void TiebaApi::likeForum(const QString &fid, const QString &kw)
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("fid"), fid);
    b.add(QLatin1String("kw"), kw);
    b.add(QLatin1String("tbs"), ac->tbs());
    QVariantMap ctx; ctx.insert(QLatin1String("fid"), fid); ctx.insert(QLatin1String("kw"), kw);
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/c/forum/like"), miniCommonParams(), b,
                   headersWithUserAgent(QLatin1String("bdtb for Android 7.2.0.0")), ReqLikeForum, ctx);
}

void TiebaApi::unlikeForum(const QString &fid, const QString &kw)
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("fid"), fid);
    b.add(QLatin1String("kw"), kw);
    b.add(QLatin1String("tbs"), ac->tbs());
    QVariantMap ctx; ctx.insert(QLatin1String("fid"), fid); ctx.insert(QLatin1String("kw"), kw);
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/c/forum/unlike"), miniCommonParams(), b,
                   headersWithUserAgent(QLatin1String("bdtb for Android 7.2.0.0")), ReqUnlikeForum, ctx);
}

void TiebaApi::agree(const QString &threadId, const QString &postId, bool cancel)
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("thread_id"), threadId);
    if (!postId.isEmpty()) b.add(QLatin1String("post_id"), postId);
    b.add(QLatin1String("op_type"), cancel ? QLatin1String("1") : QLatin1String("0"));
    b.add(QLatin1String("obj_type"), QLatin1String("1"));
    b.add(QLatin1String("agree_type"), QLatin1String("2"));
    b.add(QLatin1String("cuid_gid"), QLatin1String(""));
    b.add(QLatin1String("forum_id"), QLatin1String(""));
    b.add(QLatin1String("personalized_rec_switch"), QLatin1String("1"));
    b.add(QLatin1String("tbs"), ac->tbs());
    b.add(QLatin1String("stoken"), ac->stoken());
    QVariantMap ctx;
    ctx.insert(QLatin1String("tid"), threadId);
    ctx.insert(QLatin1String("pid"), postId);
    ctx.insert(QLatin1String("cancel"), cancel);
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/c/agree/opAgree"), officialCommonParams(), b,
                   officialHeaders(), ReqAgree, ctx);
}

void TiebaApi::addStore(const QString &tid, const QString &pid)
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    QVariantList arr;
    QVariantMap item;
    item.insert(QLatin1String("threadId"), tid);
    item.insert(QLatin1String("postId"), pid);
    item.insert(QLatin1String("status"), 1);
    arr << item;
    b.add(QLatin1String("data"), QString::fromLatin1(Json::stringify(arr)));
    b.add(QLatin1String("stoken"), ac->stoken());
    QVariantMap ctx; ctx.insert(QLatin1String("tid"), tid); ctx.insert(QLatin1String("pid"), pid);
    ctx.insert(QLatin1String("stored"), 1);
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/c/post/addstore"), officialCommonParams(), b,
                   officialHeaders(), ReqAddStore, ctx);
}

void TiebaApi::removeStore(const QString &tid)
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("tid"), tid);
    b.add(QLatin1String("fid"), QLatin1String("null"));
    b.add(QLatin1String("tbs"), ac->tbs());
    b.add(QLatin1String("stoken"), ac->stoken());
    b.add(QLatin1String("user_id"), ac->uid());
    QVariantMap ctx; ctx.insert(QLatin1String("tid"), tid); ctx.insert(QLatin1String("stored"), 0);
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/c/post/rmstore"), officialCommonParams(), b,
                   officialHeaders(), ReqRemoveStore, ctx);
}

void TiebaApi::loadStoreList()
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("rn"), QLatin1String("50"));
    b.add(QLatin1String("offset"), QLatin1String("0"));
    b.add(QLatin1String("_client_version"), QLatin1String("11.10.8.6"));
    b.add(QLatin1String("stoken"), ac->stoken());
    b.add(QLatin1String("user_id"), ac->uid());
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/f/post/threadstore"), officialCommonParams(), b,
                   officialHeaders(), ReqStoreList, QVariantMap());
}

void TiebaApi::addPost(const QVariantMap &args)
{
    QByteArray body = Proto::addPostRequest(args);
    QVariantMap ctx;
    ctx.insert(QLatin1String("action"), QLatin1String("addPost"));
    sendProto(QLatin1String("/c/c/post/add?cmd=309731&format=protobuf"), body, true, QString(), ReqAddPost, ctx);
}

void TiebaApi::reportPost(const QString &postId)
{
    AccountManager *ac = AccountManager::instance();
    HttpParams b;
    b.add(QLatin1String("category"), QLatin1String("1"));
    b.add(QLatin1String("pid"), postId);
    b.add(QLatin1String("stoken"), ac->stoken());
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/f/ueg/checkjubao"), officialCommonParams(), b,
                   officialHeaders(), ReqReport, QVariantMap());
}

// ---- image upload (chunked multipart to /c/s/uploadPicture, no sign/common params) ----

namespace {
const int kUploadChunkSize = 512000;
const QByteArray kUploadBoundary = QByteArray("--------7da3d81520810*");
}

void TiebaApi::uploadImages(const QVariantList &paths, const QString &forumName)
{
    if (paths.isEmpty()) {
        emit uploadDone(QVariantList(), QString::fromUtf8("请选择图片"));
        return;
    }
    m_uploadPaths = paths;
    m_uploadForum = forumName;
    m_uploadResults.clear();
    m_uploadIndex = 0;
    startUploadFile();
}

void TiebaApi::startUploadFile()
{
    if (m_uploadIndex >= m_uploadPaths.size()) {
        emit uploadDone(m_uploadResults, QString());
        return;
    }
    const QString path = m_uploadPaths.at(m_uploadIndex).toString();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        emit uploadDone(m_uploadResults, QString::fromUtf8("无法读取图片: ") + path);
        return;
    }
    m_uploadFileData = f.readAll();
    f.close();
    if (m_uploadFileData.isEmpty()) {
        emit uploadDone(m_uploadResults, QString::fromUtf8("图片为空: ") + path);
        return;
    }
    m_uploadFileLen = m_uploadFileData.size();
    // Compress oversized images (mirrors the original: scale to <=1080px, JPEG q85)
    QSize decodedSize;
    if (m_uploadFileLen > 5 * 1024 * 1024) {
        QImage img;
        if (img.load(path)) {
            // Harmattan's Qt 4.7 QSize has no scaled(); compute it manually.
            const QSize src = img.size();
            QSize target = src;
            if (src.width() > 1080 || src.height() > 1080) {
                const qreal scale = qMin(qreal(1080) / src.width(),
                                         qreal(1080) / src.height());
                target = QSize(qMax(1, qRound(src.width() * scale)),
                               qMax(1, qRound(src.height() * scale)));
            }
            QImage out = (target == img.size()) ? img
                                                : img.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QBuffer buf;
            buf.open(QIODevice::WriteOnly);
            out.save(&buf, "JPG", 85);
            if (buf.data().size() > 0 && buf.data().size() < m_uploadFileData.size())
                m_uploadFileData = buf.data();
            decodedSize = out.size();
        }
        m_uploadFileLen = m_uploadFileData.size();
    }
    m_uploadFileMd5Hex = QCryptographicHash::hash(m_uploadFileData, QCryptographicHash::Md5).toHex();
    const QSize size = decodedSize.isValid() ? decodedSize : QImageReader(path).size();
    m_uploadWidth = size.width();
    m_uploadHeight = size.height();
    m_uploadTotalChunks = (int)(m_uploadFileLen / kUploadChunkSize
                                + (m_uploadFileLen % kUploadChunkSize == 0 ? 0 : 1));
    if (m_uploadTotalChunks < 1) m_uploadTotalChunks = 1;
    m_uploadChunkNo = 1;
    emit uploadProgress(m_uploadIndex + 1, m_uploadPaths.size(), path);
    startUploadChunk();
}

void TiebaApi::startUploadChunk()
{
    const bool isFinish = (m_uploadChunkNo == m_uploadTotalChunks);
    const qint64 offset = (m_uploadChunkNo - 1) * (qint64)kUploadChunkSize;
    const int curSize = (int)(isFinish ? (m_uploadFileLen - offset) : (qint64)kUploadChunkSize);
    const QByteArray chunk = m_uploadFileData.mid((int)offset, curSize);
    const QByteArray body = buildUploadMultipart(m_uploadChunkNo, isFinish, chunk);

    HttpClient::Headers headers;
    headers << qMakePair(QString::fromLatin1("User-Agent"), QString::fromLatin1("bdtb for Android 12.25.1.0"));
    headers << qMakePair(QString::fromLatin1("Cookie"), uploadCookie());
    headers << qMakePair(QString::fromLatin1("Accept"), QString::fromLatin1("application/json"));
    const QByteArray contentType = QByteArray("multipart/form-data; boundary=") + kUploadBoundary;
    const int id = m_http->postRaw(QString::fromLatin1("http://c.tieba.baidu.com/c/s/uploadPicture"),
                                   body, contentType, headers);
    Pending p;
    p.type = ReqUploadPic;
    m_pending.insert(id, p);
}

QByteArray TiebaApi::buildUploadMultipart(int chunkNo, bool isFinish, const QByteArray &chunk) const
{
    const QByteArray sep = QByteArray("--") + kUploadBoundary + QByteArray("\r\n");
    const QByteArray crlf = QByteArray("\r\n");
    const QByteArray cd = QByteArray("Content-Disposition: form-data; name=\"");
    QByteArray b;
    b += sep + cd + "alt\"" + crlf + crlf + "json" + crlf;
    b += sep + cd + "chunkNo\"" + crlf + crlf + QByteArray::number(chunkNo) + crlf;
    if (!m_uploadForum.isEmpty())
        b += sep + cd + "forum_name\"" + crlf + crlf + m_uploadForum.toUtf8() + crlf;
    b += sep + cd + "groupId\"" + crlf + crlf + "1" + crlf;
    b += sep + cd + "height\"" + crlf + crlf + QByteArray::number(m_uploadHeight) + crlf;
    b += sep + cd + "isFinish\"" + crlf + crlf + QByteArray(isFinish ? "true" : "false") + crlf;
    b += sep + cd + "is_bjh\"" + crlf + crlf + "0" + crlf;
    b += sep + cd + "pic_water_type\"" + crlf + crlf + "2" + crlf;
    b += sep + cd + "resourceId\"" + crlf + crlf + m_uploadFileMd5Hex + QByteArray::number(kUploadChunkSize) + crlf;
    b += sep + cd + "saveOrigin\"" + crlf + crlf + "false" + crlf;
    b += sep + cd + "size\"" + crlf + crlf + QByteArray::number(m_uploadFileLen) + crlf;
    if (!m_uploadForum.isEmpty())
        b += sep + cd + "small_flow_fname\"" + crlf + crlf + m_uploadForum.toUtf8() + crlf;
    b += sep + cd + "width\"" + crlf + crlf + QByteArray::number(m_uploadWidth) + crlf;
    // file part
    b += sep + cd + "chunk\"; filename=\"file\"" + crlf;
    b += QByteArray("Content-Type: application/octet-stream") + crlf + crlf;
    b += chunk + crlf;
    b += QByteArray("--") + kUploadBoundary + QByteArray("--") + crlf;
    return b;
}

QString TiebaApi::uploadCookie() const
{
    const QString id = AppSettings::instance()->baiduId();
    return id.isEmpty() ? QString::fromLatin1("ka=open")
                        : QString::fromLatin1("ka=open;BAIDUID=") + id;
}

void TiebaApi::handleUploadResponse(bool ok, int httpStatus, const QByteArray &data)
{
    if (!ok || httpStatus < 200 || httpStatus >= 300) {
        emit uploadDone(m_uploadResults, QString::fromUtf8("网络错误 HTTP %1").arg(httpStatus));
        return;
    }
    bool parseOk = false;
    QVariantMap m = Json::parse(data, &parseOk).toMap();
    if (!parseOk || m.isEmpty()) {
        emit uploadDone(m_uploadResults, QString::fromUtf8("上传响应解析失败"));
        return;
    }
    const QString err = js(m, "error_code");
    if (!err.isEmpty() && err != QLatin1String("0")) {
        const QString msg = js(m, "error_msg");
        emit uploadDone(m_uploadResults, msg.isEmpty() ? err : msg);
        return;
    }
    if (m_uploadChunkNo == m_uploadTotalChunks) {
        const QString picId = js(m, "picId");
        if (picId.isEmpty()) {
            emit uploadDone(m_uploadResults, QString::fromUtf8("服务器未返回图片 ID"));
            return;
        }
        QVariantMap r;
        r.insert(QLatin1String("picId"), picId);
        r.insert(QLatin1String("width"), m_uploadWidth);
        r.insert(QLatin1String("height"), m_uploadHeight);
        m_uploadResults.append(r);
        ++m_uploadIndex;
        startUploadFile();
    } else {
        ++m_uploadChunkNo;
        startUploadChunk();
    }
}

// ---- feeds ----

void TiebaApi::loadReplyMe(int pn)
{
    if (pn < 0) pn = 0;
    HttpParams b;
    b.add(QLatin1String("pn"), QString::number(pn));
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/u/feed/replyme"), newCommonParams(), b,
                   officialHeaders(), ReqReplyMe, QVariantMap());
}

void TiebaApi::loadAtMe(int pn)
{
    if (pn < 0) pn = 0;
    HttpParams b;
    b.add(QLatin1String("pn"), QString::number(pn));
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/u/feed/atme"), newCommonParams(), b,
                   officialHeaders(), ReqAtMe, QVariantMap());
}

void TiebaApi::loadMsg()
{
    HttpParams b;
    b.add(QLatin1String("bookmark"), QLatin1String("1"));
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/s/msg"), newCommonParams(), b,
                   officialHeaders(), ReqMsg, QVariantMap());
}

// ---- user / forum ----

void TiebaApi::loadUserProfile(const QString &uid)
{
    AccountManager *ac = AccountManager::instance();
    const bool isSelf = (uid.isEmpty() || uid == ac->uid());
    const QString target = isSelf ? ac->uid() : uid;
    QByteArray body = Proto::profileRequest(target, isSelf, 1);
    sendProto(QLatin1String("/c/u/user/profile?cmd=303012&format=protobuf"), body, true, QString(), ReqUserProfile, QVariantMap());
}

void TiebaApi::loadUserPost(const QString &uid, bool isThread, int pn)
{
    HttpParams b;
    b.add(QLatin1String("uid"), uid);
    b.add(QLatin1String("pn"), QString::number(pn));
    b.add(QLatin1String("is_thread"), isThread ? QLatin1String("1") : QLatin1String("0"));
    b.add(QLatin1String("rn"), QLatin1String("20"));
    b.add(QLatin1String("need_content"), QLatin1String("1"));
    QVariantMap ctx;
    ctx.insert(QLatin1String("isThread"), isThread);
    sendSignedForm(QLatin1String("http://c.tieba.baidu.com/c/u/feed/userpost"), miniCommonParams(), b,
                   headersWithUserAgent(QLatin1String("bdtb for Android 7.2.0.0")), ReqUserPost, ctx);
}

void TiebaApi::loadForumDetail(const QString &forumId)
{
    QByteArray body = Proto::forumDetailRequest(forumId.toLongLong());
    sendProto(QLatin1String("/c/f/forum/getforumdetail?cmd=303021&format=protobuf"), body, true, QString(), ReqForumDetail, QVariantMap());
}

// ---- misc ----

void TiebaApi::cancelAll()
{
    m_http->cancelAll();
}

// ---- response dispatch ----

bool TiebaApi::isError(const QVariantMap &m, QString *msg)
{
    // error.errno (proto) / error_code (JSON c.tieba) / no (web)
    const QVariantMap err = m.value(QLatin1String("error")).toMap();
    qlonglong errnoVal = err.value(QLatin1String("errno")).toLongLong();
    QString errmsg = err.value(QLatin1String("errmsg")).toString();
    QString errCode = m.value(QLatin1String("error_code")).toString();
    QString errMsg2 = m.value(QLatin1String("error_msg")).toString();
    qlonglong no = m.value(QLatin1String("no")).toLongLong();
    QString webErr = m.value(QLatin1String("error")).toString();

    const QString resolved = !errmsg.isEmpty() ? errmsg :
                             (!errMsg2.isEmpty() ? errMsg2 : webErr);
    const bool auth = (errCode == QLatin1String("401") || errCode == QLatin1String("401001") ||
                       errCode == QLatin1String("400021") || errnoVal == 401 ||
                       resolved.contains(QString::fromUtf8("登录")) ||
                       resolved.contains(QString::fromUtf8("BDUSS"), Qt::CaseInsensitive) ||
                       resolved.contains(QString::fromUtf8("STOKEN"), Qt::CaseInsensitive) ||
                       resolved.contains(QString::fromUtf8("凭证")));
    if (errnoVal != 0) {
        if (msg) *msg = resolved.isEmpty() ? QString::number(errnoVal) : resolved;
        if (auth) emit loginExpired();
        return true;
    }
    if (!errCode.isEmpty() && errCode != QLatin1String("0")) {
        if (msg) *msg = resolved;
        if (auth) emit loginExpired();
        return true;
    }
    if (no != 0) {
        if (msg) *msg = resolved.isEmpty() ? QString::number(no) : resolved;
        if (auth) emit loginExpired();
        return true;
    }
    return false;
}

// ---- offline page cache helpers ----

QString TiebaApi::cacheKey(const QString &prefix, const QVariantMap &ctx) const
{
    QStringList parts;
    parts << prefix;
    const QString kw = js(ctx, "kw");
    const QString tid = js(ctx, "tid");
    const QString pid = js(ctx, "pid");
    const QString forumId = js(ctx, "forumId");
    const QString sortType = js(ctx, "sortType");
    const QString seeLz = ctx.value(QLatin1String("seeLz")).toBool() ? QLatin1String("1") : QLatin1String("0");
    const QString reverse = ctx.value(QLatin1String("reverse")).toBool() ? QLatin1String("1") : QLatin1String("0");
    if (!kw.isEmpty()) parts << kw;
    if (!tid.isEmpty()) parts << tid;
    if (!pid.isEmpty()) parts << pid;
    if (!forumId.isEmpty()) parts << forumId;
    if (!sortType.isEmpty()) parts << sortType;
    if (!tid.isEmpty()) parts << (QLatin1String("lz") + seeLz) << (QLatin1String("rev") + reverse);
    parts << QString::number(ctx.value(QLatin1String("pn")).toInt());
    return parts.join(QLatin1String(":"));
}

void TiebaApi::cachePageData(const QString &prefix, const QVariantMap &ctx, const QVariantMap &data)
{
    Database::instance()->cacheJson(cacheKey(prefix, ctx), Json::stringify(data));
}

QVariantMap TiebaApi::loadCachedPage(const QString &prefix, const QVariantMap &ctx) const
{
    const QByteArray json = Database::instance()->cachedJson(cacheKey(prefix, ctx));
    if (json.isEmpty()) return QVariantMap();
    return Json::parse(json).toMap();
}

void TiebaApi::handleResponse(int type, const QVariantMap &ctx, bool ok, int status,
                              const QByteArray &data, const QString &errString)
{
    if (!ok || status != 200) {
        // Transport-level failure (status 0 = DNS/SSL/connection): retry once
        // over plain HTTP before giving up. This is what makes forum and
        // thread pages work on stock Harmattan TLS stacks.
        if (status == 0 && !m_preferInsecure &&
            (type == ReqFrsPage || type == ReqThreadPage || type == ReqSubFloor ||
             type == ReqPersonalized)) {
            retryProtoOverHttp(type, ctx);
            return;
        }
        if ((status == 401 || status == 403) && type != ReqLogin)
            emit loginExpired();
        QString err = QString::fromUtf8("网络错误 (%1)").arg(status);
        if (!errString.isEmpty())
            err += QLatin1String(" ") + errString;
        switch (type) {
        case ReqFollowedForums: {
            const QVariantList cached = Database::instance()->forumCache();
            QVariantList out;
            for (int i = 0; i < cached.size(); ++i) {
                const QVariantMap f = cached.at(i).toMap();
                QVariantMap item;
                item.insert(QLatin1String("fid"), f.value(QLatin1String("fid")));
                item.insert(QLatin1String("name"), f.value(QLatin1String("name")));
                item.insert(QLatin1String("level"), f.value(QLatin1String("level")));
                item.insert(QLatin1String("offline"), 1);
                out << item;
            }
            emit followedForumsReady(out, out.isEmpty() ? err : QString());
            break;
        }
        case ReqFrsPage: {
            QVariantMap c = loadCachedPage(QLatin1String("frs"), ctx);
            if (!c.isEmpty()) { c.insert(QLatin1String("offline"), 1); emit frsPageReady(c, QString()); }
            else emit frsPageReady(QVariantMap(), err);
            break;
        }
        case ReqThreadPage: {
            QVariantMap c = loadCachedPage(QLatin1String("pb"), ctx);
            if (!c.isEmpty()) { c.insert(QLatin1String("offline"), 1); emit threadPageReady(c, QString()); }
            else emit threadPageReady(QVariantMap(), err);
            break;
        }
        case ReqSubFloor: {
            QVariantMap c = loadCachedPage(QLatin1String("floor"), ctx);
            if (!c.isEmpty()) { c.insert(QLatin1String("offline"), 1); emit subFloorReady(c, QString()); }
            else emit subFloorReady(QVariantMap(), err);
            break;
        }
        case ReqSearchThread: emit searchThreadsReady(QVariantList(), false, err); break;
        case ReqPersonalized: emit personalizedReady(QVariantList(), false, err); break;
        case ReqSearchForum: emit searchForumsReady(QVariantList(), err); break;
        case ReqSearchUser: emit searchUsersReady(QVariantList(), err); break;
        case ReqStoreList: emit storeListReady(QVariantList(), err); break;
        case ReqReplyMe: emit replyMeReady(QVariantList(), err); break;
        case ReqAtMe: emit atMeReady(QVariantList(), err); break;
        case ReqMsg: emit msgReady(QVariantMap(), err); break;
        case ReqUserProfile: emit userProfileReady(QVariantMap(), err); break;
        case ReqUserPost: emit userPostReady(QVariantList(), false, err); break;
        case ReqForumDetail: emit forumDetailReady(QVariantMap(), err); break;
        case ReqLikeForum: emit actionFinished(QLatin1String("like"), false, err, ctx); break;
        case ReqUnlikeForum: emit actionFinished(QLatin1String("unlike"), false, err, ctx); break;
        case ReqAgree: emit actionFinished(QLatin1String("agree"), false, err, ctx); break;
        case ReqAddStore: case ReqRemoveStore: emit actionFinished(QLatin1String("store"), false, err, ctx); break;
        case ReqAddPost: emit actionFinished(QLatin1String("addPost"), false, err, QVariantMap()); break;
        case ReqReport: emit actionFinished(QLatin1String("report"), false, err, QVariantMap()); break;
        case ReqLogin: emit actionFinished(QLatin1String("login"), false, err, QVariantMap()); break;
        }
        return;
    }

    switch (type) {
    case ReqFollowedForums: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        if (err.isEmpty()) {
            QVariantList list = m.value(QLatin1String("forum_info")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap f = list.at(i).toMap();
                QVariantMap item;
                item.insert(QLatin1String("fid"), js(f, "forum_id"));
                item.insert(QLatin1String("name"), js(f, "forum_name"));
                item.insert(QLatin1String("avatar"), js(f, "avatar"));
                item.insert(QLatin1String("level"), js(f, "user_level"));
                out << item;
            }
            QList<QVariantMap> cache;
            for (int i = 0; i < out.size(); ++i)
                cache << out.at(i).toMap();
            Database::instance()->replaceForumCache(cache);
        }
        emit followedForumsReady(out, err);
        break;
    }
    case ReqPersonalized: {
        QVariantMap m = Proto::personalizedResponse(data);
        QString err;
        if (m.isEmpty()) emit personalizedReady(QVariantList(), false, QString::fromUtf8("推荐数据解析失败"));
        else if (isError(m, &err)) { qWarning("Personalized api error: %s", qPrintable(err)); emit personalizedReady(QVariantList(), false, err); }
        else {
            const QVariantList threads = m.value(QLatin1String("threads")).toList();
            // The response carries no Page block; keep paging while the server
            // still returns cards.
            emit personalizedReady(threads, !threads.isEmpty(), QString());
        }
        break;
    }
    case ReqFrsPage: {
        qDebug("FRS parse begin: %d bytes, head=%s", data.size(),
               qPrintable(QString::fromLatin1(data.left(8).toHex())));
        QVariantMap m = Proto::frsPageResponse(data);
        QString err;
        if (m.isEmpty()) { qWarning("FRS parse FAILED (empty result)"); emit frsPageReady(QVariantMap(), QString::fromUtf8("贴吧数据解析失败")); }
        else if (isError(m, &err)) { qWarning("FRS api error: %s", qPrintable(err)); emit frsPageReady(QVariantMap(), err); }
        else {
            qDebug("FRS page ok: %d threads, forum=%s",
                   m.value(QLatin1String("threads")).toList().size(),
                   qPrintable(js(m.value(QLatin1String("forum")).toMap(), "name")));
            cachePageData(QLatin1String("frs"), ctx, m);
            emit frsPageReady(m, QString());
        }
        break;
    }
    case ReqThreadPage: {
        qDebug("PB parse begin: %d bytes, head=%s", data.size(),
               qPrintable(QString::fromLatin1(data.left(8).toHex())));
        QVariantMap m = Proto::pbPageResponse(data);
        QString err;
        if (m.isEmpty()) { qWarning("PB parse FAILED (empty result)"); emit threadPageReady(QVariantMap(), QString::fromUtf8("帖子数据解析失败")); }
        else if (isError(m, &err)) { qWarning("PB api error: %s", qPrintable(err)); emit threadPageReady(QVariantMap(), err); }
        else {
            qDebug("PB page ok: %d posts", m.value(QLatin1String("posts")).toList().size());
            cachePageData(QLatin1String("pb"), ctx, m);
            emit threadPageReady(m, QString());
        }
        break;
    }
    case ReqSubFloor: {
        QVariantMap m = Proto::pbFloorResponse(data);
        QString err;
        if (m.isEmpty()) emit subFloorReady(QVariantMap(), QString::fromUtf8("楼中楼数据解析失败"));
        else if (isError(m, &err)) emit subFloorReady(QVariantMap(), err);
        else {
            cachePageData(QLatin1String("floor"), ctx, m);
            emit subFloorReady(m, QString());
        }
        break;
    }
    case ReqSearchThread: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        bool hasMore = false;
        if (err.isEmpty()) {
            QVariantMap d = m.value(QLatin1String("data")).toMap();
            hasMore = d.value(QLatin1String("has_more")).toInt() == 1;
            QVariantList list = d.value(QLatin1String("post_list")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap p = list.at(i).toMap();
                QVariantMap u = p.value(QLatin1String("user")).toMap();
                QVariantList media = p.value(QLatin1String("media")).toList();
                QVariantList mediaOut;
                for (int k = 0; k < media.size(); ++k) {
                    QVariantMap mm = media.at(k).toMap();
                    QVariantMap mo2;
                    mo2.insert(QLatin1String("type"), js(mm, "type"));
                    QString thumb = js(mm, "big_pic");
                    if (thumb.isEmpty()) thumb = js(mm, "small_pic");
                    if (thumb.isEmpty()) thumb = js(mm, "water_pic");
                    mo2.insert(QLatin1String("thumb"), thumb);
                    mo2.insert(QLatin1String("original"), js(mm, "big_pic"));
                    mo2.insert(QLatin1String("width"), js(mm, "width"));
                    mo2.insert(QLatin1String("height"), js(mm, "height"));
                    mediaOut << mo2;
                }
                QVariantMap item;
                item.insert(QLatin1String("tid"), js(p, "tid"));
                item.insert(QLatin1String("pid"), js(p, "pid"));
                item.insert(QLatin1String("title"), js(p, "title"));
                item.insert(QLatin1String("abstract"), js(p, "content"));
                item.insert(QLatin1String("time"), js(p, "time"));
                item.insert(QLatin1String("replyNum"), js(p, "post_num"));
                item.insert(QLatin1String("likeNum"), js(p, "like_num"));
                item.insert(QLatin1String("forumName"), js(p, "forum_name"));
                item.insert(QLatin1String("forumId"), js(p, "forum_id"));
                QString name = js(u, "show_nickname");
                if (name.isEmpty()) name = js(u, "user_name");
                item.insert(QLatin1String("author"), name);
                item.insert(QLatin1String("portrait"), js(u, "portrait"));
                item.insert(QLatin1String("media"), mediaOut);
                out << item;
            }
        }
        emit searchThreadsReady(out, hasMore, err);
        break;
    }
    case ReqSearchForum: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        if (err.isEmpty()) {
            QVariantMap d = m.value(QLatin1String("data")).toMap();
            QVariantList list = d.value(QLatin1String("fuzzyMatch")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap f = list.at(i).toMap();
                QVariantMap item;
                item.insert(QLatin1String("fid"), js(f, "forum_id"));
                item.insert(QLatin1String("name"), js(f, "forum_name"));
                item.insert(QLatin1String("nameShow"), js(f, "forum_name_show"));
                item.insert(QLatin1String("avatar"), js(f, "avatar"));
                item.insert(QLatin1String("intro"), js(f, "intro"));
                item.insert(QLatin1String("concernNum"), js(f, "concern_num"));
                item.insert(QLatin1String("postNum"), js(f, "post_num"));
                item.insert(QLatin1String("hasConcerned"), js(f, "has_concerned"));
                out << item;
            }
        }
        emit searchForumsReady(out, err);
        break;
    }
    case ReqSearchUser: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        if (err.isEmpty()) {
            QVariantMap d = m.value(QLatin1String("data")).toMap();
            QVariantList list = d.value(QLatin1String("fuzzyMatch")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap u = list.at(i).toMap();
                QVariantMap item;
                item.insert(QLatin1String("uid"), js(u, "id"));
                QString name = js(u, "show_nickname");
                if (name.isEmpty()) name = js(u, "user_nickname");
                if (name.isEmpty()) name = js(u, "name");
                item.insert(QLatin1String("name"), name);
                item.insert(QLatin1String("portrait"), js(u, "portrait"));
                item.insert(QLatin1String("intro"), js(u, "intro"));
                item.insert(QLatin1String("fansNum"), js(u, "fans_num"));
                item.insert(QLatin1String("hasConcerned"), js(u, "has_concerned"));
                out << item;
            }
        }
        emit searchUsersReady(out, err);
        break;
    }
    case ReqLogin: {
        bool parseOk = false;
        const QVariantMap m = Json::parse(data, &parseOk).toMap();
        QString err;
        bool success = parseOk;
        if (!parseOk) {
            err = QString::fromUtf8("登录响应解析失败");
            success = false;
        } else if (isError(m, &err)) {
            success = false;
        }
        QVariantMap out;
        if (success) {
            const QVariantMap user = m.value(QLatin1String("user")).toMap();
            const QVariantMap anti = m.value(QLatin1String("anti")).toMap();
            const QString uid = user.value(QLatin1String("id")).toString();
            if (uid.isEmpty()) {
                success = false;
                err = QString::fromUtf8("登录响应缺少用户 ID");
            } else {
                out.insert(QLatin1String("uid"), uid);
                out.insert(QLatin1String("name"), user.value(QLatin1String("name")));
                out.insert(QLatin1String("portrait"), user.value(QLatin1String("portrait")));
                out.insert(QLatin1String("bduss"), ctx.value(QLatin1String("bduss")));
                out.insert(QLatin1String("stoken"), ctx.value(QLatin1String("stoken")));
                QString tbsValue = anti.value(QLatin1String("tbs")).toString();
                if (tbsValue.isEmpty()) tbsValue = ctx.value(QLatin1String("tbs")).toString();
                out.insert(QLatin1String("tbs"), tbsValue);
                AccountManager::instance()->login(out);
            }
        }
        emit actionFinished(QLatin1String("login"), success, err, out);
        break;
    }
    case ReqLikeForum: case ReqUnlikeForum: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        bool success = parseOk;
        if (!parseOk) err = QString::fromUtf8("关注响应解析失败");
        else if (isError(m, &err)) success = false;
        emit actionFinished(type == ReqLikeForum ? QLatin1String("like") : QLatin1String("unlike"), success, err, ctx);
        break;
    }
    case ReqAgree: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        bool success = parseOk;
        if (!parseOk) err = QString::fromUtf8("点赞响应解析失败");
        else if (isError(m, &err)) success = false;
        emit actionFinished(QLatin1String("agree"), success, err, ctx);
        break;
    }
    case ReqAddStore: case ReqRemoveStore: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        bool success = parseOk;
        if (!parseOk) err = QString::fromUtf8("收藏响应解析失败");
        else if (isError(m, &err)) success = false;
        emit actionFinished(QLatin1String("store"), success, err, ctx);
        break;
    }
    case ReqStoreList: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        if (err.isEmpty()) {
            QVariantList list = m.value(QLatin1String("store_thread")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap s = list.at(i).toMap();
                QVariantMap a = s.value(QLatin1String("author")).toMap();
                QVariantMap item;
                item.insert(QLatin1String("tid"), js(s, "thread_id"));
                item.insert(QLatin1String("title"), js(s, "title"));
                item.insert(QLatin1String("forumName"), js(s, "forum_name"));
                item.insert(QLatin1String("author"), js(a, "name_show"));
                item.insert(QLatin1String("lastTime"), js(s, "last_time"));
                out << item;
            }
        }
        emit storeListReady(out, err);
        break;
    }
    case ReqReplyMe: case ReqAtMe: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        if (err.isEmpty()) {
            QVariantList list = m.value(type == ReqReplyMe ? QLatin1String("reply_list") : QLatin1String("at_list")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap msg = list.at(i).toMap();
                QVariantMap replyer = msg.value(QLatin1String("replyer")).toMap();
                QVariantMap item;
                item.insert(QLatin1String("threadId"), js(msg, "thread_id"));
                item.insert(QLatin1String("postId"), js(msg, "post_id"));
                item.insert(QLatin1String("title"), js(msg, "title"));
                item.insert(QLatin1String("content"), js(msg, "content"));
                item.insert(QLatin1String("quoteContent"), js(msg, "quote_content"));
                item.insert(QLatin1String("time"), js(msg, "time"));
                item.insert(QLatin1String("forumName"), js(msg, "fname"));
                QString name = js(replyer, "name_show");
                if (name.isEmpty()) name = js(replyer, "name");
                item.insert(QLatin1String("replyer"), name);
                item.insert(QLatin1String("replyerPortrait"), js(replyer, "portrait"));
                out << item;
            }
        }
        if (type == ReqReplyMe) emit replyMeReady(out, err);
        else emit atMeReady(out, err);
        break;
    }
    case ReqMsg: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantMap counts;
        if (err.isEmpty()) {
            QVariantMap msg = m.value(QLatin1String("message")).toMap();
            counts.insert(QLatin1String("replyme"), js(msg, "replyme"));
            counts.insert(QLatin1String("atme"), js(msg, "atme"));
            counts.insert(QLatin1String("fans"), js(msg, "fans"));
        }
        emit msgReady(counts, err);
        break;
    }
    case ReqUserProfile: {
        QVariantMap m = Proto::profileResponse(data);
        QString err;
        if (m.isEmpty()) emit userProfileReady(QVariantMap(), QString::fromUtf8("用户数据解析失败"));
        else if (isError(m, &err)) emit userProfileReady(QVariantMap(), err);
        else emit userProfileReady(m, QString());
        break;
    }
    case ReqUserPost: {
        bool parseOk = false;
        QVariant root = Json::parse(data, &parseOk);
        QVariantMap m = root.toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (isError(m, &err)) err = err;
        QVariantList out;
        if (err.isEmpty()) {
            QVariantList list = m.value(QLatin1String("post_list")).toList();
            for (int i = 0; i < list.size(); ++i) {
                QVariantMap p = list.at(i).toMap();
                QVariantMap item;
                item.insert(QLatin1String("tid"), js(p, "thread_id"));
                item.insert(QLatin1String("pid"), js(p, "post_id"));
                item.insert(QLatin1String("forumId"), js(p, "forum_id"));
                item.insert(QLatin1String("forumName"), js(p, "forum_name"));
                item.insert(QLatin1String("title"), js(p, "title"));
                item.insert(QLatin1String("time"), js(p, "create_time"));
                item.insert(QLatin1String("replyNum"), js(p, "reply_num"));
                item.insert(QLatin1String("userName"), js(p, "name_show"));
                item.insert(QLatin1String("portrait"), js(p, "user_portrait"));
                // abstract from content list
                QVariantList content = p.value(QLatin1String("content")).toList();
                QString abstract;
                for (int k = 0; k < content.size() && abstract.isEmpty(); ++k) {
                    QVariantList pc = content.at(k).toMap().value(QLatin1String("post_content")).toList();
                    for (int j = 0; j < pc.size(); ++j) {
                        QVariantMap pcItem = pc.at(j).toMap();
                        if (js(pcItem, "type") == QLatin1String("0"))
                            abstract += js(pcItem, "text");
                    }
                }
                item.insert(QLatin1String("abstract"), abstract);
                out << item;
            }
        }
        emit userPostReady(out, false, err);
        break;
    }
    case ReqForumDetail: {
        QVariantMap m = Proto::forumDetailResponse(data);
        QString err;
        if (m.isEmpty()) emit forumDetailReady(QVariantMap(), QString::fromUtf8("贴吧详情解析失败"));
        else if (isError(m, &err)) emit forumDetailReady(QVariantMap(), err);
        else emit forumDetailReady(m.value(QLatin1String("forum")).toMap(), QString());
        break;
    }
    case ReqAddPost: {
        QVariantMap m = Proto::addPostResponse(data);
        QString err;
        bool success = false;
        if (m.isEmpty()) err = QString::fromUtf8("发帖响应解析失败");
        else if (isError(m, &err)) success = false;
        else success = true;
        QVariantMap out;
        out.insert(QLatin1String("tid"), m.value(QLatin1String("tid")));
        out.insert(QLatin1String("pid"), m.value(QLatin1String("pid")));
        out.insert(QLatin1String("msg"), m.value(QLatin1String("msg")));
        emit actionFinished(QLatin1String("addPost"), success, err, out);
        break;
    }
    case ReqReport: {
        bool parseOk = false;
        QVariantMap m = Json::parse(data, &parseOk).toMap();
        QString err;
        if (!parseOk) err = QString::fromUtf8("解析失败");
        else if (m.value(QLatin1String("errno")).toInt() != 0) err = js(m, "errmsg");
        QVariantMap out;
        out.insert(QLatin1String("url"), js(m.value(QLatin1String("data")).toMap(), "url"));
        emit actionFinished(QLatin1String("report"), err.isEmpty(), err, out);
        break;
    }
    }
}

void TiebaApi::onHttpFinished(int requestId, bool ok, int httpStatus, const QByteArray &data,
                              const QString &errString)
{
    if (!m_pending.contains(requestId)) return;
    Pending p = m_pending.take(requestId);
    if (p.type == ReqUploadPic) {
        handleUploadResponse(ok, httpStatus, data);
        return;
    }
    QString msg = ok ? QString::fromLatin1("ok %1 bytes").arg(data.size())
                     : QString::fromLatin1("http %1").arg(httpStatus);
    if (!errString.isEmpty())
        msg += QLatin1String(" err: ") + errString;
    LogStore::instance()->append(QLatin1String("http"), QString::number(p.type), msg);
    handleResponse(p.type, p.ctx, ok, httpStatus, data, errString);
}
