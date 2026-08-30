#ifndef TIEBA_APPSETTINGS_H
#define TIEBA_APPSETTINGS_H

// User preferences (theme, accent color, content density, font scale, toggles)
// persisted in QSettings. Exposed to QML as primitive properties.

#include <QObject>
#include <QString>

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY settingsChanged)          // 0 light, 1 dark, 2 AMOLED
    Q_PROPERTY(QString accent READ accent WRITE setAccent NOTIFY settingsChanged)   // "#rrggbb"
    Q_PROPERTY(int density READ density WRITE setDensity NOTIFY settingsChanged)    // 0 compact, 1 standard, 2 comfortable
    Q_PROPERTY(int fontScale READ fontScale WRITE setFontScale NOTIFY settingsChanged) // percent, 100 default
    Q_PROPERTY(bool immersive READ immersive WRITE setImmersive NOTIFY settingsChanged)
    Q_PROPERTY(bool onlyWifiImages READ onlyWifiImages WRITE setOnlyWifiImages NOTIFY settingsChanged)
    Q_PROPERTY(QString baiduId READ baiduId WRITE setBaiduId NOTIFY settingsChanged) // upload cookie BAIDUID
public:
    static AppSettings *instance();

    int theme() const;
    void setTheme(int theme);
    QString accent() const;
    void setAccent(const QString &color);
    int density() const;
    void setDensity(int density);
    int fontScale() const;
    void setFontScale(int percent);
    bool immersive() const;
    void setImmersive(bool on);
    bool onlyWifiImages() const;
    void setOnlyWifiImages(bool on);
    QString baiduId() const;
    void setBaiduId(const QString &id);

signals:
    void settingsChanged();

private:
    explicit AppSettings(QObject *parent = 0);
    static AppSettings *s_instance;
};

#endif // TIEBA_APPSETTINGS_H
