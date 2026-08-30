#include "util.h"
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QFileDialog>
#include <QtGui/QImageReader>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

Util *Util::s_instance = 0;

Util::Util(QObject *parent)
    : QObject(parent)
{
}

Util *Util::instance()
{
    if (!s_instance)
        s_instance = new Util();
    return s_instance;
}

QString Util::timeAgo(qlonglong ms)
{
    if (ms <= 0)
        return QString();
    qlonglong now = QDateTime::currentMSecsSinceEpoch();
    qlonglong diff = now - ms;
    if (diff < 0)
        diff = 0;
    const qlonglong minute = 60 * 1000LL;
    const qlonglong hour = 60 * minute;
    const qlonglong day = 24 * hour;
    if (diff < minute)
        return QString::fromUtf8("刚刚");
    if (diff < hour)
        return QString::fromUtf8("%1 分钟前").arg(diff / minute);
    if (diff < day)
        return QString::fromUtf8("%1 小时前").arg(diff / hour);
    if (diff < 30 * day)
        return QString::fromUtf8("%1 天前").arg(diff / day);
    return QDateTime::fromMSecsSinceEpoch(ms).toString(QString::fromUtf8("yyyy-MM-dd"));
}

QString Util::formatNumber(qint64 n)
{
    if (n >= 100000000)
        return QString::fromUtf8("%1 亿").arg(double(n) / 100000000.0, 0, 'f', 1);
    if (n >= 10000)
        return QString::fromUtf8("%1 万").arg(double(n) / 10000.0, 0, 'f', 1);
    return QString::number(n);
}

void Util::copyText(const QString &text)
{
    QApplication::clipboard()->setText(text);
    emit toast(QString::fromUtf8("已复制到剪贴板"));
}

void Util::showToast(const QString &message)
{
    emit toast(message);
}

void Util::openUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

void Util::saveImageToGallery(const QString &localPath)
{
    const QString pictures = QDir::homePath() + QLatin1String("/MyDocs/Pictures");
    QDir().mkpath(pictures);
    QString base = QFileInfo(localPath).fileName();
    if (QFileInfo(base).suffix().isEmpty()) {
        QByteArray format = QImageReader::imageFormat(localPath).toLower();
        if (format == "jpeg") format = "jpg";
        if (!format.isEmpty())
            base += QLatin1Char('.') + QString::fromLatin1(format);
    }
    QString dest = pictures + QLatin1Char('/') + base;
    int i = 1;
    while (QFile::exists(dest)) {
        dest = pictures + QLatin1Char('/') + QString::number(i++) + QLatin1Char('_') + base;
    }
    if (QFile::copy(localPath, dest))
        emit toast(QString::fromUtf8("已保存到相册"));
    else
        emit toast(QString::fromUtf8("保存失败"));
}

QString Util::pickImage()
{
    // Harmattan shows the standard file picker (blocking, like the native dialog).
    return QFileDialog::getOpenFileName(
        0, QString::fromUtf8("选择图片"), QDir::homePath(),
        QString::fromUtf8("图片 (*.png *.jpg *.jpeg *.gif *.bmp)"));
}
