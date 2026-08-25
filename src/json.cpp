#include "json.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

namespace {

static inline bool isDigit(char c) { return c >= '0' && c <= '9'; }

static inline int hexVal(char c)
{
    if (c >= '0' && c <= '9') return int(c - '0');
    if (c >= 'a' && c <= 'f') return int(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return int(c - 'A' + 10);
    return -1;
}

struct Parser
{
    const QByteArray &d;
    int p;
    int len;
    bool ok;

    explicit Parser(const QByteArray &data) : d(data), p(0), len(data.size()), ok(true) {}

    void skipWs()
    {
        while (p < len) {
            char c = d.at(p);
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++p;
            else break;
        }
    }

    QVariant fail() { ok = false; return QVariant(); }

    bool match(const char *lit)
    {
        int n = int(qstrlen(lit));
        if (p + n > len) return false;
        if (d.mid(p, n) == QByteArray(lit, n)) { p += n; return true; }
        return false;
    }

    QVariant parseValue()
    {
        skipWs();
        if (p >= len) return fail();
        char c = d.at(p);
        switch (c) {
        case '{': return parseObject();
        case '[': return parseArray();
        case '"': {
            const QString value = parseString();
            return ok ? QVariant(value) : QVariant();
        }
        case 't': if (match("true")) return QVariant(true); return fail();
        case 'f': if (match("false")) return QVariant(false); return fail();
        case 'n': if (match("null")) return QVariant(); return fail();
        default: return parseNumber();
        }
    }

    QString parseString()
    {
        ++p; // skip opening quote
        QByteArray out;
        bool closed = false;
        while (p < len) {
            unsigned char c = (unsigned char)d.at(p);
            if (c == '"') { ++p; closed = true; break; }
            if (c == '\\') {
                ++p;
                if (p >= len) { ok = false; break; }
                char e = d.at(p);
                switch (e) {
                case '"': out += '"'; ++p; break;
                case '\\': out += '\\'; ++p; break;
                case '/': out += '/'; ++p; break;
                case 'b': out += '\b'; ++p; break;
                case 'f': out += '\f'; ++p; break;
                case 'n': out += '\n'; ++p; break;
                case 'r': out += '\r'; ++p; break;
                case 't': out += '\t'; ++p; break;
                case 'u': {
                    ++p;
                    quint32 cp = 0;
                    for (int i = 0; i < 4; ++i, ++p) {
                        if (p >= len || hexVal(d.at(p)) < 0) { ok = false; break; }
                        cp = (cp << 4) | quint32(hexVal(d.at(p)));
                    }
                    if (!ok) break;
                    QString tmp;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (p + 5 >= len || d.at(p) != '\\' || d.at(p + 1) != 'u') {
                            ok = false;
                            break;
                        }
                        p += 2;
                        quint32 lo = 0;
                        for (int i = 0; i < 4; ++i, ++p) {
                            if (p >= len || hexVal(d.at(p)) < 0) { ok = false; break; }
                            lo = (lo << 4) | quint32(hexVal(d.at(p)));
                        }
                        if (!ok || lo < 0xDC00 || lo > 0xDFFF) {
                            ok = false;
                            break;
                        }
                        quint32 combined = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                        tmp = QString::fromUcs4(&combined, 1);
                    } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
                        ok = false;
                        break;
                    } else {
                        tmp = QChar(quint16(cp));
                    }
                    out += tmp.toUtf8();
                    break;
                }
                default: ok = false; break;
                }
            } else {
                if (c < 0x20) { ok = false; break; }
                out += (char)c;
                ++p;
            }
        }
        if (!closed) ok = false;
        const QString result = QString::fromUtf8(out);
        if (result.toUtf8() != out) ok = false;
        return result;
    }

