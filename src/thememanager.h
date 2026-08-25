#ifndef TIEBA_THEMEMANAGER_H
#define TIEBA_THEMEMANAGER_H

// Computes the active color palette and spacing/font metrics from AppSettings.
// Exposed to QML as "appTheme" so it does not collide with MeeGo's built-in
// "theme" context object.

#include <QObject>
#include <QColor>

class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode NOTIFY themeChanged)
    Q_PROPERTY(bool dark READ dark NOTIFY themeChanged)

    Q_PROPERTY(QColor background READ background NOTIFY themeChanged)
    Q_PROPERTY(QColor cardBackground READ cardBackground NOTIFY themeChanged)
    Q_PROPERTY(QColor textPrimary READ textPrimary NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY themeChanged)
    Q_PROPERTY(QColor textTertiary READ textTertiary NOTIFY themeChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)
    Q_PROPERTY(QColor accentPressed READ accentPressed NOTIFY themeChanged)
    Q_PROPERTY(QColor divider READ divider NOTIFY themeChanged)
    Q_PROPERTY(QColor toolbarBackground READ toolbarBackground NOTIFY themeChanged)
    Q_PROPERTY(QColor overlay READ overlay NOTIFY themeChanged)
    Q_PROPERTY(QColor badge READ badge NOTIFY themeChanged)

    Q_PROPERTY(int spacingSmall READ spacingSmall NOTIFY themeChanged)
    Q_PROPERTY(int spacingMedium READ spacingMedium NOTIFY themeChanged)
    Q_PROPERTY(int spacingLarge READ spacingLarge NOTIFY themeChanged)
    Q_PROPERTY(int fontSmall READ fontSmall NOTIFY themeChanged)
    Q_PROPERTY(int fontMedium READ fontMedium NOTIFY themeChanged)
    Q_PROPERTY(int fontLarge READ fontLarge NOTIFY themeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY themeChanged)
    Q_PROPERTY(int radius READ radius NOTIFY themeChanged)

public:
    static ThemeManager *instance();

    int mode() const;
    bool dark() const;

    QColor background() const;
    QColor cardBackground() const;
    QColor textPrimary() const;
    QColor textSecondary() const;
    QColor textTertiary() const;
    QColor accent() const;
    QColor accentPressed() const;
    QColor divider() const;
    QColor toolbarBackground() const;
    QColor overlay() const;
    QColor badge() const;

    int spacingSmall() const;
    int spacingMedium() const;
    int spacingLarge() const;
    int fontSmall() const;
    int fontMedium() const;
    int fontLarge() const;
    QString fontFamily() const;
    int radius() const;

public slots:
    void refresh();

signals:
    void themeChanged();

private:
    explicit ThemeManager(QObject *parent = 0);
    static ThemeManager *s_instance;

    int m_mode;
    bool m_dark;
    QColor m_background, m_cardBackground, m_textPrimary, m_textSecondary, m_textTertiary;
    QColor m_accent, m_accentPressed, m_divider, m_toolbarBackground, m_overlay, m_badge;
    int m_density;
    int m_fontScale;
};

#endif // TIEBA_THEMEMANAGER_H
