#ifndef TIEBA_PBWIRE_H
#define TIEBA_PBWIRE_H

// Minimal Protocol Buffer wire-format codec for Qt 4.7.
// Implements varint encode/decode plus the five wire types:
//   0 = varint, 1 = fixed64, 2 = length-delimited, 5 = fixed32.
// This is intentionally NOT a schema compiler; higher-level message classes
// (see proto_messages.h) use these primitives to serialize/parse their fields.

#include <QByteArray>
#include <QString>
#include <QtGlobal>

class PbWriter
{
public:
    PbWriter() {}

    void clear() { m_data.clear(); }
    QByteArray data() const { return m_data; }

    void varint(int field, quint64 value);
    void fixed32(int field, quint32 value);
    void fixed64(int field, quint64 value);
    void bytes(int field, const QByteArray &value);
    void string(int field, const QString &value);
    void message(int field, const QByteArray &value); // pre-serialized nested message
    void boolean(int field, bool value);

    // Low-level helpers shared with readers.
    static QByteArray encodeVarint(quint64 value);
    static QByteArray encodeUtf8(const QString &s);

private:
    QByteArray m_data;
    void key(int field, int wireType);
};

class PbReader
{
public:
    explicit PbReader(const QByteArray &data) : m_data(data), m_pos(0), m_wire(0) {}

    // Reads the next key. Returns the field number, or 0 when the buffer is
    // exhausted. After a non-zero return, the corresponding typed getter below
    // must be called (or skipField()) to advance past the value.
    int next();
    int wireType() const { return m_wire; }

    quint64 varint();
    quint32 fixed32();
    quint64 fixed64();
    QByteArray bytes();          // length-delimited payload
    QString string();            // UTF-8 string
    QByteArray message();        // length-delimited payload (alias of bytes())
    bool boolean();

    void skipField();

    // Reads a varint starting at position *pos and advances *pos. Returns false
    // on malformed input.
    static bool readVarint(const QByteArray &data, int *pos, quint64 *out);

private:
    QByteArray m_data;
    int m_pos;
    int m_wire;
    bool atEnd() const { return m_pos >= m_data.size(); }
    QByteArray takeLengthDelimited();
};

#endif // TIEBA_PBWIRE_H
