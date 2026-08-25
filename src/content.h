#ifndef TIEBA_CONTENT_H
#define TIEBA_CONTENT_H

// Canonicalization of post/thread rich-text fragments and URL resolution.
// Both the JSON ContentBean and protobuf PbContent shapes are normalized into
// the same QVariantMap fragment form consumed by QML.

#include <QVariantMap>
#include <QVariantList>
#include <QString>

namespace Content
{
    // Avatar URL templates.
    QString avatarUrl(const QString &portrait);
    QString bigAvatarUrl(const QString &portrait);

    // Emoticon remote asset URL (bundled set is served here as .png too).
    QString emoticonUrl(const QString &id);

    // Image thumbnail / original resolution from a type-3/20 fragment.
    QString imageThumb(const QVariantMap &frag);
    QString imageOriginal(const QVariantMap &frag);

    // Voice playback URL.
    QString voiceUrl(const QString &md5);

    // Plain-text preview for a list of abstract fragments.
    QString joinAbstract(const QVariantList &abstracts);

    // Adds resolved "thumb"/"original"/"emoUrl"/"voiceUrl" keys to a fragment.
    void finalizeFragment(QVariantMap &frag);
    void finalizeFragments(QVariantList &frags);

    // Number-to-string helper (proto ints are delivered as qlonglong).
    QString numStr(const QVariant &v);
}

#endif // TIEBA_CONTENT_H
