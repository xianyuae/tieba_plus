#ifndef TIEBA_SIGNUTIL_H
#define TIEBA_SIGNUTIL_H

#include <QMap>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QPair>

// Holds request parameters keyed by name (auto-sorted). The signing algorithm
// and the wire format both need the parameters sorted by key, so we keep them
// in a QMap and derive both the sign source and the encoded body from it.
class HttpParams
{
public:
    HttpParams() {}
    HttpParams(const QMap<QString, QString> &map) : m_map(map) {}

    void add(const QString &key, const QString &value) { m_map.insert(key, value); }
    void addIfAbsent(const QString &key, const QString &value)
    {
        if (!m_map.contains(key)) m_map.insert(key, value);
    }
    bool contains(const QString &key) const { return m_map.contains(key); }
    QString value(const QString &key) const { return m_map.value(key); }
    QMap<QString, QString> map() const { return m_map; }
    int size() const { return m_map.size(); }

    // Sorted "k=v" concatenated with no separator, using DECODED values.
    QString signSource() const;

    // Sorted form-encoded body: "k=v&k=v" (space as '+', UTF-8 percent-encoded).
    QByteArray formEncoded() const;

    // Sorted query string: "k=v&k=v" (space as '+'), no leading '?'.
    QByteArray queryEncoded() const;

private:
    QMap<QString, QString> m_map;
};

namespace SignUtil
{
    QString md5(const QByteArray &data);   // 32 lowercase hex chars
    QString md5(const QString &text);      // UTF-8 bytes then md5
    QString sign(const HttpParams &params, const QString &secret);
    QByteArray formEncode(const QString &s);
    QString urlEncode(const QString &s);
    QString randomHex(int byteCount);      // 2*byteCount lowercase hex chars
    QString randomDigits(int digitCount);
    QString currentTimestampMs();
}

// Baidu's anti-crawl "st" noise parameters (replicated from StParamInterceptor).
namespace StParams
{
    // method=false => stMethod="1" (mirrors StParamInterceptor()).
    void addTo(HttpParams &params, bool method = false);
}

#endif // TIEBA_SIGNUTIL_H
