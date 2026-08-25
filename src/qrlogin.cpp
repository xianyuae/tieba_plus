#include "qrlogin.h"
#include "json.h"

#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QUuid>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkCookieJar>

namespace {

const char *kReferer = "https://passport.baidu.com/";

QNetworkRequest makeRequest(const QUrl &url)
{
    QNetworkRequest req(url);
    req.setRawHeader(QByteArray("Referer"), QByteArray(kReferer));
    req.setRawHeader(QByteArray("User-Agent"),
                     QByteArray("Mozilla/5.0 (Linux; Android 4.4) AppleWebKit/537.36"));
    return req;
}

QString jsonpPayload(const QString &text)
{
    const int b = text.indexOf(QLatin1Char('{'));
    const int e = text.lastIndexOf(QLatin1Char('}'));
    if (b < 0 || e <= b)
        return text;
    return text.mid(b, e - b + 1);
}

QString nowMs() { return QString::number(QDateTime::currentMSecsSinceEpoch()); }

} // namespace

QrLoginController *QrLoginController::s_instance = 0;

QrLoginController *QrLoginController::instance()
{
    if (!s_instance)
        s_instance = new QrLoginController();
    return s_instance;
}

QrLoginController::QrLoginController(QObject *parent)
    : QObject(parent)
    , m_state(Idle)
    , m_startedMs(0)
    , m_pollReply(0)
{
    m_nam.setCookieJar(new QNetworkCookieJar(this));
    m_pollTimer.setInterval(2500);
    connect(&m_pollTimer, SIGNAL(timeout()), this, SLOT(onPollTimeout()));
}

QrLoginController::State QrLoginController::state() const { return m_state; }

QString QrLoginController::statusText() const { return m_statusText; }

void QrLoginController::setState(State s)
{
    if (m_state == s)
        return;
    m_state = s;
    switch (s) {
    case FetchingQr: m_statusText = QString::fromUtf8("正在获取二维码…"); break;
    case WaitingScan: m_statusText = QString::fromUtf8("请使用手机百度 App 扫描二维码"); break;
    case Scanned: m_statusText = QString::fromUtf8("已扫码，请在手机上确认登录"); break;
    case LoggingIn: m_statusText = QString::fromUtf8("正在验证登录…"); break;
    case Done: m_statusText = QString::fromUtf8("登录成功"); break;
    case Error: break;
    default: m_statusText.clear(); break;
    }
    emit stateChanged();
}

void QrLoginController::cancel()
{
    m_pollTimer.stop();
    if (m_pollReply) {
        m_pollReply->abort();
        m_pollReply->deleteLater();
        m_pollReply = 0;
    }
    setState(Idle);
}

void QrLoginController::fail(const QString &message)
{
    m_pollTimer.stop();
    m_statusText = message;
    setState(Error);
    emit finished(false, QString(), QString(), message);
}

void QrLoginController::start()
{
    m_pollTimer.stop();
    if (m_pollReply) {
        m_pollReply->abort();
        m_pollReply->deleteLater();
        m_pollReply = 0;
    }
    m_gid = QString(QUuid::createUuid().toString());
    m_gid = m_gid.mid(1, m_gid.length() - 2).toUpper();
    m_sign.clear();
    m_startedMs = QDateTime::currentMSecsSinceEpoch();
    setState(FetchingQr);

    QUrl url(QLatin1String("https://passport.baidu.com/v2/api/getqrcode"));
    url.addQueryItem(QLatin1String("lp"), QLatin1String("pc"));
    url.addQueryItem(QLatin1String("qrloginfrom"), QLatin1String("pc"));
    url.addQueryItem(QLatin1String("gid"), m_gid);
    url.addQueryItem(QLatin1String("apiver"), QLatin1String("v3"));
    url.addQueryItem(QLatin1String("tpl"), QLatin1String("tb"));
    url.addQueryItem(QLatin1String("tt"), nowMs());
    url.addQueryItem(QLatin1String("_"), nowMs());

    QNetworkReply *reply = m_nam.get(makeRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onQrCodeReply()));
}

void QrLoginController::onQrCodeReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    if (m_state != FetchingQr)
        return;
    if (reply->error() != QNetworkReply::NoError) {
        fail(QString::fromUtf8("获取二维码失败: ") + reply->errorString());
        return;
    }
    const QVariantMap m =
        Json::parse(jsonpPayload(QString::fromUtf8(reply->readAll())).toUtf8()).toMap();
    const QString imgurl = m.value(QLatin1String("imgurl")).toString();
    m_sign = m.value(QLatin1String("sign")).toString();
    if (imgurl.isEmpty() || m_sign.isEmpty()) {
        fail(QString::fromUtf8("二维码响应无效"));
        return;
    }

    QUrl img(QLatin1String("https://") + imgurl);
    QNetworkReply *imgReply = m_nam.get(makeRequest(img));
    connect(imgReply, SIGNAL(finished()), this, SLOT(onQrImageReply()));
}

void QrLoginController::onQrImageReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    if (m_state != FetchingQr)
        return;
    if (reply->error() != QNetworkReply::NoError) {
        fail(QString::fromUtf8("下载二维码失败"));
        return;
    }
    const QByteArray data = reply->readAll();
    if (data.isEmpty()) {
        fail(QString::fromUtf8("二维码数据为空"));
        return;
    }
    m_qrFile = QDir::homePath() + QLatin1String("/.cache/TiebaLite/qr.png");
    QDir().mkpath(QDir::homePath() + QLatin1String("/.cache/TiebaLite"));
    QFile f(m_qrFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fail(QString::fromUtf8("无法保存二维码"));
        return;
    }
    f.write(data);
    f.close();

    setState(WaitingScan);
    emit qrImageReady(m_qrFile);
    startPolling();
}

