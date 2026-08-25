# TiebaLite — Post/Thread Rich-Text Content Model

Distilled reference for a Qt/QML port (QML rich text, not Android Spans). Everything
below is taken from the TiebaLite 4.0.0 sources under
`app/src/main/java/com/huanchengfly/tieba/post/` and
`app/src/main/protos/`.

> **Important architectural fact.** There are **two** content encodings in this
> codebase:
>
> 1. **Protobuf `PbContent`** (package `tieba`, `protos/PbContent.proto`) — this is
>    the **active** representation. The Compose UI renders only this. It comes from
>    the official Tieba **Plus** API (`PbPage` / `PbFloor` protobuf endpoints).
> 2. **JSON `ContentBean`** (`ThreadContentBean.ContentBean`) — the **legacy** JSON
>    thread API (`/c/f/pb/page`-style), still deserialized by
>    `api/adapters/ContentMsgAdapter.java` but **no longer rendered** anywhere in
>    the UI (the app migrated fully to protobuf). Its field set mirrors `PbContent`
>    one-to-one and its `type` values use the same numeric convention, stored as
>    strings.
>
> A Qt port should consume the **Protobuf** shape. The JSON shape is documented here
> only for completeness/backward compatibility.

---

## 1. The content fragment types

### 1.1 Protobuf `PbContent` (authoritative)

`protos/PbContent.proto`, `package tieba`:

```proto
message PbContent {
  int32  type = 1;
  string text = 2;
  string link = 3;
  string src = 4;
  string bsize = 5;         // "width,height" (comma, NOT 'x')
  string bigSrc = 6;
  string bigSize = 7;
  string cdnSrc = 8;
  string bigCdnSrc = 9;
  string imgType = 10;
  string c = 11;            // emoticon asset id, e.g. "image_emoticon25"
  string voiceMD5 = 12;
  uint32 duringTime = 13;   // seconds
  int64  uid = 15;          // @mention target user id
  string dynamic = 16;      // dynamic (watermarked/looped) image URL
  string _static = 17;
  uint32 width = 18;
  uint32 height = 19;
  string originSrc = 25;    // original full-res image URL
  uint32 originSize = 27;   // original size in bytes
  string mediaSubtitle = 31;
  int32  urlType = 32;
  MemeInfo memeInfo = 33;
  uint32 isLongPic = 34;
  uint32 showOriginalBtn = 35;
  string cdnSrcActive = 36;
}
```

The exact `type` → meaning mapping is in
`api/models/protos/Extensions.kt` (`List<PbContent>.renders`):

| `type` | Fragment kind | Fields used | How rendered |
|-------:|---------------|-------------|--------------|
| `0`, `9`, `27` | **Text** | `text` | plain text (runs collapsed; multiple spaces squashed for abstracts) |
| `1` | **Link / URL** | `link` (URL), `text` (anchor label) | 🔗 icon + colored anchor; tap opens `link` |
| `2` | **Emoticon** | `text` (name), `c` (asset id) | inline image; text is rewritten to `#(<c>)` and resolved against the emoticon table |
| `3` | **Image** | `originSrc`, `bigCdnSrc`, `bigSrc`, `dynamic`, `cdnSrc`, `cdnSrcActive`, `src`; `bsize` = `"w,h"`; `showOriginalBtn`, `originSize` | `PicContentRender` — full/half-width, aspect = w/h |
| `4` | **@Mention** | `text` (display, e.g. `@foo`), `uid` (user id) | colored span, tap → `user/<uid>` |
| `5` | **Video** | `src` (poster URL), `link` (video stream URL), `text` (web page URL), `bsize` = `"w,h"` | `VideoContentRender`; if `src` blank → 🎥 + anchor to `text` |
| `10` | **Voice / Audio** | `voiceMD5`, `duringTime` | `VoiceContentRender` → voice player |
| `20` | **Image (alt form)** | `src` (used for both thumb & origin), `bsize`, `showOriginalBtn`, `originSize` | `PicContentRender` |

Notes:

- `type = 3` vs `type = 20`: both are images. Type `3` uses the full URL family
  (`originSrc`/`bigCdnSrc`/…) while type `20` is a simpler form where `src` is the
  only URL.
