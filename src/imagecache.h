#ifndef TIEBA_IMAGECACHE_H
#define TIEBA_IMAGECACHE_H

// Two-level image cache: disk (raw bytes keyed by md5(url)) + deferred network
// fetch. QML's Image handles decode; the disk cache avoids re-downloads and
// enables offline display. `cachedPath` is synchronous; `load` is async and
// emits `loaded` with a local path.

#include <QObject>
#include <QHash>
#include <QSet>
#include <QString>
#include <QByteArray>
#include "httpclient.h"

class ImageCache : public QObject
{
    Q_OBJECT
public:
    static ImageCache *instance();
    void setCacheDir(const QString &dir);

    Q_INVOKABLE QString cachedPath(const QString &url);
    Q_INVOKABLE void load(const QString &url);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString fileUrl(const QString &path);

signals:
    void loaded(const QString &url, const QString &path);
    void failed(const QString &url);

private slots:
    void onFinished(int requestId, bool ok, int httpStatus, const QByteArray &data,
                    const QString &errString);

private:
    explicit ImageCache(QObject *parent = 0);
    static ImageCache *s_instance;

    HttpClient *m_http;
    QHash<int, QString> m_urlByRequest;
    QSet<QString> m_inflight;
    QString m_dir;

    QString keyFor(const QString &url) const;
    QString pathFor(const QString &url) const;
};

#endif // TIEBA_IMAGECACHE_H
