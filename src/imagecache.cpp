#include "imagecache.h"
#include "signutil.h"
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

ImageCache *ImageCache::s_instance = 0;

ImageCache::ImageCache(QObject *parent)
    : QObject(parent)
{
    m_http = new HttpClient(this);
    connect(m_http, SIGNAL(finished(int, bool, int, QByteArray, QString)),
            this, SLOT(onFinished(int, bool, int, QByteArray, QString)));
}

ImageCache *ImageCache::instance()
{
    if (!s_instance)
        s_instance = new ImageCache();
    return s_instance;
}

void ImageCache::setCacheDir(const QString &dir)
{
    m_dir = dir;
    QDir().mkpath(m_dir);
}

QString ImageCache::keyFor(const QString &url) const
{
    return SignUtil::md5(url.toUtf8());
}

QString ImageCache::pathFor(const QString &url) const
{
    return m_dir + QLatin1Char('/') + keyFor(url);
}

QString ImageCache::cachedPath(const QString &url)
{
    const QString p = pathFor(url);
    return QFile::exists(p) ? p : QString();
}

QString ImageCache::fileUrl(const QString &path)
{
    return QUrl::fromLocalFile(path).toString();
}

void ImageCache::load(const QString &url)
{
    if (url.isEmpty()) return;
    const QString p = pathFor(url);
    if (QFile::exists(p)) {
        emit loaded(url, p);
        return;
    }
    if (m_inflight.contains(url))
        return;
    m_inflight.insert(url);
    const int id = m_http->get(url, HttpClient::Headers());
    m_urlByRequest.insert(id, url);
}

void ImageCache::onFinished(int requestId, bool ok, int httpStatus, const QByteArray &data,
                            const QString &errString)
{
    Q_UNUSED(errString);
    const QString url = m_urlByRequest.take(requestId);
    m_inflight.remove(url);
    if (url.isEmpty())
        return;
    if (!ok || httpStatus != 200) {
        emit failed(url);
        return;
    }
    const QString p = pathFor(url);
    QFile f(p);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(data);
        f.close();
        emit loaded(url, p);
    } else {
        emit failed(url);
    }
}

void ImageCache::clear()
{
    m_http->cancelAll();
    m_urlByRequest.clear();
    m_inflight.clear();
    QDir dir(m_dir);
    const QStringList files = dir.entryList(QDir::Files);
    for (int i = 0; i < files.size(); ++i)
        dir.remove(files.at(i));
}