- `memeInfo` (`MemeInfo`: `pckId`, `picId`, `picUrl`, `thumbnail`, `width`,
  `height`, `detailLink`) is a *sticker/meme* attachment present in the proto but
  not currently rendered by the `when` above.
- `Post` also carries a whole-thread-level video (`video_info`, see §5) and a
  `is_voice` flag, independent of the per-fragment `type=10`.

### 1.2 Protobuf `Abstract` (thread-list rich abstract)

`protos/Abstract.proto`:

```proto
message Abstract {
  int32  type = 1;
  string text = 2;
  string link = 3;
  string src = 4;
  string un = 5;          // user name (for @mention)
  string duringTime = 6;
  string voiceMD5 = 7;
}
```

Mapping (`abstractText` in `Extensions.kt`): `0` → text; `2` → emoticon (rendered
as `#(<c>)`); `4` → @mention text. Everything else ignored. This shape feeds the
forum thread list / search results / "my posts" abstract previews
(`ThreadInfo.richAbstract`, `ThreadInfo._abstract`,
`OriginThreadInfo._abstract`, `PostInfoList.rich_abstract`,
`PostInfoContent.post_content`).

### 1.3 Legacy JSON `ContentBean`

`api/models/ThreadContentBean.kt` (parsed by `ContentMsgAdapter.java`). `type` is a
**String**; numeric values mirror `PbContent` (`"0"` text, `"1"` link, `"2"`
emoticon, `"3"` image, `"4"` @mention, `"5"` video, `"10"` voice, `"20"` image).
Confirmed in code: `MediaAdapter.java` treats JSON `type == "3"` as an image.
Fields:

```kotlin
class ContentBean {
    val type: String?         // "0".."5","10","20", ...
    val text: String?
    val link: String?
    val src: String?
    val uid: String?
    val originSrc: String?    // @SerializedName("origin_src")
    val cdnSrc: String?       // @SerializedName("cdn_src")
    val cdnSrcActive: String? // @SerializedName("cdn_src_active")
    val bigCdnSrc: String?    // @SerializedName("big_cdn_src")
    val duringTime: String?   // @SerializedName("during_time")
    val bsize: String?        // "w,h"
    val c: String?            // emoticon asset id
    val width: String?
    val height: String?
    val isLongPic: String?    // @SerializedName("is_long_pic")
    val voiceMD5: String?     // @SerializedName("voice_md5")
}
```

---

## 2. Image representation

### 2.1 URL fields (type `3`)

| Field (PbContent) | JSON equivalent | Meaning |
|---|---|---|
| `originSrc` | `origin_src` | original, full-resolution URL |
| `bigCdnSrc` | `big_cdn_src` | CDN large thumbnail |
| `bigSrc` | — | large thumbnail (non-CDN) |
| `dynamic` | — | dynamic/animated preview image |
| `cdnSrc` | `cdn_src` | CDN small thumbnail |
| `cdnSrcActive` | `cdn_src_active` | active/alternate CDN thumbnail |
| `src` | `src` | base image URL |

Thumbnail selection is done by `ImageUtil.getUrl(context, isSmallPic, originUrl,
vararg smallPicUrls)`. For a body image the call passes, in quality order:

```
originSrc, bigCdnSrc, bigSrc, dynamic_, cdnSrc, cdnSrcActive, src
```

- **Thumbnail**: the **first non-empty** value in that order (unless data-saver
  mode reverses the list). Fallback = `originSrc`.
- **Original**: always `originSrc`.

`ImageUtil.getPicId(url)` = the URL's file name with `.jpg` stripped (i.e. the
image id used for the photo viewer / `#(pic,...)` authoring code).

### 2.2 Dimensions & size

- `bsize` = the string `"<width>,<height>"` (comma-separated; **not** `x`).
  `PbContent` also has separate `width`/`height` uints.
- `originSize` = original file size in bytes (shown on the "view original" button).
- `isLongPic` = long/scrolling screenshot flag.
- `showOriginalBtn` = whether to show the "view original" control.

### 2.3 `#(pic, …)` authoring code

When **composing** (uploading), an image is inserted into the post body text as:

