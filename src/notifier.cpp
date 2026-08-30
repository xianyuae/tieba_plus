#include "notifier.h"
#ifdef TIEBA_HAS_DBUS
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#endif
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>

Notifier *Notifier::s_instance = 0;

Notifier::Notifier(QObject *parent)
    : QObject(parent)
{
}

Notifier *Notifier::instance()
{
    if (!s_instance)
        s_instance = new Notifier();
    return s_instance;
}

void Notifier::notify(const QString &title, const QString &body)
{
#ifdef TIEBA_HAS_DBUS
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QLatin1String("org.freedesktop.Notifications"),
        QLatin1String("/org/freedesktop/Notifications"),
        QLatin1String("org.freedesktop.Notifications"),
        QLatin1String("Notify"));

    QStringList actions;
    QVariantMap hints;
    msg << QVariant(QString::fromUtf8("百度贴吧+"))
        << QVariant(quint32(0))
        << QVariant(QString())
        << QVariant(title)
        << QVariant(body)
        << QVariant(actions)
        << QVariant(hints)
        << QVariant(int(-1));

    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
#else
    Q_UNUSED(title);
    Q_UNUSED(body);
#endif
}
