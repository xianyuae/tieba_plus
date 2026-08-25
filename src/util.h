#ifndef TIEBA_UTIL_H
#define TIEBA_UTIL_H

// Small QML-facing helpers: time/formatting, clipboard, external URL, gallery
// save, and a toast signal.

#include <QObject>
#include <QString>

class Util : public QObject
{
    Q_OBJECT
public:
    static Util *instance();

    Q_INVOKABLE QString timeAgo(qlonglong ms);
    Q_INVOKABLE QString formatNumber(qint64 n);
    Q_INVOKABLE void copyText(const QString &text);
    Q_INVOKABLE void openUrl(const QString &url);
    Q_INVOKABLE void saveImageToGallery(const QString &localPath);
    Q_INVOKABLE QString pickImage(); // native file picker, returns local path or ""

signals:
    void toast(const QString &message);

private:
    explicit Util(QObject *parent = 0);
    static Util *s_instance;
};

#endif // TIEBA_UTIL_H