```
#(pic,<picId>,<width>,<height>)
```

(source: `UploadResultBean.UploadInfo.getPic()` and `ReplyPage.kt`). `<picId>` is
the bare id (no extension). This is a *compose-time* convention, **not** an
emoticon code (see §3) and not what the renderer consumes — the renderer uses the
`originSrc`/`cdnSrc`/… URL fields directly. The corresponding classic original
URL template (used in `ForumBeanCaster.java` when only a picId is available):

```
http://imgsrc.baidu.com/forum/pic/item/<fileName>   (fileName includes .jpg)
http://imgsa.baidu.com/forum/pic/item/<fileName>    (fallback)
```

### 2.4 Layout of multiple images

There is **no classic 9-grid** in this Compose build. Concrete behavior:

- **Floor content** (`PicContentRender` in `PbContentRender.kt`): each image
  fragment renders as its own block —
  `fillMaxWidth(1f)` on compact screens, `fillMaxWidth(0.5f)` on non-compact,
  with `aspectRatio(width/height)`. Multiple images simply stack vertically.
- **Thread-list feed card** (`ThreadMedia` in `FeedCard.kt`): a single photo uses
  a 2:1 box; multiple photos show a horizontal row of **up to 3** thumbnails in a
  3:1 box with a `+N` badge when there are more than 3.
- **Compose limit**: a reply allows up to **9** images (`ReplyPage.kt`:
  `9 - selectedImages.size`).

If the QML port wants the traditional Tieba "nine-grid" look
(1 image large; 2/4 → 2×2; 3/5/6 → 3-col; 7/8/9 → 3×3), that layout rule is **not**
implemented in this source tree and would be a port-side decision.

---

## 3. Emoticons / Emoji

### 3.1 In-band code format

Two text formats are recognized (regexes in `utils/EmoticonUtil.kt`):

| Context | Format | Example | Regex |
|---|---|---|---|
| Post body (app) | `#(<name>)` | `#(滑稽)` | `#\((([一-龥\w~])+)\)` |
| Web / HTML | `(#<name>)` | `(#滑稽)` | `\(#(([\u4e00-\u9fa5\w~])+)\)` |

`<name>` is CJK ideographs (`一`–`龥`), word chars, or `~`. Note: **`#(pic,…)` is
NOT an emoticon** — it is the image authoring code (§2.3) and does not match the
emoticon regex (the charset excludes `,`).

In protobuf content, an emoticon is `type = 2` with:

- `text` = the human-readable name (e.g. `"滑稽"`)
- `c` = the asset id (e.g. `"image_emoticon25"`)

The renderer registers `(text → c)` and emits the inline code `#(<c>)`.

### 3.2 Name → asset mapping

`utils/EmoticonManager.kt` (`DEFAULT_EMOTICON_MAPPING`) and
`utils/EmoticonUtil.kt` (web maps) map **name → `image_emoticon<N>`**. Excerpt:

```
呵呵→1   哈哈→2   吐舌→3   啊→4   酷→5   怒→6   开心→7   汗→8   泪→9
黑线→10  鄙视→11  不高兴→12 真棒→13 钱→14 疑问→15 阴险→16 吐→17 咦→18
委屈→19  花心→20  呼~→21   笑眼→22 冷→23 太开心→24 滑稽→25 勉强→26 狂汗→27
乖→28    睡觉→29  惊哭→30  生气→31 惊讶→32 喷→33
爱心→34  心碎→35  玫瑰→36  礼物→37 彩虹→38 星星月亮→39 太阳→40 钱币→41
灯泡→42  茶杯→43  蛋糕→44  音乐→45 haha→46 胜利→47 大拇指→48 弱→49 OK→50
生气→61  沙发→77  手纸→78  香蕉→79 便便→80 药丸→81 红领巾→82 蜡烛→83 三道杠→84
```

(`生气` is intentionally mapped twice: `31` in the classic web map and `61` in the
default map. `EmoticonUtil` also maps `(#噗)` → `image_emoticon89`.)

### 3.3 Asset naming & bundling

