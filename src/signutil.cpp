#include "signutil.h"
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <cstdlib>

QString HttpParams::signSource() const
{
    QString out;
    QMap<QString, QString>::const_iterator it = m_map.constBegin();
    for (; it != m_map.constEnd(); ++it) {
        out += it.key();
        out += QLatin1Char('=');
        out += it.value();
    }
    return out;
}

QByteArray HttpParams::formEncoded() const
{
    QByteArray out;
    bool first = true;
    QMap<QString, QString>::const_iterator it = m_map.constBegin();
    for (; it != m_map.constEnd(); ++it) {
        if (!first) out += '&';
        first = false;
        out += SignUtil::formEncode(it.key());
        out += '=';
        out += SignUtil::formEncode(it.value());
    }
    return out;
}

QByteArray HttpParams::queryEncoded() const
{
    return formEncoded();
}

QString SignUtil::md5(const QByteArray &data)
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data);
    return QString::fromLatin1(hash.result().toHex());
}

QString SignUtil::md5(const QString &text)
{
    return md5(text.toUtf8());
}

QString SignUtil::sign(const HttpParams &params, const QString &secret)
{
    return md5(params.signSource() + secret);
}

QByteArray SignUtil::formEncode(const QString &s)
{
    static const char hexDigits[] = "0123456789ABCDEF";
    const QByteArray utf8 = s.toUtf8();
    QByteArray out;
    out.reserve(utf8.size() * 2);
    for (int i = 0; i < utf8.size(); ++i) {
        unsigned char c = (unsigned char)utf8.at(i);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '*') {
            out += char(c);
        } else if (c == ' ') {
            out += '+';
        } else {
            out += '%';
            out += hexDigits[c >> 4];
            out += hexDigits[c & 0x0F];
        }
    }
    return out;
}

QString SignUtil::urlEncode(const QString &s)
{
    return QString::fromLatin1(formEncode(s));
}

QString SignUtil::randomHex(int byteCount)
{
    QString out;
    for (int i = 0; i < byteCount; ++i) {
        int v = qrand() & 0xFF;
        out += QString::number(v, 16).rightJustified(2, QLatin1Char('0'));
    }
    return out;
}

QString SignUtil::randomDigits(int digitCount)
{
    QString out;
    for (int i = 0; i < digitCount; ++i)
        out += QString::number(qrand() % 10);
    return out;
}

QString SignUtil::currentTimestampMs()
{
    return QString::number(QDateTime::currentMSecsSinceEpoch());
}

void StParams::addTo(HttpParams &params, bool method)
{
    const int num = 100 + (qrand() % 750); // 100..849 (mirrors nextInt(100,850))
    QString stErrorNums;
    if (num >= 100 && num <= 120) {
        stErrorNums = QLatin1String("0");
        params.add(QLatin1String("stErrorNums"), stErrorNums);
    } else {
        stErrorNums = QLatin1String("1");
        params.add(QLatin1String("stErrorNums"), stErrorNums);
        params.add(QLatin1String("stMethod"), method ? QLatin1String("2") : QLatin1String("1"));
        params.add(QLatin1String("stMode"), QLatin1String("1"));
        params.add(QLatin1String("stTimesNum"), QLatin1String("1"));
        params.add(QLatin1String("stTime"), QString::number(num));
        const int stSize = qRound(((double(qrand()) / RAND_MAX) * 8.0 + 0.4) * num);
        params.add(QLatin1String("stSize"), QString::number(stSize));
    }
}
