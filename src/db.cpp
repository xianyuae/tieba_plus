#include "db.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>
#include <QtCore/QDateTime>

Database *Database::s_instance = 0;

Database::Database(QObject *parent)
    : QObject(parent), m_open(false)
{
}

Database *Database::instance()
{
    if (!s_instance)
        s_instance = new Database();
    return s_instance;
}

bool Database::open(const QString &path)
{
    m_path = path;
    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), QLatin1String("tieba"));
    db.setDatabaseName(path);
    if (!db.open()) {
        m_open = false;
        return false;
    }
    m_open = true;

    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS account ("
        " uid TEXT PRIMARY KEY, name TEXT, portrait TEXT, bduss TEXT, stoken TEXT,"
        " tbs TEXT, level INTEGER DEFAULT 0, is_current INTEGER DEFAULT 0 )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS draft ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT, forum_name TEXT, forum_id TEXT,"
        " thread_id TEXT, thread_title TEXT, floor TEXT, content TEXT, images TEXT,"
        " updated_at INTEGER DEFAULT 0 )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS history ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT, type TEXT, tid TEXT, title TEXT,"
        " author TEXT, forum_name TEXT, forum_id TEXT, updated_at INTEGER DEFAULT 0 )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS search_history ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT, keyword TEXT UNIQUE, updated_at INTEGER DEFAULT 0 )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS favorite ("
        " tid TEXT PRIMARY KEY, title TEXT, author TEXT, forum_name TEXT,"
        " forum_id TEXT, reply_num INTEGER DEFAULT 0, updated_at INTEGER DEFAULT 0 )"));
    // Existing installations predate forum_id. SQLite reports an error when
    // the column already exists, which is safe to ignore here.
    exec(QLatin1String("ALTER TABLE history ADD COLUMN forum_id TEXT"));
    exec(QLatin1String("ALTER TABLE favorite ADD COLUMN forum_id TEXT"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS blacklist_user ("
        " uid TEXT PRIMARY KEY, name TEXT )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS blacklist_keyword ("
        " keyword TEXT PRIMARY KEY )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS forum_cache ("
        " fid TEXT PRIMARY KEY, name TEXT, level TEXT,"
        " updated_at INTEGER DEFAULT 0 )"));
    exec(QLatin1String(
        "CREATE TABLE IF NOT EXISTS page_cache ("
        " key TEXT PRIMARY KEY, data TEXT, updated_at INTEGER DEFAULT 0 )"));
    return true;
}

bool Database::isOpen() const { return m_open; }

bool Database::exec(const QString &sql) const
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    return q.exec(sql);
}

QVariantList Database::query(const QString &sql) const
{
    QVariantList rows;
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    if (!q.exec(sql))
        return rows;
    while (q.next()) {
        QVariantMap row;
        QSqlRecord rec = q.record();
        for (int i = 0; i < rec.count(); ++i)
            row.insert(rec.fieldName(i), q.value(i));
        rows.append(row);
    }
    return rows;
}

// --- Accounts ---
QVariantList Database::accounts() const
{
    return query(QLatin1String("SELECT * FROM account ORDER BY rowid"));
}

QVariantMap Database::currentAccount() const
{
    QVariantList rows = query(QLatin1String("SELECT * FROM account WHERE is_current=1 LIMIT 1"));
    return rows.isEmpty() ? QVariantMap() : rows.first().toMap();
}

void Database::saveAccount(const QVariantMap &acc)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String(
        "INSERT OR REPLACE INTO account (uid, name, portrait, bduss, stoken, tbs, level, is_current) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(acc.value(QLatin1String("uid")).toString());
    q.addBindValue(acc.value(QLatin1String("name")).toString());
    q.addBindValue(acc.value(QLatin1String("portrait")).toString());
    q.addBindValue(acc.value(QLatin1String("bduss")).toString());
    q.addBindValue(acc.value(QLatin1String("stoken")).toString());
    q.addBindValue(acc.value(QLatin1String("tbs")).toString());
    q.addBindValue(acc.value(QLatin1String("level")).toInt());
    q.addBindValue(acc.value(QLatin1String("is_current"), 0).toInt());
    q.exec();
}

