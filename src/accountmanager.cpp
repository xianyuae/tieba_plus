#include "accountmanager.h"
#include "db.h"

AccountManager *AccountManager::s_instance = 0;

AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
{
    refresh();
}

AccountManager *AccountManager::instance()
{
    if (!s_instance)
        s_instance = new AccountManager();
    return s_instance;
}

void AccountManager::refresh()
{
    const QVariantMap next = Database::instance()->currentAccount();
    if (next != m_current) {
        m_current = next;
        emit accountChanged();
    }
}

bool AccountManager::loggedIn() const
{
    return !m_current.value(QLatin1String("bduss")).toString().isEmpty();
}

QString AccountManager::bduss() const { return m_current.value(QLatin1String("bduss")).toString(); }
QString AccountManager::stoken() const { return m_current.value(QLatin1String("stoken")).toString(); }
QString AccountManager::tbs() const { return m_current.value(QLatin1String("tbs")).toString(); }
QString AccountManager::uid() const { return m_current.value(QLatin1String("uid")).toString(); }
QString AccountManager::name() const { return m_current.value(QLatin1String("name")).toString(); }
QString AccountManager::portrait() const { return m_current.value(QLatin1String("portrait")).toString(); }

void AccountManager::login(const QVariantMap &account)
{
    QVariantMap a = account;
    // UID is returned by /c/s/login. Keep manual-login compatibility, but do
    // not invent a numeric UID: an invented value would be sent to the API and
    // silently load another user's data. The login page therefore accepts UID
    // as an optional field and the network login flow fills it automatically.
    a.insert(QLatin1String("uid"), a.value(QLatin1String("uid")).toString().trimmed());
    a.insert(QLatin1String("is_current"), 1);
    Database::instance()->saveAccount(a);
    Database::instance()->setCurrentAccount(a.value(QLatin1String("uid")).toString());
    m_current = Database::instance()->currentAccount();
    emit accountChanged();
}

void AccountManager::switchAccount(const QString &uid)
{
    Database::instance()->setCurrentAccount(uid);
    refresh();
}

void AccountManager::logout()
{
    const QString uid = m_current.value(QLatin1String("uid")).toString();
    if (!m_current.isEmpty()) {
        // Credentials are sensitive; remove the account row instead of
        // leaving BDUSS/STOKEN/tbs recoverable in SQLite after logout.
        Database::instance()->removeAccount(uid);
    }
    const QVariantList remaining = Database::instance()->accounts();
    if (!remaining.isEmpty()) {
        const QString nextUid = remaining.first().toMap().value(QLatin1String("uid")).toString();
        Database::instance()->setCurrentAccount(nextUid);
        m_current = Database::instance()->currentAccount();
    } else {
        m_current = QVariantMap();
    }
    emit accountChanged();
}

QVariantList AccountManager::accounts()
{
    return Database::instance()->accounts();
}

QVariantMap AccountManager::current()
{
    return m_current;
}
