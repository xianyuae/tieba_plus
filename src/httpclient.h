#ifndef TIEBA_HTTPCLIENT_H
#define TIEBA_HTTPCLIENT_H

// Thin async wrapper over QNetworkAccessManager. Each request gets an integer
// id; the `finished` signal reports (id, ok, httpStatus, body). A 60-second
// per-request timeout aborts the underlying reply. All I/O is non-blocking.

#include <QObject>
#include <QHash>
#include <QList>
#include <QPair>
#include <QString>
#include <QByteArray>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QTimer;

class HttpClient : public QObject
{
    Q_OBJECT
public:
    typedef QList<QPair<QString, QString> > Headers;

    explicit HttpClient(QObject *parent = 0);
    virtual ~HttpClient();

    int get(const QString &url, const Headers &headers = Headers());
    int postForm(const QString &url, const QByteArray &body, const Headers &headers = Headers());
    int postRaw(const QString &url, const QByteArray &body, const QByteArray &contentType,
                const Headers &headers = Headers());

    void cancel(int requestId);
    void cancelAll();

signals:
    void finished(int requestId, bool ok, int httpStatus, const QByteArray &data,
                  const QString &errString);
    void progress(int requestId, qint64 received, qint64 total);

private slots:
    void onReplyFinished();
    void onTimeout();
    void onProgress(qint64 received, qint64 total);

private:
    int start(QNetworkRequest &req, const QByteArray *body);
    void cleanup(int id);

    QNetworkAccessManager *m_nam;
    QHash<int, QNetworkReply *> m_replies;
    QHash<QNetworkReply *, int> m_replyToId;
    QHash<int, QTimer *> m_timers;
    QHash<QTimer *, int> m_timerToId;
    int m_nextId;
};

#endif // TIEBA_HTTPCLIENT_H
