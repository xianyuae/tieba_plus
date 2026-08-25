#ifndef TIEBA_JSON_H
#define TIEBA_JSON_H

// Lightweight JSON parser/serializer for Qt 4.7 (which lacks QJsonDocument).
// Parses into QVariant trees: QVariantMap / QVariantList / QString / qlonglong /
// double / bool / invalid QVariant (null).
// Numbers are preserved as qlonglong when integral so large IDs/timestamps are
// not rounded; non-integral numbers become double.

#include <QVariant>
#include <QByteArray>

class Json
{
public:
    // Parses UTF-8 JSON. On success returns the root value and sets *ok (if
    // provided) to true. On failure returns an invalid QVariant and *ok=false.
    static QVariant parse(const QByteArray &data, bool *ok = 0);

    // Serializes a QVariant tree (QVariantMap/List/String/Bool/Int/UInt/LongLong/
    // Double/invalid) into compact JSON.
    static QByteArray stringify(const QVariant &value);

private:
    Json();
};

#endif // TIEBA_JSON_H