- Bundled resources: `res/drawable/image_emoticon<N>.webp` (51 files: the classic
  set `1..50` incl. `89`, plus the emoji set `34..50`).
- The full dynamic id list (`getEmoticonInlineContent` / `init`) is
  `1..50`, `61..101`, `125..137`.
- Any id **not** bundled is fetched at runtime from:

```
http://static.tieba.baidu.com/tb/editor/images/client/image_emoticon<N>.png
```

  and cached to `<cacheDir>/emoticon/image_emoticon<N>.png`.

A Qt port needs: (a) the bundled `image_emoticon1..50.webp` (+ `89`, and emoji
`34..50`) assets, and (b) runtime fetch for ids `61..101` and `125..137` using the
`static.tieba.baidu.com` template above.

---

## 4. @mention and quote/reference

### 4.1 @Mention

Encoded as a `PbContent` fragment with `type = 4`:

- `text` = the display label (typically `@<nickname>`),
- `uid` (int64) = the mentioned user's id.

Rendered as a colored (accent) annotation tagged `"user"` with the value `<uid>`;
tapping navigates to `user/<uid>`. (See `Extensions.kt` `renders`, `PbContentText`
in `PbContentRender.kt`.)

`Abstract` uses `type = 4` + `un` (user name) for the same thing in list previews.

### 4.2 Quote / reference to another floor

There is **no dedicated "quote" content-fragment type** inside the `PbContent` list.
Quoting is expressed at the *floor* level and via sub-posts:

- **`Post.quote_id`** (`string`, field 50) — a floor that quotes/refers to another
  post.
- **Sub-post replies** (`Post.sub_post_list` → `SubPostList`, `Post.sub_post_number`)
  — the reply chain is rendered as `@<author> : <content>` inline beneath the floor
  (`SubPostList.getContentText` in `Extensions.kt`; the `@`-form is built by
  `StringUtil.buildAnnotatedStringWithUser`). `SubPostList` carries `id`, `content`
  (`repeated PbContent`), `time`, `author_id`, `title`, `floor`, `author`,
  `is_giftpost`, `agree`, `location`, `is_fake_top`, `is_author_view`.
- **Forwarded / quoted thread** — `Post.origin_thread_info` /
  `ThreadInfo.origin_thread_info` (`OriginThreadInfo`), rendered as
  `OriginThreadCard`. `OriginThreadInfo` holds `title`, `media` (`repeated Media`),
  `_abstract` (`repeated Abstract`), `fname`, `tid`, `fid`, `voice_info`,
  `video_info`, `content` (`repeated PbContent`), `author`, `agree`, `reply_num`,
  `pid`, `good_types`, `top_types`, etc.
- `Quote.proto` exists (`post_id`, `user_name`, `user_id`, `ip`, `content`) but is
  **not** used by the current content pipeline.

---

## 5. Video and Voice

### 5.1 Video

Two independent representations:

**(a) In-content video fragment — `PbContent type = 5`**

- `src`  = poster/thumbnail image URL,
- `link` = the playable video stream URL,
- `text` = the web page URL (fallback destination),
- `bsize` = `"<width>,<height>"`.

Rendering (`VideoContentRender`): if `src` is non-blank → `VideoPlayer` with
`videoUrl = link`, `thumbnailUrl = src`, `aspectRatio(w/h)`. If `src` is blank →
an inline 🎥 anchor to `text` (opens webview). When `link` is empty but `src` is
present, a plain clickable poster is shown that opens `text`.

**(b) Thread-level video — `VideoInfo`** (on `Post.video_info` /
`ThreadInfo.videoInfo` / `OriginThreadInfo.video_info`)

`protos/VideoInfo.proto`:

```proto
message VideoInfo {
  string videoMD5 = 1;
  string videoUrl = 2;
  uint32 videoDuration = 3;
  uint32 videoWidth = 4;
  uint32 videoHeight = 5;
  string thumbnailUrl = 6;
  uint32 thumbnailWidth = 7;
  uint32 thumbnailHeight = 8;
  string mediaSubtitle = 11;
}
```

Feed cards render this with `thumbnailUrl` as poster, `aspectRatio(max(thumbW/thumbH, 16/9))`.

### 5.2 Voice / Audio

