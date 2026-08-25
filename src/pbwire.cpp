#include "pbwire.h"

QByteArray PbWriter::encodeVarint(quint64 value)
{
    QByteArray out;
    while (value >= 0x80) {
        out += char((value & 0x7F) | 0x80);
        value >>= 7;
    }
    out += char(value & 0x7F);
    return out;
}

QByteArray PbWriter::encodeUtf8(const QString &s)
{
    return s.toUtf8();
}

void PbWriter::key(int field, int wireType)
{
    m_data += encodeVarint(quint64((quint64(field) << 3) | quint64(wireType)));
}

void PbWriter::varint(int field, quint64 value)
{
    key(field, 0);
    m_data += encodeVarint(value);
}

void PbWriter::fixed32(int field, quint32 value)
{
    key(field, 5);
    for (int i = 0; i < 4; ++i)
        m_data += char((value >> (i * 8)) & 0xFF);
}

void PbWriter::fixed64(int field, quint64 value)
{
    key(field, 1);
    for (int i = 0; i < 8; ++i)
        m_data += char((value >> (i * 8)) & 0xFF);
}

void PbWriter::bytes(int field, const QByteArray &value)
{
    key(field, 2);
    m_data += encodeVarint(quint64(value.size()));
    m_data += value;
}

void PbWriter::string(int field, const QString &value)
{
    bytes(field, value.toUtf8());
}

void PbWriter::message(int field, const QByteArray &value)
{
    bytes(field, value);
}

void PbWriter::boolean(int field, bool value)
{
    varint(field, value ? 1 : 0);
}

bool PbReader::readVarint(const QByteArray &data, int *pos, quint64 *out)
{
    quint64 result = 0;
    int shift = 0;
    int p = *pos;
    while (p < data.size() && shift < 64) {
        unsigned char b = (unsigned char)data.at(p);
        ++p;
        if (shift == 63 && b > 1)
            return false;
        result |= quint64(b & 0x7F) << shift;
        if ((b & 0x80) == 0) {
            *pos = p;
            *out = result;
            return true;
        }
        shift += 7;
    }
    return false;
}

int PbReader::next()
{
    if (atEnd()) return 0;
    quint64 key = 0;
    if (!readVarint(m_data, &m_pos, &key)) {
        m_pos = m_data.size();
        return 0;
    }
    int field = int(key >> 3);
    m_wire = int(key & 0x07);
    if (field <= 0 || m_wire == 3 || m_wire == 4) {
        m_pos = m_data.size();
        return 0;
    }
    return field;
}

quint64 PbReader::varint()
{
    if (m_wire != 0) {
        m_pos = m_data.size();
        return 0;
    }
    quint64 v = 0;
    if (!readVarint(m_data, &m_pos, &v))
        m_pos = m_data.size();
    return v;
}

QByteArray PbReader::takeLengthDelimited()
{
    if (m_wire != 2) {
        m_pos = m_data.size();
        return QByteArray();
    }
    quint64 len = 0;
    if (!readVarint(m_data, &m_pos, &len)) {
        m_pos = m_data.size();
        return QByteArray();
    }
    if (len > quint64(m_data.size() - m_pos)) {
        m_pos = m_data.size();
        return QByteArray();
    }
    QByteArray r = m_data.mid(m_pos, int(len));
    m_pos += int(len);
    return r;
}

QByteArray PbReader::bytes() { return takeLengthDelimited(); }

QString PbReader::string() { return QString::fromUtf8(takeLengthDelimited()); }

QByteArray PbReader::message() { return takeLengthDelimited(); }

bool PbReader::boolean() { return varint() != 0; }

quint32 PbReader::fixed32()
{
    if (m_wire != 5) {
        m_pos = m_data.size();
        return 0;
    }
    quint32 v = 0;
    for (int i = 0; i < 4 && m_pos < m_data.size(); ++i)
        v |= quint32((unsigned char)m_data.at(m_pos++)) << (i * 8);
    return v;
}

quint64 PbReader::fixed64()
{
    if (m_wire != 1) {
        m_pos = m_data.size();
        return 0;
    }
    quint64 v = 0;
    for (int i = 0; i < 8 && m_pos < m_data.size(); ++i)
        v |= quint64((unsigned char)m_data.at(m_pos++)) << (i * 8);
    return v;
}

void PbReader::skipField()
{
    switch (m_wire) {
    case 0: varint(); break;
    case 1: fixed64(); break;
    case 2: takeLengthDelimited(); break;
    case 5: fixed32(); break;
    default:
        // Unknown wire type: cannot determine length, so abort cleanly.
        m_pos = m_data.size();
        break;
    }
}
