#include "appsettings.h"
#include <QtCore/QSettings>
#include <QtGui/QColor>

AppSettings *AppSettings::s_instance = 0;

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
}

AppSettings *AppSettings::instance()
{
    if (!s_instance)
        s_instance = new AppSettings();
    return s_instance;
}

static QSettings &settings()
{
    // QSettings has a private copy constructor (Qt 4.7): return a function-local
    // static by reference instead of by value. Lives for the whole process and
    // syncs on destruction (and periodically while the event loop runs).
    static QSettings s(QSettings::UserScope, QLatin1String("TiebaLite"), QLatin1String("tieba"));
    return s;
}

int AppSettings::theme() const { return settings().value(QLatin1String("ui/theme"), 0).toInt(); }
void AppSettings::setTheme(int theme)
{
    if (theme != this->theme()) {
        settings().setValue(QLatin1String("ui/theme"), theme);
        emit settingsChanged();
    }
}

QString AppSettings::accent() const { return settings().value(QLatin1String("ui/accent"), QLatin1String("#00a2ff")).toString(); }
void AppSettings::setAccent(const QString &color)
{
    const QColor parsed(color);
    const QString normalized = parsed.isValid() ? parsed.name() : QLatin1String("#00a2ff");
    if (normalized != this->accent()) {
        settings().setValue(QLatin1String("ui/accent"), normalized);
        emit settingsChanged();
    }
}

int AppSettings::density() const { return settings().value(QLatin1String("ui/density"), 1).toInt(); }
void AppSettings::setDensity(int density)
{
    density = qBound(0, density, 2);
    if (density != this->density()) {
        settings().setValue(QLatin1String("ui/density"), density);
        emit settingsChanged();
    }
}

int AppSettings::fontScale() const { return settings().value(QLatin1String("ui/font_scale"), 100).toInt(); }
void AppSettings::setFontScale(int percent)
{
    percent = qBound(80, percent, 160);
    if (percent != this->fontScale()) {
        settings().setValue(QLatin1String("ui/font_scale"), percent);
        emit settingsChanged();
    }
}

bool AppSettings::immersive() const { return settings().value(QLatin1String("ui/immersive"), false).toBool(); }
void AppSettings::setImmersive(bool on)
{
    if (on != this->immersive()) {
        settings().setValue(QLatin1String("ui/immersive"), on);
        emit settingsChanged();
    }
}

bool AppSettings::autoSignOnLaunch() const { return settings().value(QLatin1String("sign/auto_on_launch"), false).toBool(); }
void AppSettings::setAutoSignOnLaunch(bool on)
{
    if (on != this->autoSignOnLaunch()) {
        settings().setValue(QLatin1String("sign/auto_on_launch"), on);
        emit settingsChanged();
    }
}

bool AppSettings::onlyWifiImages() const { return settings().value(QLatin1String("ui/only_wifi_images"), false).toBool(); }
void AppSettings::setOnlyWifiImages(bool on)
{
    if (on != this->onlyWifiImages()) {
        settings().setValue(QLatin1String("ui/only_wifi_images"), on);
        emit settingsChanged();
    }
}

QString AppSettings::baiduId() const { return settings().value(QLatin1String("account/baidu_id"), QString()).toString(); }
void AppSettings::setBaiduId(const QString &id)
{
    if (id != this->baiduId()) {
        settings().setValue(QLatin1String("account/baidu_id"), id);
        emit settingsChanged();
    }
}
