#ifndef TIEBA_NOTIFIER_H
#define TIEBA_NOTIFIER_H

// System notifications via org.freedesktop.Notifications (DBus). Harmattan apps
// are suspended when backgrounded, so notifications are only used for in-session
// events (e.g. one-key sign-in completion) as a progressive enhancement.

#include <QObject>
#include <QString>

class Notifier : public QObject
{
    Q_OBJECT
public:
    static Notifier *instance();
    Q_INVOKABLE void notify(const QString &title, const QString &body);

private:
    explicit Notifier(QObject *parent = 0);
    static Notifier *s_instance;
};

#endif // TIEBA_NOTIFIER_H
