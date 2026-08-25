#include "content.h"
#include <QtCore/QStringList>

namespace {

QString firstNonEmpty(const QStringList &urls)
{
    for (int i = 0; i < urls.size(); ++i)
        if (!urls.at(i).trimmed().isEmpty())
            return urls.at(i);
    return QString();
}

} // namespace

QString Content::numStr(const QVariant &v)
{
    if (v.type() == QVariant::String)
        return v.toString();
    if (v.canConvert(QVariant::LongLong))
        return QString::number(v.toLongLong());
    return v.toString();
}

QString Content::avatarUrl(const QString &portrait)
{
    if (portrait.isEmpty())
        return QString();
    if (portrait.startsWith(QLatin1String("http://")) || portrait.startsWith(QLatin1String("https://")))
        return portrait;
    return QLatin1String("http://tb.himg.baidu.com/sys/portrait/item/") + portrait;
}

QString Content::bigAvatarUrl(const QString &portrait)
{
    if (portrait.isEmpty())
        return QString();
    if (portrait.startsWith(QLatin1String("http://")) || portrait.startsWith(QLatin1String("https://")))
        return portrait;
    return QLatin1String("http://tb.himg.baidu.com/sys/portraith/item/") + portrait;
}

QString Content::emoticonUrl(const QString &id)
{
    return QLatin1String("http://static.tieba.baidu.com/tb/editor/images/client/image_emoticon") +
           id + QLatin1String(".png");
}

QString Content::imageThumb(const QVariantMap &frag)
{
    QStringList urls;
    urls << frag.value(QLatin1String("originSrc")).toString()
         << frag.value(QLatin1String("bigCdnSrc")).toString()
         << frag.value(QLatin1String("bigSrc")).toString()
         << frag.value(QLatin1String("dynamic")).toString()
         << frag.value(QLatin1String("cdnSrc")).toString()
         << frag.value(QLatin1String("cdnSrcActive")).toString()
         << frag.value(QLatin1String("src")).toString();
    QString thumb = firstNonEmpty(urls);
    if (thumb.isEmpty())
        thumb = frag.value(QLatin1String("originSrc")).toString();
    return thumb;
}

QString Content::imageOriginal(const QVariantMap &frag)
{
    QString origin = frag.value(QLatin1String("originSrc")).toString();
    if (!origin.isEmpty())
        return origin;
    return frag.value(QLatin1String("src")).toString();
}

QString Content::voiceUrl(const QString &md5)
{
    return QLatin1String("https://tiebac.baidu.com/c/p/voice?voice_md5=") +
           md5 + QLatin1String("&play_from=pb_voice_play");
}

QString Content::joinAbstract(const QVariantList &abstracts)
{
    QString out;
    for (int i = 0; i < abstracts.size(); ++i) {
        const QVariantMap a = abstracts.at(i).toMap();
        const int type = a.value(QLatin1String("type")).toInt();
        const QString text = a.value(QLatin1String("text")).toString();
        if (type == 0 || type == 9 || type == 27)
            out += text;
        else if (type == 2)
            out += QLatin1String("#(") + a.value(QLatin1String("c")).toString() + QLatin1String(")");
        else if (type == 4)
            out += a.value(QLatin1String("un")).toString();
        // other types are ignored (images/videos are rendered separately)
    }
    return out;
}

void Content::finalizeFragment(QVariantMap &frag)
{
    const int type = frag.value(QLatin1String("type")).toInt();
    if (type == 3 || type == 20) {
        frag.insert(QLatin1String("thumb"), imageThumb(frag));
        frag.insert(QLatin1String("original"), imageOriginal(frag));
    } else if (type == 2) {
        frag.insert(QLatin1String("emoUrl"), emoticonUrl(frag.value(QLatin1String("c")).toString()));
    } else if (type == 10) {
        frag.insert(QLatin1String("voiceUrl"), voiceUrl(frag.value(QLatin1String("voiceMD5")).toString()));
    }
}

void Content::finalizeFragments(QVariantList &frags)
{
    for (int i = 0; i < frags.size(); ++i) {
        QVariantMap m = frags.at(i).toMap();
        finalizeFragment(m);
        frags[i] = m;
    }
}