    QVariant parseNumber()
    {
        int start = p;
        if (p < len && d.at(p) == '-') ++p;
        if (p >= len || !isDigit(d.at(p))) return fail();
        if (d.at(p) == '0' && p + 1 < len && isDigit(d.at(p + 1))) return fail();
        while (p < len && isDigit(d.at(p))) ++p;
        bool isDouble = false;
        if (p < len && d.at(p) == '.') {
            isDouble = true;
            ++p;
            if (p >= len || !isDigit(d.at(p))) return fail();
            while (p < len && isDigit(d.at(p))) ++p;
        }
        if (p < len && (d.at(p) == 'e' || d.at(p) == 'E')) {
            isDouble = true;
            ++p;
            if (p < len && (d.at(p) == '+' || d.at(p) == '-')) ++p;
            if (p >= len || !isDigit(d.at(p))) return fail();
            while (p < len && isDigit(d.at(p))) ++p;
        }
        QByteArray token = d.mid(start, p - start);
        QString text = QString::fromLatin1(token);
        if (isDouble)
            return QVariant(text.toDouble());
        bool okInt = false;
        qlonglong iv = text.toLongLong(&okInt);
        if (okInt)
            return QVariant(iv);
        return QVariant(text.toDouble());
    }

    QVariant parseObject()
    {
        ++p; // skip {
        QVariantMap obj;
        skipWs();
        if (p < len && d.at(p) == '}') { ++p; return QVariant(obj); }
        while (p < len) {
            skipWs();
            if (p >= len) break;
            if (d.at(p) != '"') return fail();
            QString key = parseString();
            if (!ok) return QVariant();
            skipWs();
            if (p >= len || d.at(p) != ':') return fail();
            ++p;
            QVariant v = parseValue();
            if (!ok) return QVariant();
            obj.insert(key, v);
            skipWs();
            if (p < len && d.at(p) == ',') { ++p; continue; }
            if (p < len && d.at(p) == '}') { ++p; break; }
            return fail();
        }
        return QVariant(obj);
    }

    QVariant parseArray()
    {
        ++p; // skip [
        QVariantList arr;
        skipWs();
        if (p < len && d.at(p) == ']') { ++p; return QVariant(arr); }
        while (p < len) {
            QVariant v = parseValue();
            if (!ok) return QVariant();
            arr.append(v);
            skipWs();
            if (p < len && d.at(p) == ',') { ++p; continue; }
            if (p < len && d.at(p) == ']') { ++p; break; }
            return fail();
        }
        return QVariant(arr);
    }
};

static void appendString(QByteArray &out, const QString &s)
{
    out += '"';
    const QByteArray utf8 = s.toUtf8();
    for (int i = 0; i < utf8.size(); ++i) {
        char c = utf8.at(i);
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if ((unsigned char)c < 0x20) {
                out += "\\u";
                out += QByteArray::number(quint32((unsigned char)c), 16).rightJustified(4, '0');
            } else {
                out += c;
            }
        }
    }
    out += '"';
}

static void appendValue(QByteArray &out, const QVariant &v)
{
    switch (v.type()) {
    case QVariant::Invalid:
        out += "null";
        break;
    case QVariant::Bool:
        out += v.toBool() ? "true" : "false";
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        out += QByteArray::number(v.toLongLong());
        break;
    case QVariant::Double:
        out += QByteArray::number(v.toDouble(), 'g', 16);
        break;
    case QVariant::String:
        appendString(out, v.toString());
        break;
    case QVariant::List:
    case QVariant::StringList: {
        out += '[';
        const QVariantList list = v.toList();
        for (int i = 0; i < list.size(); ++i) {
            if (i) out += ',';
            appendValue(out, list.at(i));
        }
        out += ']';
        break;
    }
    case QVariant::Map: {
        out += '{';
        const QVariantMap map = v.toMap();
        QMap<QString, QVariant>::const_iterator it = map.constBegin();
        bool first = true;
        for (; it != map.constEnd(); ++it) {
            if (!first) out += ',';
            first = false;
            appendString(out, it.key());
            out += ':';
            appendValue(out, it.value());
        }
        out += '}';
        break;
    }
    default:
        appendString(out, v.toString());
        break;
    }
}

} // namespace

QVariant Json::parse(const QByteArray &data, bool *ok)
{
    Parser parser(data);
    QVariant result = parser.parseValue();
    parser.skipWs();
    if (parser.p != parser.len)
        parser.ok = false;
    if (ok) *ok = parser.ok;
    return parser.ok ? result : QVariant();
}

QByteArray Json::stringify(const QVariant &value)
{
    QByteArray out;
    appendValue(out, value);
    return out;
}