**(a) `PbContent type = 10`**

- `voiceMD5` (string), `duringTime` (uint32, seconds).

Playback URL template (`VoiceContentRender`):

```
https://tiebac.baidu.com/c/p/voice?voice_md5=<voiceMD5>&play_from=pb_voice_play
```

**(b) `Voice` message** (`protos/Voice.proto`) — `type`, `during_time`,
`voice_md5`; used in `OriginThreadInfo.voice_info`.

The voice player (`VoicePlayerView`) also parses `voice_md5` out of the same
`tiebac.baidu.com/c/p/voice` URL query.

---

## 6. Overall container shape (floor / thread)

### 6.1 Thread response container

`protos/PbPage/PbPageResponseData.proto` (package `tieba.pbPage`), returned by the
`/c/f/pb/page` protobuf endpoint:

```proto
message PbPageResponseData {
  User user = 1;
  SimpleForum forum = 2;
  Page page = 3;
  Anti anti = 4;
  AddPost add_post = 5;
  repeated Post post_list = 6;   // the floors
  int32 has_floor = 7;
  ThreadInfo thread = 8;         // thread metadata
  Lbs location = 9;
  int32 is_new_url = 10;
  repeated User user_list = 13;
  int32 server_time = 14;
  Post first_floor_post = 38;    // floor 1 (main post), also in post_list
  SimpleForum display_forum = 39;
  ...
}
```

`SimpleForum` = `{ id, name, avatar, memberNum, postNum }`. `Page` carries
`current_page`, `new_total_page`, `has_more`, `has_prev` (pagination). `Anti`
carries `tbs` (the CSRF token used for replies/deletes).

### 6.2 A floor (`Post`)

`protos/Post.proto` (package `tieba`) — the core per-floor object:

```proto
message Post {
  uint64 id = 1;
  string title = 2;
  uint32 floor = 3;              // 1 = 楼主 (main post)
  uint32 time = 4;               // unix seconds
  repeated PbContent content = 5; // the rich-text fragment list
  repeated string arr_video = 6;
  Lbs lbs_info = 7;              // geo location
  uint32 is_vote = 8;
  uint32 is_voice = 9;
  uint32 is_ntitle = 10;         // "no title" (hide title)
  uint32 is_bub = 11;
  string vote_crypt = 12;
  uint32 sub_post_number = 13;
  string time_ex = 14;           // alternative/precise time string
  SubPost sub_post_list = 15;    // sub-replies
  AddPostList add_post_list = 16;
  string bimg_url = 17;
  string ios_bimg_format = 18;
  int64 author_id = 19;
  uint32 add_post_number = 20;
  SignatureData signature = 21;
  TailInfo tail_info = 22;       // tail / signature banner
  User author = 23;
  Zan zan = 24;
  int32 storecount = 25;
  TPointPost tpoint_post = 26;
  ActPost act_post = 27;
  PbPresent present = 28;
  VideoInfo video_info = 29;     // thread-level video (see §5)
  PbPostZan post_zan = 30;
  int32 is_hot_post = 31;
  repeated TailInfo ext_tails = 32;
  TogetherHi high_together = 33;
  SkinInfo skin_info = 34;
  DealInfo pb_deal_info = 35;
  string lego_card = 36;
  Agree agree = 37;              // like count/state
  SimpleForum from_forum = 38;
  int32 is_post_visible = 39;
  int32 need_log = 40;
  int32 img_num_abtest = 41;
  OriginThreadInfo origin_thread_info = 42; // quoted/forwarded thread
  int32 is_fold = 43;
  string fold_tip = 44;
  int32 is_top_agree_post = 45;
  int64 tid = 46;                // thread id
  int32 show_squared = 47;
  int32 is_bjh = 48;
  string quote_id = 50;
  int32 is_wonderful_post = 51;
  repeated HeadItem item_star = 52;
  Item item = 53;
  Item outer_item = 54;
  Advertisement advertisement = 55;
  int32 fold_comment_status = 56;
  string fold_comment_apply_url = 57;
  NovelInfo novel_info = 58;
}
```

