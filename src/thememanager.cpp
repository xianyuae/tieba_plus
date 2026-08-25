#include "thememanager.h"
#include "appsettings.h"

ThemeManager *ThemeManager::s_instance = 0;

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_mode(0), m_dark(false), m_density(1), m_fontScale(100)
{
    AppSettings *s = AppSettings::instance();
    connect(s, SIGNAL(settingsChanged()), this, SLOT(refresh()));
    refresh();
}

ThemeManager *ThemeManager::instance()
{
    if (!s_instance)
        s_instance = new ThemeManager();
    return s_instance;
}

void ThemeManager::refresh()
{
    AppSettings *s = AppSettings::instance();
    m_mode = s->theme();
    m_dark = (m_mode == 1 || m_mode == 2);
    m_density = s->density();
    m_fontScale = s->fontScale();
    m_accent = QColor(s->accent());

    if (!m_dark) {
        m_background = QColor(0xf2, 0xf3, 0xf5);
        m_cardBackground = QColor(0xff, 0xff, 0xff);
        m_textPrimary = QColor(0x1a, 0x1a, 0x1a);
        m_textSecondary = QColor(0x8a, 0x8f, 0x99);
        m_textTertiary = QColor(0x6f, 0x76, 0x80);
        m_divider = QColor(0xe6, 0xe8, 0xeb);
        m_toolbarBackground = QColor(0xfa, 0xfa, 0xfc);
        m_overlay = QColor(0, 0, 0, 0);
        m_badge = QColor(0xff, 0x3b, 0x30);
    } else {
        m_background = (m_mode == 2) ? QColor(0, 0, 0) : QColor(0x1a, 0x1b, 0x1e);
        m_cardBackground = (m_mode == 2) ? QColor(0x0d, 0x0d, 0x0f) : QColor(0x24, 0x26, 0x2a);
        m_textPrimary = QColor(0xe8, 0xea, 0xed);
        m_textSecondary = QColor(0x9a, 0xa0, 0xa6);
        m_textTertiary = QColor(0x86, 0x8c, 0x95);
        m_divider = (m_mode == 2) ? QColor(0x1c, 0x1c, 0x1e) : QColor(0x2f, 0x31, 0x36);
        m_toolbarBackground = (m_mode == 2) ? QColor(0x05, 0x05, 0x06) : QColor(0x20, 0x21, 0x24);
        m_overlay = QColor(0, 0, 0, 0);
        m_badge = QColor(0xff, 0x45, 0x38);
    }

    m_accentPressed = m_accent.darker(115);

    emit themeChanged();
}

int ThemeManager::mode() const { return m_mode; }
bool ThemeManager::dark() const { return m_dark; }
QColor ThemeManager::background() const { return m_background; }
QColor ThemeManager::cardBackground() const { return m_cardBackground; }
QColor ThemeManager::textPrimary() const { return m_textPrimary; }
QColor ThemeManager::textSecondary() const { return m_textSecondary; }
QColor ThemeManager::textTertiary() const { return m_textTertiary; }
QColor ThemeManager::accent() const { return m_accent; }
QColor ThemeManager::accentPressed() const { return m_accentPressed; }
QColor ThemeManager::divider() const { return m_divider; }
QColor ThemeManager::toolbarBackground() const { return m_toolbarBackground; }
QColor ThemeManager::overlay() const { return m_overlay; }
QColor ThemeManager::badge() const { return m_badge; }

int ThemeManager::spacingSmall() const
{
    static const int v[3] = { 6, 8, 10 };
    return v[qBound(0, m_density, 2)];
}

int ThemeManager::spacingMedium() const
{
    static const int v[3] = { 10, 14, 18 };
    return v[qBound(0, m_density, 2)];
}

int ThemeManager::spacingLarge() const
{
    static const int v[3] = { 14, 20, 26 };
    return v[qBound(0, m_density, 2)];
}

int ThemeManager::fontSmall() const { return (12 * m_fontScale) / 100; }
int ThemeManager::fontMedium() const { return (15 * m_fontScale) / 100; }
int ThemeManager::fontLarge() const { return (18 * m_fontScale) / 100; }
QString ThemeManager::fontFamily() const { return QLatin1String("MHeiGB18030C-Medium"); }
int ThemeManager::radius() const { return 6; }