void Database::removeAccount(const QString &uid)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("DELETE FROM account WHERE uid=?"));
    q.addBindValue(uid);
    q.exec();
}

void Database::setCurrentAccount(const QString &uid)
{
    exec(QLatin1String("UPDATE account SET is_current=0"));
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("UPDATE account SET is_current=1 WHERE uid=?"));
    q.addBindValue(uid);
    q.exec();
}

// --- Drafts ---
QVariantList Database::drafts() const
{
    return query(QLatin1String("SELECT * FROM draft ORDER BY updated_at DESC"));
}

int Database::saveDraft(const QVariantMap &d)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String(
        "INSERT INTO draft (forum_name, forum_id, thread_id, thread_title, floor, content, images, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(d.value(QLatin1String("forum_name")).toString());
    q.addBindValue(d.value(QLatin1String("forum_id")).toString());
    q.addBindValue(d.value(QLatin1String("thread_id")).toString());
    q.addBindValue(d.value(QLatin1String("thread_title")).toString());
    q.addBindValue(d.value(QLatin1String("floor")).toString());
    q.addBindValue(d.value(QLatin1String("content")).toString());
    q.addBindValue(d.value(QLatin1String("images")).toString());
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    if (!q.exec()) return -1;
    return q.lastInsertId().toInt();
}

void Database::removeDraft(int id)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("DELETE FROM draft WHERE id=?"));
    q.addBindValue(id);
    q.exec();
}

// --- History ---
QVariantList Database::history() const
{
    return query(QLatin1String("SELECT * FROM history ORDER BY updated_at DESC"));
}

void Database::addHistory(const QVariantMap &h)
{
    QSqlDatabase db = QSqlDatabase::database(QLatin1String("tieba"));
    QSqlQuery old(db);
    old.prepare(QLatin1String("DELETE FROM history WHERE type=? AND tid=?"));
    old.addBindValue(h.value(QLatin1String("type")).toString());
    old.addBindValue(h.value(QLatin1String("tid")).toString());
    old.exec();

    QSqlQuery q(db);
    q.prepare(QLatin1String(
        "INSERT INTO history (type, tid, title, author, forum_name, forum_id, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(h.value(QLatin1String("type")).toString());
    q.addBindValue(h.value(QLatin1String("tid")).toString());
    q.addBindValue(h.value(QLatin1String("title")).toString());
    q.addBindValue(h.value(QLatin1String("author")).toString());
    q.addBindValue(h.value(QLatin1String("forum_name")).toString());
    q.addBindValue(h.value(QLatin1String("forum_id")).toString());
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.exec();
    QSqlQuery prune(db);
    prune.exec(QLatin1String("DELETE FROM history WHERE id NOT IN (SELECT id FROM history ORDER BY updated_at DESC LIMIT 500)"));
}

void Database::clearHistory()
{
    exec(QLatin1String("DELETE FROM history"));
}

// --- Search history ---
QVariantList Database::searchHistory() const
{
    return query(QLatin1String("SELECT * FROM search_history ORDER BY updated_at DESC"));
}

void Database::addSearch(const QString &keyword)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String(
        "INSERT OR REPLACE INTO search_history (keyword, updated_at) VALUES (?, ?)"));
    q.addBindValue(keyword);
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.exec();
}

void Database::clearSearch()
{
    exec(QLatin1String("DELETE FROM search_history"));
}

// --- Favorites ---
QVariantList Database::favorites() const
{
    return query(QLatin1String("SELECT * FROM favorite ORDER BY updated_at DESC"));
}

void Database::addFavorite(const QVariantMap &f)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String(
        "INSERT OR REPLACE INTO favorite (tid, title, author, forum_name, forum_id, reply_num, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(f.value(QLatin1String("tid")).toString());
    q.addBindValue(f.value(QLatin1String("title")).toString());
    q.addBindValue(f.value(QLatin1String("author")).toString());
    q.addBindValue(f.value(QLatin1String("forum_name")).toString());
    q.addBindValue(f.value(QLatin1String("forum_id")).toString());
    q.addBindValue(f.value(QLatin1String("reply_num")).toInt());
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.exec();
}

void Database::removeFavorite(const QString &tid)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("DELETE FROM favorite WHERE tid=?"));
    q.addBindValue(tid);
    q.exec();
}