void QrLoginController::startPolling()
{
    m_pollTimer.start();
    onPollTimeout();
}

void QrLoginController::onPollTimeout()
{
    if (m_pollReply)
        return; // previous long-poll still in flight
    if (QDateTime::currentMSecsSinceEpoch() - m_startedMs > 5 * 60 * 1000) {
        fail(QString::fromUtf8("二维码已过期，请刷新"));
        return;
    }
    QUrl url(QLatin1String("https://passport.baidu.com/channel/unicast"));
    url.addQueryItem(QLatin1String("channel_id"), m_sign);
    url.addQueryItem(QLatin1String("tpl"), QLatin1String("tb"));
    url.addQueryItem(QLatin1String("gid"), m_gid);
    url.addQueryItem(QLatin1String("apiver"), QLatin1String("v3"));
    url.addQueryItem(QLatin1String("tt"), nowMs());
    url.addQueryItem(QLatin1String("_"), nowMs());

    m_pollReply = m_nam.get(makeRequest(url));
    connect(m_pollReply, SIGNAL(finished()), this, SLOT(onPollReply()));
}

void QrLoginController::onPollReply()
{
    QNetworkReply *reply = m_pollReply;
    m_pollReply = 0;
    if (!reply)
        return;
    reply->deleteLater();
    if (m_state != WaitingScan && m_state != Scanned)
        return;
    if (reply->error() != QNetworkReply::NoError)
        return; // transient network error - keep polling

    const QVariantMap m =
        Json::parse(jsonpPayload(QString::fromUtf8(reply->readAll())).toUtf8()).toMap();
    const int errno_ = m.value(QLatin1String("errno")).toInt();
    if (errno_ == 1)
        return; // no scan yet
    if (errno_ != 0)
        return;

    const QVariantMap cv =
        Json::parse(m.value(QLatin1String("channel_v")).toString().toUtf8()).toMap();
    const QString v = cv.value(QLatin1String("v")).toString();
    if (cv.value(QLatin1String("status")).toInt() == 1 && v.isEmpty()) {
        setState(Scanned); // scanned, waiting for confirm on phone
        return;
    }
    if (v.isEmpty())
        return;

    m_pollTimer.stop();
    setState(LoggingIn);

    QUrl url(QLatin1String("https://passport.baidu.com/v3/login/main/qrbdusslogin"));
    url.addQueryItem(QLatin1String("bduss"), v);
    url.addQueryItem(QLatin1String("qrcode"), QLatin1String("1"));
    url.addQueryItem(QLatin1String("tpl"), QLatin1String("tb"));
    url.addQueryItem(QLatin1String("apiver"), QLatin1String("v3"));
    url.addQueryItem(QLatin1String("tt"), nowMs());
    url.addQueryItem(QLatin1String("traceid"), QString());
    url.addQueryItem(QLatin1String("time"), nowMs());
    url.addQueryItem(QLatin1String("alg"), QLatin1String("v3"));
    url.addQueryItem(QLatin1String("elapsed"), QLatin1String("1"));
    url.addQueryItem(QLatin1String("u"),
                     QString::fromUtf8(QUrl::toPercentEncoding(
                         QLatin1String("https://tieba.baidu.com/index/tbwise/mine"))));

    QNetworkReply *loginReply = m_nam.get(makeRequest(url));
    connect(loginReply, SIGNAL(finished()), this, SLOT(onLoginReply()));
}

void QrLoginController::onLoginReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    reply->deleteLater();
    if (m_state != LoggingIn)
        return;
    if (reply->error() != QNetworkReply::NoError) {
        fail(QString::fromUtf8("登录交换失败: ") + reply->errorString());
        return;
    }

    const QByteArray body = reply->readAll();

    // BDUSS/STOKEN arrive as cookies on passport.baidu.com.
    QString bduss, stoken;
    QNetworkCookieJar *jar = m_nam.cookieJar();
    if (jar) {
        const QList<QNetworkCookie> cookies =
            jar->cookiesForUrl(QUrl(QLatin1String("https://passport.baidu.com/")));
        for (int j = 0; j < cookies.size(); ++j) {
            const QString name = QString::fromLatin1(cookies.at(j).name());
            if (name == QLatin1String("BDUSS"))
                bduss = QString::fromLatin1(cookies.at(j).value());
            else if (name == QLatin1String("STOKEN"))
                stoken = QString::fromLatin1(cookies.at(j).value());
        }
    }
    const QVariantMap root = Json::parse(stripJsonp(QString::fromUtf8(body)).toUtf8()).toMap();
    const QVariantMap data = root.value(QLatin1String("data")).toMap();
    const QVariantMap session = data.value(QLatin1String("session")).toMap();
    if (bduss.isEmpty())
        bduss = session.value(QLatin1String("bduss")).toString();
    if (stoken.isEmpty())
        stoken = session.value(QLatin1String("stoken")).toString();
    if (bduss.isEmpty()) {
        fail(QString::fromUtf8("扫码登录失败，请重试"));
        return;
    }

    const QVariantMap user = data.value(QLatin1String("user")).toMap();
    setState(Done);
    emit finished(true, bduss, stoken, user.value(QLatin1String("displayName")).toString());
}

QString QrLoginController::stripJsonp(const QString &text)
{
    return jsonpPayload(text);
}
