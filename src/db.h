#ifndef TIEBA_DB_H
#define TIEBA_DB_H

// SQLite persistence layer (Qt 4.7 QSqlDatabase). Tables: accounts, drafts,
// history, search_history, favorites, blacklist_user, blacklist_keyword,
// forum_cache, sign_record.

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>

class Database : public QObject
{
    Q_OBJECT
public:
    static Database *instance();
    bool open(const QString &path);
    bool isOpen() const;

    // Accounts
    Q_INVOKABLE QVariantList accounts() const;
    Q_INVOKABLE QVariantMap currentAccount() const;
    Q_INVOKABLE void saveAccount(const QVariantMap &acc);
    Q_INVOKABLE void removeAccount(const QString &uid);
    Q_INVOKABLE void setCurrentAccount(const QString &uid);

    // Drafts
    Q_INVOKABLE QVariantList drafts() const;
    Q_INVOKABLE int saveDraft(const QVariantMap &d);
    Q_INVOKABLE void removeDraft(int id);

    // History
    Q_INVOKABLE QVariantList history() const;
    Q_INVOKABLE void addHistory(const QVariantMap &h);
    Q_INVOKABLE void clearHistory();

    // Search history
    Q_INVOKABLE QVariantList searchHistory() const;
    Q_INVOKABLE void addSearch(const QString &keyword);
    Q_INVOKABLE void clearSearch();

    // Favorites (collected threads)
    Q_INVOKABLE QVariantList favorites() const;
    Q_INVOKABLE void addFavorite(const QVariantMap &f);
    Q_INVOKABLE void removeFavorite(const QString &tid);
    Q_INVOKABLE bool isFavorite(const QString &tid) const;

    // Blacklist
    Q_INVOKABLE QVariantList blacklistUsers() const;
    Q_INVOKABLE QVariantList blacklistKeywords() const;
    Q_INVOKABLE void addBlacklistUser(const QString &uid, const QString &name);
    Q_INVOKABLE void addBlacklistKeyword(const QString &keyword);
    Q_INVOKABLE void removeBlacklistUser(const QString &uid);
    Q_INVOKABLE void removeBlacklistKeyword(const QString &keyword);

    // Followed-forum cache + sign records
    Q_INVOKABLE QVariantList forumCache() const;
    Q_INVOKABLE void replaceForumCache(const QList<QVariantMap> &forums);
    Q_INVOKABLE bool signedToday(const QString &date) const;
    Q_INVOKABLE void markSigned(const QString &date, int count);

    // Offline page cache (generic key -> JSON; used by TiebaApi for
    // frs/pb/floor pages so cached lists show when offline)
    void cacheJson(const QString &key, const QByteArray &json);
    QByteArray cachedJson(const QString &key) const;

private:
    explicit Database(QObject *parent = 0);
    static Database *s_instance;
    bool m_open;
    QString m_path;
    bool exec(const QString &sql) const;
    QVariantList query(const QString &sql) const;};

#endif // TIEBA_DB_H
