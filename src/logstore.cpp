#include "logstore.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QTextStream>

LogStore *LogStore::s_instance = 0;

LogStore *LogStore::instance()
{
    if (!s_instance) s_instance = new LogStore();
    return s_instance;
}

LogStore::LogStore(QObject *parent)
    : QObject(parent)
{
    const QString dir = QDir::homePath() + QLatin1String("/.config/TiebaLite");
    QDir().mkpath(dir);
    m_path = dir + QLatin1String("/tieba.log");
}

void LogStore::append(const QString &level, const QString &tag, const QString &msg)
{
    const QString time = QDateTime::currentDateTime().toString(QLatin1String("MM-dd hh:mm:ss"));
    m_lines.prepend(QVariantList() << time << level << tag << msg);
    while (m_lines.size() > 500) m_lines.removeLast();

    QFile f(m_path);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out.setCodec("UTF-8");
        out << time << QLatin1Char(' ') << level << QLatin1Char('[') << tag << QLatin1Char(']')
            << msg << QLatin1Char('\n');
    }
}

QVariantList LogStore::read() const
{
    QVariantList out;
    for (int i = 0; i < m_lines.size(); ++i)
        out << QVariant(m_lines.at(i));
    return out;
}

void LogStore::clear()
{
    m_lines.clear();
    QFile f(m_path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        f.close();
}

QString LogStore::path() const
{
    return m_path;
}
