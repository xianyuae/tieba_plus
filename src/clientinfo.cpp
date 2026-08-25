#include "clientinfo.h"
#include "signutil.h"
#include <QtCore/QSettings>
#include <QtCore/QDateTime>
#include <QtCore/QTime>
#include <QtCore/QByteArray>
#include <cstdlib>

ClientInfo *ClientInfo::s_instance = 0;

static QString randomCuid()
{
    // 40 uppercase hex chars, a common cuid shape.
    return SignUtil::randomHex(20).toUpper();
}

static QString randomAid()
{
    // c3_aid is a short hex token.
    return SignUtil::randomHex(8);
}

static QString randomBaiduId()
{
    // 32 hex chars, uppercase typical of BAIDUID.
    return SignUtil::randomHex(16).toUpper();
}

static QString randomSampleId()
{
    return SignUtil::randomHex(8);
}

static QString randomImei()
{
    // 15 decimal digits.
    return SignUtil::randomDigits(15);
}

static QString randomAndroidId()
{
    // android_id is sent base64-encoded; we just keep a random token.
    return SignUtil::randomHex(16);
}

ClientInfo::ClientInfo(QObject *parent)
    : QObject(parent)
    , m_firstInstallTime(0)
    , m_lastUpdateTime(0)
    , m_activeTimestamp(0)
{
    qsrand(uint(QTime::currentTime().msec() + QDateTime::currentMSecsSinceEpoch() % 0x7fffffff));

    m_model = QLatin1String("Nokia N9");
    m_brand = QLatin1String("Nokia");
    m_osVersion = QLatin1String("26"); // emulated Android 8.0 API level

    refresh();
}

ClientInfo *ClientInfo::instance()
{
    if (!s_instance)
        s_instance = new ClientInfo();
    return s_instance;
}

QString ClientInfo::ensureValue(const QString &key, const QString &/*generatorKey*/, QString (*gen)())
{
    QSettings settings(QSettings::UserScope, QLatin1String("TiebaLite"), QLatin1String("tieba"));
    QString v = settings.value(key).toString();
    if (v.isEmpty()) {
        v = gen();
        settings.setValue(key, v);
    }
    return v;
}

void ClientInfo::refresh()
{
    QSettings settings(QSettings::UserScope, QLatin1String("TiebaLite"), QLatin1String("tieba"));

    m_cuid = settings.value(QLatin1String("device/cuid")).toString();
    if (m_cuid.isEmpty()) { m_cuid = randomCuid(); settings.setValue(QLatin1String("device/cuid"), m_cuid); }

    m_clientId = settings.value(QLatin1String("device/client_id")).toString();
    if (m_clientId.isEmpty()) {
        m_clientId = QLatin1String("wappc_") + SignUtil::currentTimestampMs() + QLatin1Char('_') +
                     QString::number(qrand() % 1000);
        settings.setValue(QLatin1String("device/client_id"), m_clientId);
    }

    m_baiduId = settings.value(QLatin1String("device/baiduid")).toString();
    if (m_baiduId.isEmpty()) { m_baiduId = randomBaiduId(); settings.setValue(QLatin1String("device/baiduid"), m_baiduId); }

    m_androidId = settings.value(QLatin1String("device/android_id")).toString();
    if (m_androidId.isEmpty()) { m_androidId = randomAndroidId(); settings.setValue(QLatin1String("device/android_id"), m_androidId); }

    m_c3Aid = settings.value(QLatin1String("device/c3_aid")).toString();
    if (m_c3Aid.isEmpty()) { m_c3Aid = randomAid(); settings.setValue(QLatin1String("device/c3_aid"), m_c3Aid); }

    m_sampleId = settings.value(QLatin1String("device/sample_id")).toString();
    if (m_sampleId.isEmpty()) { m_sampleId = randomSampleId(); settings.setValue(QLatin1String("device/sample_id"), m_sampleId); }

    m_imei = settings.value(QLatin1String("device/imei")).toString();
    if (m_imei.isEmpty()) { m_imei = randomImei(); settings.setValue(QLatin1String("device/imei"), m_imei); }

    qlonglong now = QDateTime::currentMSecsSinceEpoch();
    if (m_firstInstallTime == 0) {
        m_firstInstallTime = settings.value(QLatin1String("device/first_install_time"), now).toLongLong();
        if (m_firstInstallTime == 0) { m_firstInstallTime = now / 1000; settings.setValue(QLatin1String("device/first_install_time"), m_firstInstallTime); }
    }
    m_lastUpdateTime = settings.value(QLatin1String("device/last_update_time"), now / 1000).toLongLong();
    m_activeTimestamp = now / 1000;
}

QString ClientInfo::cuid() const { return m_cuid; }
QString ClientInfo::clientId() const { return m_clientId; }
QString ClientInfo::baiduId() const { return m_baiduId; }
QString ClientInfo::androidId() const { return m_androidId; }
QString ClientInfo::c3Aid() const { return m_c3Aid; }
QString ClientInfo::sampleId() const { return m_sampleId; }
QString ClientInfo::imei() const { return m_imei; }
QString ClientInfo::model() const { return m_model; }
QString ClientInfo::brand() const { return m_brand; }
QString ClientInfo::osVersion() const { return m_osVersion; }

QString ClientInfo::userAgent() const
{
    return QLatin1String("bdtb for Android ") + clientVersionOfficial();
}

QString ClientInfo::webUserAgent() const
{
    return QLatin1String("Mozilla/5.0 (Linux; Android 8.0; Nokia N9) AppleWebKit/537.36 "
                         "(KHTML, like Gecko) Chrome/80.0.3987.149 Mobile Safari/537.36");
}

qlonglong ClientInfo::firstInstallTime() const { return m_firstInstallTime; }
qlonglong ClientInfo::lastUpdateTime() const { return m_lastUpdateTime; }
qlonglong ClientInfo::activeTimestamp() const { return m_activeTimestamp; }
