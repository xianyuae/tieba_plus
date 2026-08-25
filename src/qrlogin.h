#ifndef TIEBA_QRLOGIN_H
#define TIEBA_QRLOGIN_H

// Native Baidu QR-code login (no WebView needed - the old Harmattan WebKit
// cannot render the modern passport pages). Protocol:
//   1. GET /v2/api/getqrcode                 -> {imgurl, sign}
//   2. download https://<imgurl>             -> QR PNG shown on screen
//   3. poll /channel/unicast?channel_id=sign -> {errno, channel_v:{status,v}}
//   4. GET /v3/login/main/qrbdusslogin?v     -> {data:{session:{bduss,stoken}}}
// The BDUSS/STOKEN are then fed into the normal /c/s/login flow.

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>

class QNetworkReply;

class QrLoginController : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY stateChanged)

public:
    enum State { Idle, FetchingQr, WaitingScan, Scanned, LoggingIn, Done, Error };

    static QrLoginController *instance();

    State state() const;
    QString statusText() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void cancel();

signals:
    void stateChanged();
    void qrImageReady(const QString &filePath);
    void finished(bool ok, const QString &bduss, const QString &stoken,
                  const QString &displayName);

private slots:
    void onQrCodeReply();
    void onQrImageReply();
    void onPollReply();
    void onLoginReply();
    void onPollTimeout();

private:
    explicit QrLoginController(QObject *parent = 0);
    void setState(State s);
    void fail(const QString &message);
    void startPolling();
    static QString stripJsonp(const QString &text);

    static QrLoginController *s_instance;
    QNetworkAccessManager m_nam;
    QTimer m_pollTimer;
    State m_state;
    QString m_statusText;
    QString m_gid;
    QString m_sign;
    QString m_qrFile;
    qint64 m_startedMs;
    QNetworkReply *m_pollReply;
};

#endif // TIEBA_QRLOGIN_H
