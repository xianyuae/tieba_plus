#include "httpclient.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <zlib.h>

// Harmattan ships an ancient OpenSSL (0.9.8, TLS 1.0 only) and a stale CA
// store. Modern Baidu endpoints negotiate TLS anyway or fail; we therefore
// ignore certificate errors on every reply and log the outcome loudly so
// failures are diagnosable from the PC over SSH.

namespace {

static bool netVerbose()
{
    static int v = -1;
    if (v < 0)
        v = qgetenv("TIEBA_DEBUG_NET").isEmpty() ? 0 : 1;
    return v == 1;
}

// Raw-deflate inflate (no zlib/gzip wrapper). Returns false on any error.
static bool inflateRaw(const QByteArray &in, QByteArray *out)
{
    if (in.isEmpty()) return false;
    z_stream strm;
    qMemSet(&strm, 0, sizeof(strm));
    if (inflateInit2(&strm, -15) != Z_OK) return false;
    out->clear();
    strm.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(in.constData()));
    strm.avail_in = static_cast<uInt>(in.size());
    char buf[16384];
    bool ok = false;
    for (;;) {
        strm.next_out = reinterpret_cast<Bytef *>(buf);
        strm.avail_out = sizeof(buf);
        const int rc = inflate(&strm, Z_NO_FLUSH);
        out->append(buf, sizeof(buf) - strm.avail_out);
        if (rc == Z_STREAM_END) { ok = true; break; }
        if (rc != Z_OK) break; // Z_DATA_ERROR etc.
        if (strm.avail_in == 0) { ok = !out->isEmpty(); break; } // truncated stream
    }
    inflateEnd(&strm);
    return ok && !out->isEmpty();
}

// Detects gzip payloads (some Baidu CDN nodes force gzip even without an
// Accept-Encoding request header; Qt4's QNAM does not auto-decompress).
static QByteArray maybeGunzip(const QByteArray &body)
{
    if (body.size() <= 18)
        return body;
    if (static_cast<unsigned char>(body.at(0)) != 0x1f ||
        static_cast<unsigned char>(body.at(1)) != 0x8b)
        return body;

    // Parse gzip member header per RFC 1952 to find where deflate data starts.
    int flg = static_cast<unsigned char>(body.at(3));
    int pos = 10;
    if (flg & 0x04) { // FEXTRA
        if (pos + 2 > body.size()) return body;
        const int xlen = static_cast<unsigned char>(body.at(pos)) |
                         (static_cast<unsigned char>(body.at(pos + 1)) << 8);
        pos += 2 + xlen;
    }
    if (flg & 0x08) { // FNAME
        while (pos < body.size() && body.at(pos) != '\0') ++pos;
        ++pos;
    }
    if (flg & 0x10) { // FCOMMENT
        while (pos < body.size() && body.at(pos) != '\0') ++pos;
        ++pos;
    }
    if (flg & 0x02) pos += 2; // FHCRC
    if (pos >= body.size() - 8) return body;

    QByteArray deflated;
    QByteArray plain;
    if (!inflateRaw(body.mid(pos, body.size() - 8 - pos), &plain))
        return body;
    return plain;
}

} // namespace


HttpClient::HttpClient(QObject *parent)
    : QObject(parent), m_nextId(1)
{
    m_nam = new QNetworkAccessManager(this);
}

HttpClient::~HttpClient()
{
}

int HttpClient::get(const QString &url, const Headers &headers)
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    for (int i = 0; i < headers.size(); ++i)
        req.setRawHeader(headers.at(i).first.toUtf8(), headers.at(i).second.toUtf8());
    return start(req, 0);
}

int HttpClient::postForm(const QString &url, const QByteArray &body, const Headers &headers)
{
    return postRaw(url, body, QByteArray("application/x-www-form-urlencoded"), headers);
}

int HttpClient::postRaw(const QString &url, const QByteArray &body, const QByteArray &contentType,
                        const Headers &headers)
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QString::fromLatin1(contentType)));
    for (int i = 0; i < headers.size(); ++i)
        req.setRawHeader(headers.at(i).first.toUtf8(), headers.at(i).second.toUtf8());
    return start(req, &body);
}

int HttpClient::start(QNetworkRequest &req, const QByteArray *body)
{
    const int id = m_nextId++;
    qDebug("NET >> #%d %s %s (%d bytes)", id,
           body ? "POST" : "GET", qPrintable(req.url().toString()), body ? body->size() : 0);

    QNetworkReply *reply = body ? m_nam->post(req, *body) : m_nam->get(req);

    // Old device CA store / hostname mismatches must not kill requests.
    reply->ignoreSslErrors();
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            reply, SLOT(ignoreSslErrors()));

    m_replies.insert(id, reply);
    m_replyToId.insert(reply, id);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(60000);
    m_timers.insert(id, timer);
    m_timerToId.insert(timer, id);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onProgress(qint64, qint64)));

    timer->start();
    return id;
}

void HttpClient::cleanup(int id)
{
    if (m_timers.contains(id)) {
        QTimer *t = m_timers.value(id);
        t->stop();
        m_timerToId.remove(t);
        t->deleteLater();
        m_timers.remove(id);
    }
    m_replies.remove(id);
}

void HttpClient::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    const int id = m_replyToId.value(reply, -1);
    m_replyToId.remove(reply);

    const bool ok = (reply->error() == QNetworkReply::NoError);
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    const QString errString = ok ? QString() : reply->errorString();

    cleanup(id);
    reply->deleteLater();

    QByteArray data = maybeGunzip(raw);
    if (data.size() != raw.size())
        qDebug("NET GZIP #%d decompressed %d -> %d bytes", id, raw.size(), data.size());

    if (ok)
        qDebug("NET << #%d OK status=%d bytes=%d", id, status, data.size());
    else
        qWarning("NET << #%d FAIL status=%d bytes=%d err=[%s]", id, status, data.size(),
                 qPrintable(errString));
    if (netVerbose()) {
        const QByteArray head = data.left(512);
        qDebug("NET BODY #%d: %s%s", id, head.constData(),
               data.size() > 512 ? "..." : "");
    }

    if (id > 0)
        emit finished(id, ok, status, data, errString);
}

void HttpClient::onTimeout()
{
    QTimer *t = qobject_cast<QTimer *>(sender());
    if (!t) return;
    const int id = m_timerToId.value(t, -1);
    if (id > 0 && m_replies.contains(id)) {
        qWarning("NET TIMEOUT #%d after 60s, aborting", id);
        m_replies.value(id)->abort();
    }
}

void HttpClient::onProgress(qint64 received, qint64 total)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;
    const int id = m_replyToId.value(reply, -1);
    if (id > 0)
        emit progress(id, received, total);
}

void HttpClient::cancel(int requestId)
{
    if (m_replies.contains(requestId))
        m_replies.value(requestId)->abort();
}

void HttpClient::cancelAll()
{
    QList<int> ids = m_replies.keys();
    for (int i = 0; i < ids.size(); ++i)
        m_replies.value(ids.at(i))->abort();
}
