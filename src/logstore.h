#ifndef TIEBA_LOGSTORE_H
#define TIEBA_LOGSTORE_H

// Minimal in-memory + file log store for the About > 日志 viewer.
// Keeps the last 500 entries in memory and appends to
// ~/.config/TiebaLite/tieba.log on disk.

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantList>

class LogStore : public QObject
{
    Q_OBJECT
public:
    static LogStore *instance();

    Q_INVOKABLE void append(const QString &level, const QString &tag, const QString &msg);
    Q_INVOKABLE QVariantList read() const;   // newest first
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString path() const;

private:
    explicit LogStore(QObject *parent = 0);
    static LogStore *s_instance;
    QString m_path;
    QList<QVariantList> m_lines; // {time, level, tag, msg}
};

#endif // TIEBA_LOGSTORE_H
