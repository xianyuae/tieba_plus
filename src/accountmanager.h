#ifndef TIEBA_ACCOUNTMANAGER_H
#define TIEBA_ACCOUNTMANAGER_H

// Multi-account manager. The current account exposes BDUSS/STOKEN/tbs/uid/name/
// portrait for the network layer and UI. Credentials persist in SQLite.

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>

class AccountManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY accountChanged)
    Q_PROPERTY(QString bduss READ bduss NOTIFY accountChanged)
    Q_PROPERTY(QString stoken READ stoken NOTIFY accountChanged)
    Q_PROPERTY(QString tbs READ tbs NOTIFY accountChanged)
    Q_PROPERTY(QString uid READ uid NOTIFY accountChanged)
    Q_PROPERTY(QString name READ name NOTIFY accountChanged)
    Q_PROPERTY(QString portrait READ portrait NOTIFY accountChanged)
public:
    static AccountManager *instance();

    bool loggedIn() const;
    QString bduss() const;
    QString stoken() const;
    QString tbs() const;
    QString uid() const;
    QString name() const;
    QString portrait() const;

    Q_INVOKABLE void login(const QVariantMap &account);
    Q_INVOKABLE void switchAccount(const QString &uid);
    Q_INVOKABLE void logout();
    Q_INVOKABLE QVariantList accounts();
    Q_INVOKABLE QVariantMap current();
    Q_INVOKABLE void refresh();

signals:
    void accountChanged();
    void loginExpired();

private:
    explicit AccountManager(QObject *parent = 0);
    static AccountManager *s_instance;
    QVariantMap m_current;
};

#endif // TIEBA_ACCOUNTMANAGER_H