`Agree` (`protos/Agree.proto`) = `{ agreeNum, hasAgree, agreeType, disagreeNum,
diffAgreeNum }`. `diffAgreeNum` is what the UI shows as the like count.

`SubPostList` (`protos/SubPostList.proto`) = `{ id, content(repeated PbContent),
time, author_id, title, floor, author(User), is_giftpost, agree(Agree),
location(Lbs), is_fake_top, is_author_view }`.

### 6.3 Time, floor, author (how a floor header is built)

From `ThreadPage.kt` `getDescText` / `PostCard`:

- **Time**: `post.time` (uint32 unix seconds) → relative time string
  (`DateTimeUtils.getRelativeTimeString`); `post.time_ex` is an alternative raw
  string the API also sends. Thread level uses `ThreadInfo.createTime`,
  `ThreadInfo.lastTime`, `lastTimeInt`.
- **Floor**: `post.floor`. Displayed as `第<floor>楼` (string `tip_post_floor`)
  **only when `floor > 1`** (floor 1 is the main post / 楼主).
- **Author**: `post.author` (`User`), with `author_id` as the raw id. Header
  renders avatar (`portrait`), name, level chip (`level_id`), moderator tag
  (`is_bawu`/`bawu_type` → 吧主 / 小吧主), and an `LZ` (楼主) chip when
  `author.id == thread author id`.
- Header desc line = `relativeTime · 第N楼 · IP属地(ip_address)`.
- Sub-posts use `sub_post_number` count and show `SubPostList` rows.

### 6.4 User & avatar

`protos/User.proto` fields relevant to a floor: `id` (int64), `name`,
`nameShow` (nickname), `portrait`, `portraith`, `level_id`, `is_bawu`,
`bawu_type`, `ip_address`, `type`, `is_manager`.

Display name = nickname vs username depending on
`showBothUsernameAndNickname` preference (`StringUtil.getUsernameAnnotatedString`).

Avatar URL templates (`StringUtil.getAvatarUrl` / `getBigAvatarUrl`):

```
http://tb.himg.baidu.com/sys/portrait/item/<portrait>     // normal
http://tb.himg.baidu.com/sys/portraith/item/<portrait>    // large
```

If `portrait` already starts with `http://` / `https://`, it is used verbatim.

---

## Appendix — file map

| Concern | Source file |
|---|---|
| Fragment type mapping (`PbContent`) | `api/models/protos/Extensions.kt` (`List<PbContent>.renders`) |
| Fragment type mapping (`Abstract`) | `api/models/protos/Extensions.kt` (`abstractText`) |
| Render widgets | `ui/common/PbContentRender.kt` (`TextContentRender`, `PicContentRender`, `VideoContentRender`, `VoiceContentRender`) |
| Floor render | `ui/page/thread/ThreadPage.kt` (`PostCard`, `getDescText`) |
| Floor data pipeline | `ui/page/thread/ThreadViewModel.kt` |
| Legacy JSON model + adapter | `api/models/ThreadContentBean.kt`, `api/adapters/ContentMsgAdapter.java` |
| Emoticon mapping / assets | `utils/EmoticonManager.kt`, `utils/EmoticonUtil.kt`, `res/drawable/image_emoticon*.webp` |
| Image URL resolution / picId | `utils/ImageUtil.kt` (`getUrl`, `getPicId`) |
| Image viewer data | `ui/utils/PhotoViewUtils.kt` |
| URL templates (origin pic, avatar) | `api/caster/ForumBeanCaster.java`, `utils/StringUtil.kt` |
| Voice URL template | `ui/common/PbContentRender.kt` (`VoiceContentRender`) |
| Protos | `app/src/main/protos/PbContent.proto`, `Post.proto`, `SubPost.proto`, `SubPostList.proto`, `ThreadInfo.proto`, `OriginThreadInfo.proto`, `Abstract.proto`, `Media.proto`, `VideoInfo.proto`, `Voice.proto`, `Agree.proto`, `User.proto`, `SimpleForum.proto`, `Quote.proto`, `LinkInfo.proto`, `PbLinkInfo.proto`, `MemeInfo.proto`, `Timgs.proto`, `ThreadPicList.proto`, `PbPage/PbPageResponseData.proto` |
