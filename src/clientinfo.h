#ifndef TIEBA_CLIENTINFO_H
#define TIEBA_CLIENTINFO_H

// Stable device/account-agnostic fingerprint values, persisted in QSettings so
// they survive restarts (the server relies on a stable cuid / client_id).

#include <QObject>
#include <QString>
#include <QtGlobal>

class ClientInfo : public QObject
{
    Q_OBJECT
public:
    static ClientInfo *instance();

    QString cuid() const;
    QString clientId() const;
    QString baiduId() const;
    QString androidId() const;   // base64, mirrors android_id
    QString c3Aid() const;       // cuid_galaxy3 / c3_aid
    QString sampleId() const;
    QString imei() const;
    QString model() const;
    QString brand() const;
    QString osVersion() const;   // emulated Android API level
    QString userAgent() const;   // "bdtb for Android 12.25.1.0"
    QString webUserAgent() const;

    qlonglong firstInstallTime() const;
    qlonglong lastUpdateTime() const;
    qlonglong activeTimestamp() const;

    // Version constants for the official JSON / protobuf clients.
    static QString clientVersionOfficial() { return QLatin1String("12.25.1.0"); }
    static QString clientVersionProto() { return QLatin1String("12.52.1.0"); }

    // Regenerates per-launch values and touches activeTimestamp. Call once at
    // startup after QSettings is ready.
    void refresh();

private:
    explicit ClientInfo(QObject *parent = 0);
    static ClientInfo *s_instance;
    QString ensureValue(const QString &key, const QString &generatorKey, QString (*gen)());

    QString m_cuid;
    QString m_clientId;
    QString m_baiduId;
    QString m_androidId;
    QString m_c3Aid;
    QString m_sampleId;
    QString m_imei;
    QString m_model;
    QString m_brand;
    QString m_osVersion;
    qlonglong m_firstInstallTime;
    qlonglong m_lastUpdateTime;
    qlonglong m_activeTimestamp;
};

#endif // TIEBA_CLIENTINFO_H
