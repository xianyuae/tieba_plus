#include <QtGui/QApplication>
#include <QtDeclarative/QDeclarativeContext>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QTextCodec>
#include <QtCore/QDateTime>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include "qmlapplicationviewer.h"

#include "db.h"
#include "clientinfo.h"
#include "accountmanager.h"
#include "appsettings.h"
#include "thememanager.h"
#include "imagecache.h"
#include "notifier.h"
#include "util.h"
#include "tiebaapi.h"
#include "logstore.h"
#include "qrlogin.h"

// Debug output goes to three places:
//  1. stderr  - visible on the PC when the app is started from an SSH session
//               (developer mode) or from Qt Creator: "TIEBA-DBG ..." lines.
//  2. ~/.config/TiebaLite/qml.log on the device.
//  3. LogStore (in-app log viewer under About).
// Set TIEBA_DEBUG_NET=1 to also dump request/response bodies for network calls.
static void tiebaMessageHandler(QtMsgType type, const char *msg)
{
    const QByteArray stamp = QDateTime::currentDateTime()
        .toString(QLatin1String("MM-dd hh:mm:ss.zzz")).toLatin1();
    const char *tag = "TIEBA-DBG";
    fprintf(stderr, "%s [%s] %s\n", tag, stamp.constData(), msg);
    fflush(stderr);

    QFile f(QDir::homePath() + QLatin1String("/.config/TiebaLite/qml.log"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QByteArray line(tag);
        line += " [";
        line += stamp;
        line += "] ";
        line += msg;
        line += '\n';
        f.write(line);
    }
}

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));
    QCoreApplication::setApplicationName(QString::fromUtf8("百度贴吧+"));
    QCoreApplication::setApplicationVersion(QLatin1String("1.0.0"));

    // Route Qt/QML diagnostics to stderr and ~/.config/TiebaLite/qml.log so
    // load errors are visible on device and on desktop.
    qInstallMsgHandler(tiebaMessageHandler);

    // Ensure any C++ tr()/qsTr() fallback decodes source text as UTF-8 rather
    // than the locale codec (GBK on Chinese devices). All user-facing strings
    // now live in qml/tieba/strings.js, which the QML engine reads with
    // QString::fromUtf8 regardless of device locale.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    // Bundle a CJK-capable font so Chinese text renders on every device.
    const QString appDir = QCoreApplication::applicationDirPath();
    QString fontFile;
    const QStringList fontCandidates = QStringList()
        << QDir(appDir).filePath(QLatin1String("fonts/MHei18030C5.ttf"))
        << QDir(appDir).filePath(QLatin1String("../fonts/MHei18030C5.ttf"))
        << QDir(appDir).filePath(QLatin1String("../../fonts/MHei18030C5.ttf"))
        << QLatin1String("/usr/share/fonts/nokia/MHei18030C5.ttf");
    for (int i = 0; i < fontCandidates.size(); ++i) {
        if (QFile::exists(fontCandidates.at(i))) {
            fontFile = fontCandidates.at(i);
            break;
        }
    }
    const int fontId = fontFile.isEmpty() ? -1 : QFontDatabase::addApplicationFont(fontFile);
    if (fontId != -1) {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            QFont font = app->font();
            font.setFamily(families.first());
            font.setStyleHint(QFont::SansSerif);
            app->setFont(font);
        }
    }

    // Translations (i18n/*.qm deployed to /opt/tieba/i18n).
    QTranslator translator;
    QString i18nDir;
    const QStringList i18nCandidates = QStringList()
        << QDir(appDir).filePath(QLatin1String("i18n"))
        << QDir(appDir).filePath(QLatin1String("../i18n"))
        << QDir(appDir).filePath(QLatin1String("../../i18n"));
    for (int i = 0; i < i18nCandidates.size(); ++i) {
        if (QFile::exists(QDir(i18nCandidates.at(i)).filePath(QLatin1String("tieba_en.qm")))) {
            i18nDir = i18nCandidates.at(i);
            break;
        }
    }
    if (i18nDir.isEmpty())
        i18nDir = QDir(appDir).filePath(QLatin1String("../i18n"));
    const QString localeName = QLocale::system().name();
    bool translationLoaded = translator.load(QLatin1String("tieba_") + localeName, i18nDir);
    if (!translationLoaded)
        translationLoaded = translator.load(QLatin1String("tieba_") + localeName.left(2), i18nDir);
    if (!translationLoaded && localeName.left(2) != QLatin1String("zh"))
        translationLoaded = translator.load(QLatin1String("tieba_en"), i18nDir);
    if (translationLoaded)
        app->installTranslator(&translator);

    // Initialize persistence + singletons before QML loads.
    const QString configDir = QDir::homePath() + QLatin1String("/.config/TiebaLite");
    QDir().mkpath(configDir);
    Database::instance()->open(configDir + QLatin1String("/tieba.db"));
    ClientInfo::instance()->refresh();
    AccountManager::instance()->refresh();
    AppSettings::instance();
    ThemeManager::instance();
    ImageCache::instance()->setCacheDir(QDir::homePath() + QLatin1String("/.cache/TiebaLite/images"));
    Notifier::instance();
    Util::instance();
    TiebaApi::instance();
    LogStore::instance();

    QmlApplicationViewer viewer;
    qDebug("TIEBA-BOOT viewer created");
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    QDeclarativeContext *context = viewer.rootContext();
    context->setContextProperty(QLatin1String("qrlogin"), QrLoginController::instance());
    context->setContextProperty(QLatin1String("db"), Database::instance());
    context->setContextProperty(QLatin1String("accounts"), AccountManager::instance());
    context->setContextProperty(QLatin1String("settings"), AppSettings::instance());
    context->setContextProperty(QLatin1String("appTheme"), ThemeManager::instance());
    context->setContextProperty(QLatin1String("img"), ImageCache::instance());
    context->setContextProperty(QLatin1String("notifier"), Notifier::instance());
    context->setContextProperty(QLatin1String("util"), Util::instance());
    context->setContextProperty(QLatin1String("api"), TiebaApi::instance());
    context->setContextProperty(QLatin1String("logstore"), LogStore::instance());

    QString qmlMain = QString::fromLocal8Bit(qgetenv("TIEBA_MAIN_QML"));
    if (qmlMain.isEmpty())
        qmlMain = QLatin1String("qml/tieba/main.qml");
    viewer.setMainQmlFile(qmlMain);
    qDebug("TIEBA-BOOT qml set: %s", qPrintable(qmlMain));
    viewer.showExpanded();
    qDebug("TIEBA-BOOT shown, entering event loop");
    return app->exec();
}