bool Database::isFavorite(const QString &tid) const
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("SELECT 1 FROM favorite WHERE tid=? LIMIT 1"));
    q.addBindValue(tid);
    return q.exec() && q.next();
}

// --- Blacklist ---
QVariantList Database::blacklistUsers() const
{
    return query(QLatin1String("SELECT * FROM blacklist_user"));
}

QVariantList Database::blacklistKeywords() const
{
    return query(QLatin1String("SELECT * FROM blacklist_keyword"));
}

void Database::addBlacklistUser(const QString &uid, const QString &name)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("INSERT OR REPLACE INTO blacklist_user (uid, name) VALUES (?, ?)"));
    q.addBindValue(uid);
    q.addBindValue(name);
    q.exec();
}

void Database::addBlacklistKeyword(const QString &keyword)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("INSERT OR REPLACE INTO blacklist_keyword (keyword) VALUES (?)"));
    q.addBindValue(keyword);
    q.exec();
}

void Database::removeBlacklistUser(const QString &uid)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("DELETE FROM blacklist_user WHERE uid=?"));
    q.addBindValue(uid);
    q.exec();
}

void Database::removeBlacklistKeyword(const QString &keyword)
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("DELETE FROM blacklist_keyword WHERE keyword=?"));
    q.addBindValue(keyword);
    q.exec();
}

// --- Forum cache ---
QVariantList Database::forumCache() const
{
    return query(QLatin1String("SELECT * FROM forum_cache ORDER BY name"));
}

void Database::replaceForumCache(const QList<QVariantMap> &forums)
{
    QSqlDatabase db = QSqlDatabase::database(QLatin1String("tieba"));
    db.transaction();
    QSqlQuery clear(db);
    clear.exec(QLatin1String("DELETE FROM forum_cache"));
    QSqlQuery q(db);
    q.prepare(QLatin1String(
        "INSERT OR REPLACE INTO forum_cache (fid, name, level, updated_at) VALUES (?, ?, ?, ?)"));
    for (int i = 0; i < forums.size(); ++i) {
        const QVariantMap &f = forums.at(i);
        q.bindValue(0, f.value(QLatin1String("fid")).toString());
        q.bindValue(1, f.value(QLatin1String("name")).toString());
        q.bindValue(2, f.value(QLatin1String("level")).toString());
        q.bindValue(3, QDateTime::currentMSecsSinceEpoch());
        if (!q.exec()) {
            db.rollback();
            return;
        }
    }
    db.commit();
}

void Database::clearCache()
{
    QSqlDatabase db = QSqlDatabase::database(QLatin1String("tieba"));
    db.transaction();
    QSqlQuery q(db);
    const bool forumsCleared = q.exec(QLatin1String("DELETE FROM forum_cache"));
    const bool pagesCleared = q.exec(QLatin1String("DELETE FROM page_cache"));
    if (forumsCleared && pagesCleared)
        db.commit();
    else
        db.rollback();
}

// --- Offline page cache ---

void Database::cacheJson(const QString &key, const QByteArray &json)
{
    // Prune entries older than 14 days (best-effort, runs on each write).
    QSqlQuery prune(QSqlDatabase::database(QLatin1String("tieba")));
    prune.prepare(QLatin1String("DELETE FROM page_cache WHERE updated_at < ?"));
    prune.addBindValue(QDateTime::currentMSecsSinceEpoch() - 14LL * 24 * 3600 * 1000);
    prune.exec();

    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("INSERT OR REPLACE INTO page_cache (key, data, updated_at) VALUES (?, ?, ?)"));
    q.addBindValue(key);
    q.addBindValue(QString::fromUtf8(json));
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    q.exec();
}

QByteArray Database::cachedJson(const QString &key) const
{
    QSqlQuery q(QSqlDatabase::database(QLatin1String("tieba")));
    q.prepare(QLatin1String("SELECT data FROM page_cache WHERE key=? AND updated_at>=? LIMIT 1"));
    q.addBindValue(key);
    q.addBindValue(QDateTime::currentMSecsSinceEpoch() - 14LL * 24 * 3600 * 1000);
    if (q.exec() && q.next())
        return q.value(0).toString().toUtf8();
    return QByteArray();
}
