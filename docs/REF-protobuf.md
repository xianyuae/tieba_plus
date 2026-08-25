# TiebaLite Protobuf Wire Schema Reference

Distilled from the TiebaLite Android reference source (proto files under
`app/src/main/protos/`) for a hand-written C++ protobuf codec (Qt 4.7, no
protobuf library — encode/decode varint + fields manually).

> All `.proto` files are `syntax = "proto3"` and `package tieba` (or a
> `tieba.<sub>` subpackage). `java_package` maps each to
> `com.huanchengfly.tieba.post.api.models.protos[.<sub>]`.

---

## 1. Wire-format basics (proto3)

The protobuf wire format has no message framing of its own. A message is a
concatenation of `field = (field_number << 3) | wire_type` key varints followed
by the field value.

| Wire type | # | Used by |
|---|---|---|
| varint | 0 | `int32`, `int64`, `uint32`, `uint64`, `bool`, `enum` |
| 64-bit | 1 | `fixed64`, `sfixed64`, `double` |
| length-delimited | 2 | `string`, `bytes`, nested `message`, and **packed repeated scalars** |
| 32-bit | 5 | `fixed32`, `sfixed32`, `float` |

Two proto3 details that matter for a hand codec:

1. **Packed repeated scalars** — in proto3, a `repeated` field of a *numeric*
   scalar type (`int32/int64/uint32/uint64/bool/enum/fixed*/double/float`) is
   **packed by default**: all elements are written into ONE length-delimited
   field (wire type 2) whose payload is the concatenation of each element's raw
   encoding. `repeated string/bytes/message` are **not** packed (each element is
   its own field). This reference uses packed repeats everywhere (there is no
   `[packed=false]` in the tree).
2. **`optional` keyword** — proto3 scalar/`message` fields without `optional`
   have no presence tracking but are still omitted from the wire when unset;
   `optional` adds explicit presence but does **not** change the wire type or
   field number. For a decoder this means: treat every singular field as
   "possibly absent". Only `repeated` changes cardinality on the wire.

Field tables below use this notation:

- **Wire**: `varint` (=0), `64` (=1), `LD` (=2), `32` (=5).
- **Rep**: `rep` = repeated; blank = singular.
- **Type**: `string`, `bytes`, `bool`, `int32`, `int64`, `uint32`, `uint64`,
  `double`, or a nested message name. Repeated numeric scalars are marked
  `(packed)` — on the wire they are a single `LD` field.

---

## 2. HTTP transport & request/response envelope

### 2.1 Where the data goes

The protobuf API is a set of `POST` endpoints. The request body is
**multipart/form-data** (not raw protobuf) built by
`buildProtobufRequestBody()` in `ProtobufRequest.kt`:

- Boundary: `--------7da3d81520810*`
- Form part **`client_version`** — only when `clientVersion` is **not**
  `TIEBA_V12` / `TIEBA_V12_POST` (i.e. only for the legacy v11 client).
- Form part **`stoken`** — only when `needSToken=true` (default) and an stoken
  is available. `pbFloor` is sent with `needSToken=false`.
- Form part **`data`** (filename `"file"`) — the raw protobuf bytes of the
  outermost **`*Request`** message (`data.encode()`).

The HTTP **response body** is raw protobuf bytes of the **`*Response`** message
(Retrofit + Square Wire decodes it directly).

### 2.2 Request nesting

```
XxxRequest                 (outermost message, serialized to the multipart "data" bytes)
  ├─ 1: data = XxxRequestData   (length-delimited nested message)
  │      └─ …: common = CommonRequest   (shared device/account fields, see §2.5)
```

There is **no `extra` envelope field** in this reference; the only envelope is
`data` (field number **1**) on requests and `data` (field number **2**) on
responses.

### 2.3 Response nesting

```
XxxResponse
  ├─ 1: error = Error              (length-delimited message)
  └─ 2: data  = XxxResponseData    (length-delimited message)
```

**Exception:** `GetBawuInfoResponse` swaps them:
`data = 1`, `error = 2`.

### 2.4 Endpoints (from `OfficialProtobufTiebaApi.kt`)

| Feature | Endpoint (POST) | Request message | Response message |
|---|---|---|---|
| 吧首页 frs page | `/c/f/frs/page?cmd=301001` (+ header `forum_name`) | `FrsPageRequest` | `FrsPageResponse` |
| 帖子楼层 pb page | `/c/f/pb/page?cmd=302001&format=protobuf` | `PbPageRequest` | `PbPageResponse` |
| 楼中楼 pb floor | `/c/f/pb/floor?cmd=302002&format=protobuf` | `PbFloorRequest` | `PbFloorResponse` |
| 发帖/回帖 | `/c/c/post/add?cmd=309731&format=protobuf` | `AddPostRequest` | `AddPostResponse` |
| 用户主页 profile | `/c/u/user/profile?cmd=303012&format=protobuf` | `ProfileRequest` | `ProfileResponse` |
| 用户信息 | `/c/u/user/getuserinfo?cmd=303024&format=protobuf` | `GetUserInfoRequest` | `GetUserInfoResponse` |
| 吧详情 | `/c/f/forum/getforumdetail?cmd=303021&format=protobuf` | `GetForumDetailRequest` | `GetForumDetailResponse` |
| 吧务列表 | `/c/f/forum/getBawuInfo?cmd=301007&format=protobuf` | `GetBawuInfoRequest` | `GetBawuInfoResponse` |
| 吧规 | `/c/f/forum/forumRuleDetail?cmd=309690&format=protobuf` | `ForumRuleDetailRequest` | `ForumRuleDetailResponse` |

### 2.5 Common shared envelope messages

**`Error`** (package `tieba`) — used as the `error` field of every response.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 error_code` |
| 2 | LD | | `string error_msg` |
| 3 | LD | | `string user_msg` |

**`ProtoCommonResponse`** (package `tieba`) — generic fallback response.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |

**`CommonRequest`** (package `tieba`) — device/account/anti-spoof block embedded
in most `*RequestData` messages. Field numbers are non-contiguous; preserve
them exactly.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 _client_type` |
| 2 | LD | | `string _client_version` |
| 3 | LD | | `string _client_id` |
| 5 | LD | | `string _phone_imei` |
| 6 | LD | | `string from` |
| 7 | LD | | `string cuid` |
| 8 | varint | | `int64 _timestamp` |
| 9 | LD | | `string model` |
| 10 | LD | | `string BDUSS` (optional) |
| 11 | LD | | `string tbs` (optional) |
| 12 | varint | | `int32 net_type` |
| 14 | LD | | `string _phone_newimei` |
| 23 | LD | | `string sign` (optional) |
| 24 | LD | | `string pversion` |
| 25 | LD | | `string _os_version` |
| 26 | LD | | `string brand` |
| 28 | LD | | `string lego_lib_version` |
| 29 | LD | | `string applist` (optional) |
| 30 | LD | | `string stoken` (optional) |
| 31 | LD | | `string z_id` (optional) |
| 32 | LD | | `string cuid_galaxy2` |
| 33 | LD | | `string cuid_gid` (optional) |
| 34 | LD | | `string oaid` (optional) |
| 35 | LD | | `string c3_aid` |
| 36 | LD | | `string sample_id` (optional) |
| 37 | varint | | `int32 scr_w` |
| 38 | varint | | `int32 scr_h` |
| 39 | 64 | | `double scr_dip` |
| 40 | varint | | `int32 q_type` (optional) |
| 41 | varint | | `int32 is_teenager` (optional) |
| 42 | LD | | `string sdk_ver` |
| 43 | LD | | `string framework_ver` |
| 44 | LD | | `string swan_game_ver` |
| 49 | varint | | `int64 active_timestamp` |
| 50 | varint | | `int64 first_install_time` |
| 51 | varint | | `int64 last_update_time` |
| 53 | LD | | `string event_day` |
| 54 | LD | | `string android_id` |
| 55 | varint | | `int32 cmode` |
| 56 | LD | | `string start_scheme` (optional) |
| 57 | varint | | `int32 start_type` |
| 61 | LD | | `string extra` (optional) |
| 62 | LD | | `string user_agent` |
| 63 | varint | | `int32 personalized_rec_switch` |
| 70 | LD | | `string device_score` |

**`Page`** (package `tieba`) — pagination block.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 page_size` |
| 2 | varint | | `int32 offset` |
| 3 | varint | | `int32 current_page` |
| 4 | varint | | `int32 total_count` |
| 5 | varint | | `int32 total_page` |
| 6 | varint | | `int32 has_more` |
| 7 | varint | | `int32 has_prev` |
| 8 | varint | | `int32 cur_good_id` |
| 9 | varint | | `int32 req_num` |
| 10 | varint | | `int32 pnum` |
| 11 | varint | | `int32 tnum` |
| 12 | varint | | `int32 total_num` |
| 13 | varint | | `int32 lz_total_floor` |
| 14 | varint | | `int32 new_total_page` |

**`AppPosInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string ap_mac` (optional) |
| 2 | varint | | `bool ap_connected` (optional) |
| 3 | LD | | `string coordinate_type` (optional) |
| 6 | varint | | `int64 addr_timestamp` (optional) |
| 7 | LD | | `string asp_shown_info` (optional) |

**`Anti`** (package `tieba`) — post/anti-spam flags.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string tbs` |
| 2 | varint | | `int32 ifpost` |
| 3 | varint | | `int32 ifposta` |
| 4 | varint | | `int32 forbid_flag` |
| 5 | LD | | `string forbid_info` |
| 6 | varint | | `int32 block_stat` |
| 7 | varint | | `int32 hide_stat` |
| 8 | varint | | `int32 vcode_stat` |
| 9 | varint | | `int32 days_tofree` |
| 10 | varint | | `int32 has_chance` |
| 11 | varint | | `int32 ifvoice` |
| 24 | LD | rep | `DelThreadText del_thread_text` |

**`DelThreadText`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 text_id` |
| 2 | LD | | `string text_info` |

---

## 3. Content-type conventions (no proto enums)

The `type` fields are plain `int32`, not proto enums. Values observed in the
Kotlin render code (`Extensions.kt`):

**`PbContent.type`:**

| Value | Meaning | Relevant fields |
|---|---|---|
| 0, 9, 27 | plain text | `text` |
| 1 | link | `text` (anchor), `link` (URL) |
| 2 | emoticon | `text`, `c` (emoticon id) |
| 3 | image | `bsize="w,h"`, `originSrc`, `cdnSrc`/`src`, `showOriginalBtn`, `originSize` |
| 4 | @user mention | `text` (name), `uid` |
| 5 | video | `src` (cover), `link` (video), `text` (web url), `bsize="w,h"` |
| 10 | voice | `voiceMD5`, `duringTime` |
| 20 | image (alt) | `src`, `originSrc`/`src`, `showOriginalBtn`, `originSize`, `bsize="w,h"` |

**`Abstract.type`:** `0` = text, `2` = emoticon (`c` = id), `4` = text(link);
all other values are ignored by the renderer.

**`ThreadInfo.threadTypes` / `thread_type` / `PostInfoList.thread_type`** are
`int32`/`uint64` **bitmask/type ints**, not protobuf enums (e.g. good/essence,
top, topic flags). There is no `ThreadType` enum in the `.proto` source.

---

## 4. 吧首页帖子列表 (frs page)

### 4.1 Request/response envelope

**`FrsPageRequest`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `FrsPageRequestData data` |

**`FrsPageRequestData`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string kw` |
| 2 | varint | | `int32 rn` |
| 3 | varint | | `int32 rn_need` |
| 4 | varint | | `int32 is_good` (optional) |
| 5 | varint | | `int32 cid` (optional) |
| 8 | varint | | `int32 with_group` (optional) |
| 11 | varint | | `int32 scr_w` |
| 12 | varint | | `int32 scr_h` |
| 13 | 64 | | `double scr_dip` |
| 14 | varint | | `int32 q_type` |
| 15 | varint | | `int32 pn` |
| 16 | LD | | `string st_type` (optional) |
| 17 | varint | | `int32 ctime` (optional) |
| 18 | varint | | `int32 data_size` (optional) |
| 19 | varint | | `int32 net_error` (optional) |
| 23 | varint | | `int32 class_id` (optional) |
| 27 | varint | | `int32 st_param` (optional) |
| 39 | LD | | `CommonRequest common` |
| 40 | LD | | `string lastids` (optional) |
| 44 | varint | | `int32 category_id` (optional) |
| 45 | LD | | `string yuelaou_locate` (optional) |
| 47 | varint | | `int32 sort_type` (optional) |
| 48 | varint | | `uint64 last_click_tid` (optional) |
| 49 | varint | | `int32 load_type` |
| 50 | LD | | `AppPosInfo app_pos` |
| 51 | LD | | `AdParam ad_param` (frsPage) |
| 52 | LD | | `string obj_locate` (optional) |
| 53 | LD | | `string obj_source` (optional) |
| 55 | varint | | `int32 is_selection` (optional) |
| 56 | varint | | `int32 call_from` (optional) |
| 58 | varint | | `int64 hot_thread_id` (optional) |
| 59 | varint | | `int32 is_default_navtab` (optional) |
| 60 | LD | | `string ad_context_list` (optional) |
| 61 | LD | | `string up_schema` (optional) |
| 62 | LD | | `string ad_ext_params` (optional) |

**`FrsPageResponse`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `FrsPageResponseData data` |

**`FrsPageResponseData`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `User user` |
| 2 | LD | | `ForumInfo forum` (frsPage) |
| 4 | LD | | `Page page` |
| 5 | LD | | `Anti anti` |
| 6 | LD | | `Group group` |
| 7 | LD | rep | `ThreadInfo thread_list` |
| 8 | varint | rep | `int64 thread_id_list` **(packed)** |
| 9 | varint | | `int32 is_new_url` |
| 11 | varint | | `int32 time` |
| 12 | varint | | `int32 ctime` |
| 13 | varint | | `int64 logid` |
| 14 | varint | | `int32 server_time` |
| 17 | LD | rep | `User user_list` |
| 22 | LD | rep | `FrsTabInfo frs_tab_info` |
| 23 | LD | | `ActivityHead activity_head` |
| 37 | LD | | `NavTabInfo nav_tab_info` |
| 105 | LD | | `ForumRuleStatus forum_rule` |

### 4.2 frsPage sub-messages

**`ForumInfo`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 id` |
| 2 | LD | | `string name` |
| 3 | LD | | `string first_class` |
| 4 | LD | | `string second_class` |
| 5 | varint | | `int32 is_exists` |
| 6 | varint | | `int32 is_like` |
| 7 | varint | | `int32 user_level` |
| 8 | LD | | `string level_name` |
| 9 | varint | | `int32 member_num` |
| 10 | varint | | `int32 thread_num` |
| 11 | varint | | `int32 post_num` |
| 12 | varint | | `int32 has_frs_star` |
| 13 | varint | | `int32 cur_score` |
| 14 | varint | | `int32 levelup_score` |
| 15 | LD | | `SignInfo sign_in_info` |
| 17 | LD | rep | `Manager managers` |
| 20 | LD | | `string tids` |
| 21 | LD | rep | `Classify good_classify` |
| 24 | LD | | `string avatar` |
| 25 | LD | | `string slogan` |
| 78 | LD | | `string f_share_img` |
| 79 | LD | | `string forum_share_link` |

**`Forum`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 forum_id` |
| 2 | LD | | `string level1_dir_name` |

**`Manager`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 id` |
| 2 | LD | | `string name` |
| 3 | LD | | `string show_name` |
| 4 | LD | | `string portrait` |

**`Classify`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string name` |
| 2 | varint | | `int64 id` |
| 3 | varint | | `int32 class_id` |
| 4 | LD | | `string class_name` |

**`Group`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 hide_recommend_group` |
| 2 | varint | | `int32 group_count` |

**`NavTabInfo`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | rep | `FrsTabInfo tab` |
| 2 | LD | rep | `FrsTabInfo menu` |
| 3 | LD | rep | `FrsTabInfo head` |

**`FrsTabInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 tabId` |
| 2 | varint | | `int32 tabType` |
| 3 | LD | | `string tabName` |
| 4 | LD | | `string tabUrl` |
| 5 | LD | | `string tabGid` |
| 6 | LD | | `string tabTitle` |
| 7 | varint | | `int32 isGeneralTab` |
| 8 | LD | | `string tabCode` |
| 9 | varint | | `uint32 tabVersion` |
| 10 | varint | | `int32 isDefault` |

**`ActivityHead`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 activity_type` |
| 2 | LD | | `string activity_title` |
| 3 | LD | rep | `HeadImgs head_imgs` |
| 4 | LD | | `Size top_size` |
| 5 | LD | | `string obj_id` |
| 7 | LD | | `string pull_down_url` |
| 8 | varint | | `int32 pull_down_interval` |
| 9 | LD | | `string pull_down_pic_ios` |
| 10 | LD | | `string pull_down_pic_android` |
| 11 | LD | | `string pull_down_exposure_url` |
| 12 | LD | | `string pull_down_click_url` |
| 13 | varint | | `bool is_ad` |
| 14 | LD | | `string pull_down_schema` |
| 15 | LD | | `string pull_down_package_name` |

**`HeadImgs`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string img_url` |
| 2 | LD | | `string pc_url` |
| 3 | LD | | `string title` |
| 4 | LD | | `string subtitle` |
| 5 | LD | | `string btn_text` |
| 6 | LD | | `string tag_name_url` |
| 7 | LD | | `string tag_name_wh` |
| 8 | LD | | `string schema` |
| 9 | LD | rep | `string third_statistics_url` |
| 10 | varint | | `uint32 has_second_page` |
| 11 | LD | | `string package_name` |
| 12 | varint | | `int32 download_is_thirdpage` |
| 13 | LD | | `string download_appname` |
| 14 | LD | | `string download_developer` |
| 15 | LD | | `string download_package_size` |
| 16 | LD | | `string download_url` |
| 17 | LD | | `string download_img` |
| 18 | LD | | `string download_version` |
| 19 | LD | | `string download_user_power` |
| 20 | LD | | `string download_privacy_policy` |
| 21 | LD | | `string download_package_name` |
| 22 | varint | | `int32 download_item_id` |
| 23 | LD | | `string download_appid` |
| 24 | LD | | `string cover_url` |
| 25 | LD | | `string play_url` |
| 26 | LD | | `CoverImageColor cover_image_color` |
| 27 | LD | rep | `VideoImageColor video_image_color` |

**`Size`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 width` |
| 2 | varint | | `int32 height` |

**`AdParam`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 load_count` (optional) |
| 2 | varint | | `int32 refresh_count` (optional) |
| 3 | LD | | `string yoga_lib_version` (optional) |

**`PostTopic`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title_topic` |
| 2 | LD | | `string content_topic` |

**`ForumRuleStatus`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 2 | LD | | `string title` |
| 3 | varint | | `int32 audit_status` |
| 4 | varint | | `int32 has_forum_rule` |

---

## 5. 帖子楼层列表 (pb page)

### 5.1 Request/response envelope

**`PbPageRequest`** (package `tieba.pbPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `PbPageRequestData data` |

**`PbPageRequestData`** (package `tieba.pbPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 pb_rn` (optional) |
| 2 | varint | | `int32 mark` |
| 3 | varint | | `int32 back` (optional) |
| 4 | varint | | `int64 kz` |
| 5 | varint | | `int32 lz` |
| 6 | varint | | `int32 r` |
| 7 | varint | | `int64 pid` (optional) |
| 8 | varint | | `int32 with_floor` |
| 9 | varint | | `int32 floor_rn` |
| 10 | varint | | `int32 weipost` (optional) |
| 11 | varint | | `int32 message_id` |
| 12 | varint | | `int32 s_model` (optional) |
| 13 | varint | | `int32 rn` |
| 14 | varint | | `int32 scr_w` |
| 15 | varint | | `int32 scr_h` |
| 16 | 64 | | `double scr_dip` |
| 17 | varint | | `int32 q_type` |
| 18 | varint | | `int32 pn` |
| 19 | LD | | `string st_type` |
| 20 | varint | | `int32 thread_type` (optional) |
| 21 | varint | | `int32 banner` (optional) |
| 22 | varint | | `int32 arround` |
| 23 | varint | | `int32 last` |
| 24 | LD | | `string msg_click` |
| 25 | LD | | `CommonRequest common` |
| 26 | LD | | `string lastids` |
| 27 | LD | | `string st_from` |
| 28 | LD | | `string st_link` |
| 29 | varint | | `int32 st_stat` |
| 30 | varint | | `int64 st_task` |
| 31 | varint | | `int32 issdk` |
| 32 | LD | | `string query_word` |
| 33 | varint | | `int32 is_comm_reverse` (optional) |
| 34 | varint | | `int32 is_jumpfloor` (optional) |
| 35 | varint | | `int32 jumpfloor_num` (optional) |
| 42 | LD | | `string da_idfa` |
| 43 | LD | | `string platform` |
| 44 | varint | | `uint64 jid` |
| 45 | varint | | `uint64 fid` |
| 46 | LD | | `string jfrom` |
| 47 | LD | | `string yuelaou_locate` |
| 48 | LD | | `string yuelaou_params` |
| 50 | LD | | `string obj_source` (optional) |
| 51 | LD | | `string obj_locate` (optional) |
| 52 | LD | | `string obj_param1` (optional) |
| 53 | LD | | `AppPosInfo app_pos` |
| 54 | varint | | `uint32 from_smart_frs` (optional) |
| 55 | LD | | `string feed_nid` |
| 56 | varint | | `int64 forum_id` (optional) |
| 57 | varint | | `int32 need_repost_recommend_forum` (optional) |
| 58 | LD | | `AdParam ad_param` (pbPage) |
| 59 | varint | | `int32 need_log` |
| 60 | LD | | `string call_url` |
| 61 | LD | | `string shoubai_cuid` |
| 62 | LD | | `string ori_ugc_nid` |
| 63 | LD | | `string ori_ugc_tid` |
| 65 | varint | | `int32 ori_ugc_type` (optional) |
| 66 | LD | | `string ori_ugc_vid` |
| 68 | LD | | `string ad_context_list` |
| 69 | LD | | `string up_schema` |
| 71 | varint | | `int32 from_push` (optional) |
| 72 | LD | | `string ad_ext_params` |
| 73 | varint | | `int64 broadcast_id` (optional) |
| 74 | varint | | `int32 floor_sort_type` |
| 75 | varint | | `int32 source_type` |
| 76 | varint | | `int32 immersion_video_comment_source` (optional) |
| 77 | LD | | `AppTransmitData app_transmit_data` |
| 78 | varint | | `int32 is_fold_comment_req` (optional) |
| 79 | varint | | `int32 is_edit_comment_req` (optional) |
| 80 | varint | | `int32 request_times` (optional) |
| 81 | varint | | `int64 last_pid` (optional) |
| 82 | varint | | `int32 similar_from` (optional) |
| 83 | LD | | `string come_from` |
| 84 | LD | | `string search_query` |

**`PbPageResponse`** (package `tieba.pbPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `PbPageResponseData data` |

**`PbPageResponseData`** (package `tieba.pbPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `User user` |
| 2 | LD | | `SimpleForum forum` |
| 3 | LD | | `Page page` |
| 4 | LD | | `Anti anti` |
| 5 | LD | | `AddPost add_post` (pbPage) |
| 6 | LD | rep | `Post post_list` |
| 7 | varint | | `int32 has_floor` |
| 8 | LD | | `ThreadInfo thread` |
| 9 | LD | | `Lbs location` |
| 10 | varint | | `int32 is_new_url` |
| 11 | LD | rep | `PostBanner post_banner` |
| 12 | LD | | `BannerList banner_list` |
| 13 | LD | rep | `User user_list` |
| 14 | varint | | `int32 server_time` |
| 15 | LD | | `NewsInfo news_info` |
| 16 | LD | rep | `RecommendThread recommend_threads` |
| 17 | LD | rep | `FineBannerPb fine_banner` |
| 18 | LD | | `TwZhiBoAnti twzhibo_anti` |
| 19 | LD | | `SdkTopicThread sdk_topic_thread` |
| 20 | LD | | `PbHotPost hot_post_list` |
| 21 | LD | | `GraffitiRankListInfo graffiti_rank_list_info` |
| 22 | LD | | `AppealInfo appeal_info` |
| 23 | LD | | `GodCard god_card` |
| 24 | LD | rep | `PsRankListItem play_rank_list` |
| 25 | LD | | `RecommendBook recommend_book` |
| 26 | LD | | `AlaLiveInfo ala_info` |
| 27 | LD | | `ForumHeadlineImgInfo forum_headline_img_info` |
| 28 | LD | | `string asp_shown_info` |
| 29 | LD | | `GuessLikeStruct guess_like` |
| 30 | LD | rep | `ThreadInfo feed_thread_list` |
| 31 | varint | | `uint32 is_follow_current_channel` |
| 32 | varint | | `int32 switch_read_open` |
| 33 | LD | | `FeedExtInfo feed_info` |
| 34 | LD | | `PbTopAgreePost top_agree_post_list` |
| 35 | LD | rep | `SimpleForum repost_recommend_forum_list` |
| 36 | LD | rep | `SimpleForum from_forum_list` |
| 37 | varint | | `int64 thread_freq_num` |
| 38 | LD | | `Post first_floor_post` |
| 39 | LD | | `SimpleForum display_forum` |
| 40 | LD | rep | `SimpleUser new_agree_user` |
| 41 | LD | | `string partial_visible_toast` |
| 42 | LD | | `NaGuide na_guide` |
| 43 | LD | | `PbFollowTip follow_tip` |
| 44 | LD | | `string fold_tip` |
| 45 | varint | | `int32 exp_news_today` |
| 46 | varint | | `int32 exp_guide_today` |
| 47 | LD | | `string multi_forum_text` |
| 48 | LD | rep | `RecomTopicList thread_topic` |
| 49 | LD | rep | `PbSortType pb_sort_info` |
| 50 | varint | | `int32 sort_type` |
| 51 | LD | | `ManagerElection manager_election` |
| 52 | LD | rep | `ThreadInfo bjh_recommend` |
| 53 | LD | | `BusinessPromotInfo business_promot_info` |
| 54 | LD | | `Promotion promotion` |
| 55 | LD | | `AlaLiveInfo recom_ala_info` |
| 56 | varint | | `int32 jumptotab1` |
| 57 | LD | | `string jumptotab2` |
| 58 | LD | | `BusinessAccountInfo business_account_info` |
| 59 | LD | rep | `ThreadInfo recom_thread_info` |
| 60 | LD | | `ForumRuleStatus forum_rule` |
| 61 | varint | | `int32 show_adsense` |
| 62 | varint | | `int32 is_black_white` |
| 63 | varint | | `int32 is_official_forum` |
| 64 | LD | | `FloatingIcon floating_icon` |
| 65 | varint | | `int32 is_purchase` |
| 66 | varint | | `int32 pb_notice_type` |
| 67 | LD | | `string pb_notice` |
| 68 | varint | | `int32 has_fold_comment` |
| 70 | varint | | `int64 fold_comment_num` |

### 5.2 pbPage sub-messages

**`AddPost`** (package `tieba.pbPage`) — "追加" prompt attached to a thread.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 create_time` |
| 2 | LD | | `string post_id` |
| 3 | varint | | `int32 already_count` |
| 4 | varint | | `int32 total_count` |
| 5 | LD | | `string last_addition_content` |
| 6 | varint | | `int32 last_addition_time` |
| 7 | LD | | `string warn_msg` |

**`AdParam`** (package `tieba.pbPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 load_count` (optional) |
| 2 | varint | | `int32 refresh_count` |
| 3 | LD | | `string yoga_lib_version` |
| 4 | varint | | `int32 is_req_ad` |

---

## 6. 楼中楼 (sub floor)

### 6.1 Request/response envelope (`/c/f/pb/floor`)

**`PbFloorRequest`** (package `tieba.pbFloor`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `PbFloorRequestData data` |

**`PbFloorRequestData`** (package `tieba.pbFloor`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 kz` |
| 2 | varint | | `int64 pid` (optional) |
| 3 | varint | | `int64 spid` (optional) |
| 4 | varint | | `int32 pn` |
| 5 | varint | | `int32 scr_w` |
| 6 | varint | | `int32 scr_h` |
| 7 | 64 | | `double scr_dip` |
| 8 | LD | | `string st_type` |
| 9 | LD | | `CommonRequest common` |
| 10 | varint | | `int32 is_comm_reverse` (optional) |
| 11 | varint | | `int64 forum_id` |
| 12 | LD | | `string ori_ugc_nid` |
| 13 | LD | | `string ori_ugc_tid` |
| 15 | varint | | `int32 ori_ugc_type` (optional) |
| 16 | LD | | `string ori_ugc_vid` |
| 17 | LD | | `string top_ugc_pid` |

**`PbFloorResponse`** (package `tieba.pbFloor`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `PbFloorResponseData data` |

**`PbFloorResponseData`** (package `tieba.pbFloor`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Page page` |
| 2 | LD | | `Anti anti` |
| 3 | LD | | `Post post` |
| 4 | LD | rep | `SubPostList subpost_list` |
| 5 | LD | | `ThreadInfo thread` |
| 6 | LD | | `SimpleForum forum` |
| 7 | varint | | `int32 server_time` |
| 8 | LD | | `SimpleForum display_forum` |
| 9 | varint | | `int32 is_black_white` |

### 6.2 Sub-post messages (also embedded in `Post`)

**`SubPost`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 pid` |
| 2 | LD | rep | `SubPostList sub_post_list` |

**`SubPostList`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 id` |
| 2 | LD | rep | `PbContent content` |
| 3 | varint | | `uint32 time` |
| 4 | varint | | `int64 author_id` |
| 5 | LD | | `string title` |
| 6 | varint | | `uint32 floor` |
| 7 | LD | | `User author` |
| 8 | varint | | `int32 is_giftpost` |
| 9 | LD | | `Agree agree` |
| 10 | LD | | `Lbs location` |
| 11 | varint | | `int32 is_fake_top` |
| 12 | varint | | `int32 is_author_view` |

**`AddPostList`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 pid` |
| 2 | varint | | `uint32 total_num` |
| 3 | varint | | `uint32 total_count` |
| 4 | LD | rep | `SubPostList add_post_list` |

---

## 7. 发帖/回帖 (add post)

### 7.1 Request/response envelope

**`AddPostRequest`** (package `tieba.addPost`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `AddPostRequestData data` |

**`AddPostRequestData`** (package `tieba.addPost`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `CommonRequest common` |
| 2 | LD | | `string authsid` |
| 3 | LD | | `string sig` |
| 4 | LD | | `string tbs` |
| 5 | LD | | `string video_other` |
| 6 | LD | | `string anonymous` |
| 7 | LD | | `string can_no_forum` |
| 8 | LD | | `string is_feedback` |
| 9 | LD | | `string takephoto_num` |
| 10 | LD | | `string entrance_type` |
| 11 | LD | | `string voice_md5` |
| 12 | LD | | `string during_time` |
| 13 | LD | | `string vcode` |
| 14 | LD | | `string vcode_md5` |
| 15 | LD | | `string vcode_type` |
| 16 | LD | | `string vcode_tag` |
| 17 | LD | | `string topic_id` |
| 18 | LD | | `string new_vcode` |
| 19 | LD | | `string content` |
| 20 | LD | | `string reply_uid` (optional) |
| 21 | LD | | `string meme_text` |
| 22 | LD | | `string meme_cont_sign` |
| 23 | LD | | `string item_id` |
| 24 | LD | | `string comment_head` |
| 25 | LD | | `string works_tag` |
| 26 | LD | | `string fid` |
| 27 | LD | | `string transform_forums` |
| 28 | LD | | `string v_fid` (optional) |
| 29 | LD | | `string v_fname` (optional) |
| 30 | LD | | `string kw` |
| 31 | LD | | `string is_barrage` (optional) |
| 32 | LD | | `string barrage_time` (optional) |
| 33 | LD | | `string st_param` |
| 34 | LD | | `string ptype` |
| 35 | LD | | `string ori_ugc_nid` |
| 36 | LD | | `string ori_ugc_vid` |
| 37 | LD | | `string ori_ugc_tid` |
| 38 | LD | | `string ori_ugc_type` |
| 39 | LD | | `string is_location` |
| 40 | LD | | `string lat` |
| 41 | LD | | `string lng` |
| 42 | LD | | `string name` |
| 43 | LD | | `string sn` |
| 44 | LD | | `string from_fourm_id` |
| 45 | LD | | `string tid` |
| 46 | LD | | `string quote_id` (optional) |
| 47 | LD | | `string is_twzhibo_thread` (optional) |
| 48 | LD | | `string floor_num` (optional) |
| 49 | LD | | `string repostid` (optional) |
| 50 | LD | | `string sub_post_id` (optional) |
| 51 | LD | | `string is_ad` |
| 52 | LD | | `string is_addition` (optional) |
| 53 | LD | | `string is_giftpost` (optional) |
| 54 | LD | | `string st_type` |
| 55 | LD | | `string post_from` (optional) |
| 56 | LD | | `string real_lat` |
| 57 | LD | | `string real_lng` |
| 58 | LD | | `string name_show` |
| 59 | LD | | `string is_works` |
| 60 | LD | | `string is_pictxt` |
| 61 | LD | | `string is_story` |
| 62 | LD | | `string jid` |
| 63 | LD | | `string jfrom` |
| 64 | varint | | `int32 show_custom_figure` (optional) |
| 65 | LD | | `string from_category_id` |
| 66 | LD | | `string to_category_id` |
| 67 | varint | | `int32 is_show_bless` (optional) |

**`AddPostResponse`** (package `tieba.addPost`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `AddPostResponseData data` |

**`AddPostResponseData`** (package `tieba.addPost`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string opgroup` |
| 2 | LD | | `string tid` |
| 3 | LD | | `string pid` |
| 4 | LD | | `string video_id` |
| 5 | LD | | `string msg` |
| 6 | LD | | `string pre_msg` |
| 7 | LD | | `string color_msg` |
| 8 | LD | | `ZhiBoInfoTW twzhibo_info` |
| 9 | LD | | `ReplyExp exp` |
| 10 | LD | | `ContriInfo contri_info` |
| 11 | LD | | `ThreadEasterEgg star_info` |
| 12 | LD | | `Advertisement advertisement` |
| 13 | LD | | `IconStampInfo icon_stamp_info` |
| 14 | LD | | `PostAntiInfo info` |
| 15 | LD | | `Anti anti_stat` |
| 16 | LD | | `TbInteraction tb_hudong` |
| 17 | LD | | `VcodeInfo anti` |
| 18 | LD | | `string ext_msg` |
| 19 | LD | | `Toast toast` |

### 7.2 AddPost response sub-messages

**`PostAntiInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `AccessState access_state` |
| 2 | LD | rep | `string confilter_hitwords` |
| 3 | LD | | `string need_vcode` |
| 4 | LD | | `string vcode_md5` |
| 5 | LD | | `string vcode_prev_type` |
| 6 | LD | | `string vcode_type` |
| 7 | LD | | `string pass_token` |
| 8 | LD | | `string block_content` |
| 9 | LD | | `string block_cancel` |
| 10 | LD | | `string block_confirm` |
| 12 | LD | | `string vcode_pic_url` |
| 13 | LD | | `VcodeExtra vcode_extra` |

**`VcodeInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string vcode_md5` |
| 2 | LD | | `string vcode_pic_url` |
| 3 | LD | | `string vcode_type` |
| 4 | LD | | `VcodeExtra vcode_extra` |

**`VcodeExtra`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string textimg` |
| 2 | LD | | `string slideimg` |
| 3 | LD | | `string endpoint` |
| 4 | LD | | `string successimg` |
| 5 | LD | | `string slideendpoint` |

**`AccessState`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string type` |
| 2 | LD | | `string token` |
| 3 | LD | | `UserSessionInfo userinfo` |

**`ReplyExp`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string pre_msg` |
| 2 | LD | | `string color_msg` |
| 3 | LD | | `string current_level_max_exp` |
| 4 | LD | | `string current_level` |
| 5 | LD | | `string old` |
| 6 | LD | | `string inc` |
| 7 | LD | | `string question_msg` |
| 8 | LD | | `string question_exp` |

**`ContriInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string color_msg` |
| 2 | LD | | `string after_msg` |
| 3 | LD | rep | `ToastConfig toast_config` |

**`TbInteraction`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string content` |

**`ThreadEasterEgg`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string activity_id` |
| 2 | LD | | `string video_url` |
| 3 | LD | | `string pop_text` |
| 4 | LD | | `string pop_imageurl` |
| 5 | LD | | `ShareInfo share_info` |

**`Toast`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 icon_type` |
| 2 | LD | rep | `ToastContent content` |
| 3 | LD | | `string url` |
| 4 | LD | | `string background` |

**`IconStampInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string stamp_title` |
| 2 | LD | | `string stamp_text` |
| 3 | varint | | `int32 stamp_type` |

**`ToastContent`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string text` |
| 2 | varint | | `int32 has_color` |

**`ToastConfig`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string pre_color_msg` |
| 2 | LD | | `string toast_back_image` |

**`ShareInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | | `string content` |
| 3 | LD | | `string url` |
| 4 | LD | | `string imageurl` |

**`Advertisement`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 time` |
| 2 | LD | | `string pic` |
| 3 | LD | | `string pic_click` |
| 4 | LD | | `string jump_link` |
| 5 | LD | | `string advertisement_id` |
| 6 | LD | | `string view_statistics_url` |
| 7 | LD | | `string click_statistics_url` |
| 8 | LD | | `string floating_text` |
| 9 | LD | | `string deeplink` |
| 10 | LD | | `string scheme` |
| 11 | LD | | `string package_name` |
| 12 | LD | | `string display_ad_icon` |

---

## 8. 用户主页 / 用户信息

### 8.1 Profile (`/c/u/user/profile`)

**`ProfileRequest`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `ProfileRequestData data` |

**`ProfileRequestData`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 uid` (optional) |
| 2 | varint | | `uint32 need_post_count` |
| 3 | varint | | `int64 friend_uid` (optional) |
| 4 | varint | | `uint32 is_guest` |
| 5 | LD | | `string st_type` |
| 6 | varint | | `uint32 pn` |
| 7 | varint | | `uint32 rn` |
| 8 | varint | | `uint32 has_plist` |
| 9 | LD | | `CommonRequest common` |
| 10 | varint | | `uint32 scr_w` |
| 11 | varint | | `uint32 scr_h` |
| 12 | varint | | `uint32 q_type` |
| 13 | 64 | | `double scr_dip` |
| 14 | varint | | `int32 is_from_usercenter` |
| 15 | varint | | `int32 page` |
| 16 | LD | | `string friend_uid_portrait` |
| 17 | LD | | `string history_forum_ids` |
| 18 | LD | | `string history_forum_names` |
| 19 | varint | | `int32 need_usergrowth_task` |

**`ProfileResponse`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `ProfileResponseData data` |

**`ProfileResponseData`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `User user` |
| 2 | LD | | `Anti anti_stat` |
| 3 | LD | | `TAInfo tainfo` |
| 4 | LD | rep | `PostInfoList post_list` |
| 5 | LD | | `UserGodInfo user_god_info` |
| 6 | LD | | `UcCard uc_card` |
| 7 | LD | | `Highlist highs` |
| 8 | LD | | `DealWindow window` |
| 9 | LD | | `TbBookrack tbbookrack` |
| 10 | LD | | `Feedback feedback` |
| 11 | LD | | `UserManChannelInfo video_channel_info` |
| 12 | LD | rep | `DynamicInfo dynamic_list` |
| 13 | LD | rep | `ForumDynamic concerned_forum_list` |
| 14 | LD | | `UserAgreeInfo user_agree_info` |
| 15 | LD | | `ModuleInfo module_info` |
| 16 | LD | | `AlaLiveInfo ala_live_info` |
| 17 | LD | | `NicknameInfo nickname_info` |
| 19 | LD | rep | `AlaLiveInfo ala_live_record` |
| 20 | LD | rep | `UserMap url_map` |
| 22 | LD | rep | `BannerImage banner` |
| 23 | LD | rep | `SmartApp recom_naws_list` |
| 24 | LD | | `Namoaixud namoaixud` |
| 25 | LD | rep | `ThreadInfo newest_dynamic_list` |
| 26 | LD | | `GoodsWin goods_win` |
| 27 | LD | | `HotUserRankEntry new_god_rankinfo` |
| 28 | LD | | `string uk` |
| 29 | varint | | `int32 is_black_white` |
| 34 | varint | | `uint32 work_tab_id` |
| 35 | LD | | `FinanceTab finance_tab` |
| 36 | LD | | `MemberBlockInfo block_info` |
| 37 | LD | | `NamoaixudEntry namoaixud_entry` |
| 38 | LD | | `BubbleInfo bubble_info` |
| 39 | LD | | `VipBanner vip_banner` |
| 40 | LD | rep | `UcCardInfo common_card` |
| 41 | LD | rep | `CustomGrid custom_grid` |
| 42 | LD | rep | `CustomGrid more_grid` |

### 8.2 GetUserInfo (`/c/u/user/getuserinfo`)

**`GetUserInfoRequest`** (package `tieba.getUserInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `GetUserInfoRequestData data` |

**`GetUserInfoRequestData`** (package `tieba.getUserInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `CommonRequest common` |
| 2 | varint | | `int64 uid` |
| 3 | varint | | `int32 scr_w` |

**`GetUserInfoResponse`** (package `tieba.getUserInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `GetUserInfoResponseData data` |

**`GetUserInfoResponseData`** (package `tieba.getUserInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `User user` |

### 8.3 Profile sub-messages

**`TAInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | rep | `string foruminfo` |
| 2 | LD | rep | `string groupinfo` |
| 3 | LD | rep | `string friendinfo` |
| 4 | LD | | `CommonDistance distanceinfo` |
| 5 | varint | | `uint32 groupnum` |
| 6 | varint | | `uint32 friendnum` |
| 7 | varint | | `uint32 is_friend` |
| 8 | LD | rep | `ReplyList reply_list` |
| 9 | varint | | `uint32 userClientVersionIsLower` |
| 10 | LD | | `CommonLocation location` |
| 11 | LD | | `string hide_user_feed` |

**`UserAgreeInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 total_agree_num` |
| 2 | varint | | `int64 normal_agree_num` |
| 3 | varint | | `int64 user_agree_num` |
| 4 | varint | | `int64 video_agree_num` |
| 5 | varint | | `int64 ala_agree_num` |
| 6 | varint | | `int32 has_user_agree` |

**`UserGodInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 god_type` |
| 2 | LD | | `GodDetailInfo god_info` |
| 3 | LD | rep | `ForumGodDetailInfo forum_god_list` |
| 4 | varint | | `int32 sex` |
| 5 | varint | | `int32 age` |
| 6 | LD | | `string address` |
| 7 | LD | rep | `ThreadInfo thread_list` |
| 8 | varint | | `int32 cur_page` |
| 9 | LD | | `string total_thread` |

**`ReplyList`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 user_id` |
| 2 | varint | | `int64 friend_id` |
| 3 | LD | | `string message` |
| 4 | varint | | `uint32 time` |

**`NicknameInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 2 | varint | | `int32 left_days` |

**`MemberBlockInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 is_permanent_ban` |
| 2 | varint | | `int32 is_auto_pay` |
| 3 | varint | | `int32 is_ban` |

**`VipBanner`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | | `string sub_title` |
| 3 | LD | | `string button_lable` |
| 4 | LD | | `string bubble` |
| 5 | LD | | `string url` |
| 6 | LD | | `string button_url` |

**`FinanceTab`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | | `string general_tab_text` |
| 3 | LD | | `string general_tab_url` |
| 4 | LD | rep | `FinanceTabItems tabs` |

**`FinanceTabItems`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string tab_name` |
| 2 | LD | | `string tab_url` |
| 3 | LD | | `string icon_url` |
| 4 | LD | | `string desc_text` |
| 5 | LD | | `string tab_bubble` |
| 6 | LD | | `string tab_type` |
| 7 | LD | | `string statistic` |

**`CommonDistance`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint32 distance` |
| 2 | varint | | `uint32 time` |

**`CommonLocation`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string distance` |
| 2 | varint | | `int64 time` |

**`ForumGodDetailInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 god_id` |
| 2 | varint | | `int64 user_id` |
| 3 | LD | | `string intro` |
| 4 | LD | | `string detail_intro` |
| 5 | varint | | `int64 forum_id` |
| 6 | LD | | `string forum_name` |
| 7 | LD | | `string avatar` |

**`GodDetailInfo`** (package `tieba.profile`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 god_id` |
| 2 | varint | | `int64 user_id` |
| 3 | LD | | `string intro` |
| 4 | LD | | `string detail_intro` |

---

## 9. 用户 (User) — shared user object

**`User`** (package `tieba`). The large shared user object used by frs, pb,
profile, and post trees.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 is_login` |
| 2 | varint | | `int64 id` |
| 3 | LD | | `string name` |
| 4 | LD | | `string nameShow` |
| 5 | LD | | `string portrait` |
| 6 | varint | | `int32 no_un` |
| 7 | varint | | `int32 type` |
| 9 | varint | | `int32 userhide` |
| 11 | varint | | `int32 is_manager` |
| 12 | LD | | `string rank` |
| 13 | LD | | `string bimg_url` |
| 14 | varint | | `int32 meizhi_level` |
| 15 | varint | | `int32 is_verify` |
| 16 | varint | | `int32 is_interestman` |
| 17 | LD | rep | `Icon iconinfo` |
| 19 | varint | | `int32 user_type` |
| 20 | varint | | `int32 is_coreuser` |
| 21 | varint | | `int32 is_huinibuke` |
| 22 | LD | | `string ios_bimg_format` |
| 23 | varint | | `int32 level_id` |
| 24 | varint | | `int32 is_like` |
| 25 | varint | | `int32 is_bawu` |
| 26 | LD | | `string bawu_type` |
| 27 | LD | | `string portraith` |
| 28 | LD | | `string ip` |
| 29 | LD | | `string BDUSS` |
| 30 | varint | | `int32 fans_num` |
| 31 | varint | | `int32 concern_num` |
| 32 | varint | | `int32 sex` |
| 33 | varint | | `int32 my_like_num` |
| 34 | LD | | `string intro` |
| 35 | varint | | `int32 has_concerned` |
| 36 | LD | | `string passwd` |
| 37 | varint | | `int32 post_num` |
| 38 | LD | | `string tb_age` |
| 39 | varint | | `int32 is_mem` |
| 40 | varint | | `int32 bimg_end_time` |
| 42 | varint | | `int32 gender` |
| 43 | varint | | `int32 is_mask` |
| 44 | LD | rep | `UserPics user_pics` |
| 45 | LD | | `PrivSets privSets` |
| 46 | varint | | `int32 is_friend` |
| 47 | LD | rep | `LikeForumInfo likeForum` |
| 49 | varint | | `int32 gift_num` |
| 51 | varint | | `int32 is_select_tail` |
| 52 | varint | | `int32 is_guanfang` |
| 53 | varint | | `int32 bookmark_count` |
| 54 | varint | | `int32 bookmark_new_count` |
| 55 | LD | rep | `SimpleUser mute_user` |
| 56 | varint | | `int64 friend_num` |
| 57 | LD | | `string fansNickname` |
| 58 | LD | | `string bg_pic` |
| 62 | LD | | `GodInfo god_data` |
| 63 | varint | | `int32 heavy_user` |
| 75 | varint | | `int32 visitor_num` |
| 76 | varint | | `int32 total_visitor_num` |
| 86 | varint | | `int32 nickname_update_time` |
| 87 | varint | | `int32 thread_num` |
| 88 | varint | | `int32 agree_num` |
| 89 | varint | | `int32 left_call_num` |
| 90 | varint | | `int32 is_invited` |
| 91 | varint | | `int32 is_fans` |
| 92 | varint | | `int32 priv_thread` |
| 93 | varint | | `int32 is_videobiggie` |
| 94 | varint | | `int32 is_show_redpacket` |
| 96 | LD | | `BirthdayInfo birthday_info` |
| 97 | varint | | `int32 can_modify_avatar` |
| 98 | LD | | `string modify_avatar_desc` |
| 99 | varint | | `int32 influence` |
| 100 | LD | | `string level_influence` |
| 101 | LD | | `NewGodInfo new_god_data` (optional) |
| 103 | LD | | `BawuThrones bawu_thrones` |
| 105 | LD | | `BazhuSign bazhu_grade` |
| 106 | varint | | `int32 isDefaultAvatar` |
| 109 | varint | | `int32 favorite_num` |
| 118 | varint | | `uint32 total_agree_num` |
| 120 | LD | | `string tieba_uid` |
| 125 | LD | | `string level_name` |
| 127 | LD | | `string ip_address` |
| 128 | varint | | `int32 is_nickname_editing` |
| 129 | LD | | `string editing_nickname` |
| 138 | LD | | `string display_intro` |
| 139 | LD | rep | `string new_icon_url` |
| 140 | LD | | `string dynamic_url` |

**User sub-messages**

**`Icon`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string name` |
| 2 | varint | | `int32 weight` |
| 3 | LD | | `string url` |
| 4 | LD | | `string icon` |
| 5 | varint | | `int32 value` |
| 6 | LD | | `Terminal terminal` |
| 7 | LD | | `Position position` |
| 8 | LD | rep | `string sprite_info` |

**`Terminal`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 pc` |
| 2 | varint | | `int32 wap` |
| 3 | varint | | `int32 client` |

**`Position`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 frs` |
| 2 | varint | | `int32 pb` |
| 3 | varint | | `int32 home` |
| 4 | varint | | `int32 card` |

**`UserPics`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string big` |
| 2 | LD | | `string small` |

**`PrivSets`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 2 | varint | | `int32 like` |
| 3 | varint | | `int32 group` |
| 4 | varint | | `int32 post` |
| 6 | varint | | `int32 live` |

**`LikeForumInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string forum_name` |
| 2 | varint | | `uint64 forum_id` |

**`GodInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 id` |
| 2 | LD | | `string intro` |
| 3 | varint | | `int32 type` |
| 4 | varint | | `int64 fid` |
| 5 | varint | | `int32 followed` |
| 6 | LD | | `string recommend_reason` |
| 7 | LD | | `string forum_name` |
| 8 | varint | | `int32 can_send_msg` |
| 9 | LD | | `string prefix` |

**`NewGodInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 status` |
| 2 | varint | | `uint32 field_id` |
| 3 | LD | | `string field_name` |
| 4 | varint | | `uint32 type` |
| 5 | LD | | `string type_name` |

**`BirthdayInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 birthday_time` |
| 2 | varint | | `uint32 birthday_show_status` |
| 3 | LD | | `string constellation` |
| 4 | varint | | `uint32 age` |

**`SimpleUser`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 user_id` |
| 2 | varint | | `int32 user_status` |
| 3 | LD | | `string secureemail` |
| 4 | LD | | `string securemobil` |
| 5 | LD | | `string user_name` |
| 6 | LD | | `string user_nickname` |
| 7 | varint | | `uint32 incomplete_user` |
| 8 | LD | | `string portrait` |
| 9 | varint | | `int32 agree_type` |
| 10 | LD | | `string ahead_url` |
| 11 | LD | | `string block_msg` |
| 12 | varint | | `int32 show_onlyme` |

---

## 10. 帖子 (Post) — shared post/floor object

**`Post`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 id` |
| 2 | LD | | `string title` |
| 3 | varint | | `uint32 floor` |
| 4 | varint | | `uint32 time` |
| 5 | LD | rep | `PbContent content` |
| 6 | LD | rep | `string arr_video` |
| 7 | LD | | `Lbs lbs_info` |
| 8 | varint | | `uint32 is_vote` |
| 9 | varint | | `uint32 is_voice` |
| 10 | varint | | `uint32 is_ntitle` |
| 11 | varint | | `uint32 is_bub` |
| 12 | LD | | `string vote_crypt` |
| 13 | varint | | `uint32 sub_post_number` |
| 14 | LD | | `string time_ex` |
| 15 | LD | | `SubPost sub_post_list` |
| 16 | LD | | `AddPostList add_post_list` |
| 17 | LD | | `string bimg_url` |
| 18 | LD | | `string ios_bimg_format` |
| 19 | varint | | `int64 author_id` |
| 20 | varint | | `uint32 add_post_number` |
| 21 | LD | | `SignatureData signature` |
| 22 | LD | | `TailInfo tail_info` |
| 23 | LD | | `User author` |
| 24 | LD | | `Zan zan` |
| 25 | varint | | `int32 storecount` |
| 26 | LD | | `TPointPost tpoint_post` |
| 27 | LD | | `ActPost act_post` |
| 28 | LD | | `PbPresent present` |
| 29 | LD | | `VideoInfo video_info` |
| 30 | LD | | `PbPostZan post_zan` |
| 31 | varint | | `int32 is_hot_post` |
| 32 | LD | rep | `TailInfo ext_tails` |
| 33 | LD | | `TogetherHi high_together` |
| 34 | LD | | `SkinInfo skin_info` |
| 35 | LD | | `DealInfo pb_deal_info` |
| 36 | LD | | `string lego_card` |
| 37 | LD | | `Agree agree` |
| 38 | LD | | `SimpleForum from_forum` |
| 39 | varint | | `int32 is_post_visible` |
| 40 | varint | | `int32 need_log` |
| 41 | varint | | `int32 img_num_abtest` |
| 42 | LD | | `OriginThreadInfo origin_thread_info` |
| 43 | varint | | `int32 is_fold` |
| 44 | LD | | `string fold_tip` |
| 45 | varint | | `int32 is_top_agree_post` |
| 46 | varint | | `int64 tid` |
| 47 | varint | | `int32 show_squared` |
| 48 | varint | | `int32 is_bjh` |
| 50 | LD | | `string quote_id` |
| 51 | varint | | `int32 is_wonderful_post` |
| 52 | LD | rep | `HeadItem item_star` |
| 53 | LD | | `Item item` |
| 54 | LD | | `Item outer_item` |
| 55 | LD | | `Advertisement advertisement` |
| 56 | varint | | `int32 fold_comment_status` |
| 57 | LD | | `string fold_comment_apply_url` |
| 58 | LD | | `NovelInfo novel_info` |

**Post sub-messages (content)**

**`PbContent`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 type` |
| 2 | LD | | `string text` |
| 3 | LD | | `string link` |
| 4 | LD | | `string src` |
| 5 | LD | | `string bsize` |
| 6 | LD | | `string bigSrc` |
| 7 | LD | | `string bigSize` |
| 8 | LD | | `string cdnSrc` |
| 9 | LD | | `string bigCdnSrc` |
| 10 | LD | | `string imgType` |
| 11 | LD | | `string c` |
| 12 | LD | | `string voiceMD5` |
| 13 | varint | | `uint32 duringTime` |
| 15 | varint | | `int64 uid` |
| 16 | LD | | `string dynamic` |
| 17 | LD | | `string _static` |
| 18 | varint | | `uint32 width` |
| 19 | varint | | `uint32 height` |
| 25 | LD | | `string originSrc` |
| 27 | varint | | `uint32 originSize` |
| 31 | LD | | `string mediaSubtitle` |
| 32 | varint | | `int32 urlType` |
| 33 | LD | | `MemeInfo memeInfo` |
| 34 | varint | | `uint32 isLongPic` |
| 35 | varint | | `uint32 showOriginalBtn` |
| 36 | LD | | `string cdnSrcActive` |

**`Abstract`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 type` |
| 2 | LD | | `string text` |
| 3 | LD | | `string link` |
| 4 | LD | | `string src` |
| 5 | LD | | `string un` |
| 6 | LD | | `string duringTime` |
| 7 | LD | | `string voiceMD5` |

**`Agree`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 agreeNum` |
| 2 | varint | | `int32 hasAgree` |
| 3 | varint | | `int32 agreeType` |
| 4 | varint | | `int64 disagreeNum` |
| 5 | varint | | `int64 diffAgreeNum` |

**`Zan`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 num` |
| 2 | LD | rep | `User liker_list` |
| 3 | varint | | `int32 is_liked` |
| 4 | varint | | `int32 last_time` |
| 5 | varint | rep | `int64 liker_id` **(packed)** |
| 6 | varint | | `int32 consent_type` |

**`Lbs`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string lat` |
| 2 | LD | | `string lng` |
| 3 | LD | | `string name` |
| 4 | LD | | `string sn` |
| 5 | LD | | `string distance` |

**`LbsInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string lat` |
| 2 | LD | | `string lon` |
| 3 | LD | | `string town` |

**`Voice`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 type` |
| 2 | varint | | `int32 during_time` |
| 3 | LD | | `string voice_md5` |

**`Media`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 type` |
| 3 | LD | | `string bigPic` |
| 8 | LD | | `string srcPic` |
| 10 | varint | | `uint32 width` |
| 11 | varint | | `uint32 height` |
| 15 | LD | | `string originPic` |
| 16 | varint | | `uint32 originSize` |
| 17 | varint | | `int64 postId` |
| 18 | LD | | `string dynamicPic` |
| 19 | varint | | `uint32 isLongPic` |
| 20 | varint | | `uint32 showOriginalBtn` |

**`VideoInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string videoMD5` |
| 2 | LD | | `string videoUrl` |
| 3 | varint | | `uint32 videoDuration` |
| 4 | varint | | `uint32 videoWidth` |
| 5 | varint | | `uint32 videoHeight` |
| 6 | LD | | `string thumbnailUrl` |
| 7 | varint | | `uint32 thumbnailWidth` |
| 8 | varint | | `uint32 thumbnailHeight` |
| 11 | LD | | `string mediaSubtitle` |

**`OriginThreadInfo`** (package `tieba`) — quoted/forwarded thread.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | rep | `Media media` |
| 3 | LD | rep | `Abstract _abstract` |
| 4 | LD | | `string fname` |
| 5 | LD | | `string tid` |
| 6 | LD | | `AlaLiveInfo ala_info` |
| 7 | varint | | `int64 fid` |
| 8 | varint | | `int32 thread_type` |
| 9 | varint | | `int32 is_deleted` |
| 10 | varint | | `int32 is_ugc` |
| 11 | LD | | `Baijiahao ori_ugc_info` |
| 12 | LD | rep | `Voice voice_info` |
| 13 | LD | | `VideoInfo video_info` |
| 14 | LD | rep | `PbContent content` |
| 15 | varint | | `int32 is_new_style` |
| 16 | varint | | `int32 reply_num` |
| 18 | LD | | `User author` |
| 19 | LD | | `Agree agree` |
| 20 | varint | | `int32 shared_num` |
| 21 | LD | | `PollInfo poll_info` |
| 22 | LD | | `Item item` |
| 23 | LD | rep | `HeadItem item_star` |
| 24 | LD | rep | `PbLinkInfo pb_link_info` |
| 25 | varint | | `int64 pid` |
| 26 | varint | | `int32 good_types` |
| 27 | varint | | `int32 top_types` |
| 28 | varint | | `int32 is_frs_mask` |

**`Quote`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 post_id` |
| 2 | LD | | `string user_name` |
| 3 | varint | | `int64 user_id` |
| 4 | LD | | `string ip` |
| 5 | LD | | `string content` |

**`SignatureData`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 signature_id` |
| 2 | LD | | `string fontKeyName` |
| 3 | LD | | `string fontColor` |
| 4 | LD | rep | `SignatureContent content` |

**`SignatureContent`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 type` |
| 2 | LD | | `string text` |

**`TailInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 tail_type` |
| 2 | LD | | `string icon_url` |
| 3 | LD | | `string icon_link` |
| 4 | LD | | `string content` |

**`DealInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | | `string des` |
| 3 | varint | | `uint64 stock` |
| 4 | varint | | `uint64 sales` |
| 5 | varint | | `uint32 expire_time` |
| 6 | varint | | `uint64 unit_price` |
| 7 | varint | | `uint64 product_id` |
| 8 | LD | | `string seller_address` |
| 9 | varint | | `int64 recommendations` |
| 10 | varint | | `bool has_recommend` |
| 11 | varint | | `int32 status` |
| 12 | LD | rep | `DealMedia media` |
| 13 | LD | rep | `DealAuthInfo auth_info` |
| 14 | varint | | `uint64 ship_fee` |

**`DealAuthInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string item_name` |
| 2 | LD | | `string item_content` |
| 3 | LD | | `string item_url` |

**`DealMedia`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 type` |
| 2 | LD | | `string small_pic` |
| 3 | LD | | `string big_pic` |
| 4 | LD | | `string water_pic` |

**`DealWindow`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | rep | `DisplayWindowInfo list` |
| 2 | varint | | `uint64 total` |

**`HeadItem`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string name` |
| 2 | LD | | `string content` |
| 3 | varint | | `int32 type` |

**`Item`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 item_id` |
| 2 | LD | | `string item_name` |
| 3 | 64 | | `double icon_size` |
| 4 | LD | | `string icon_url` |
| 5 | LD | rep | `string tags` |
| 6 | 64 | | `double score` |
| 7 | varint | | `int32 star` |
| 8 | LD | | `string button_name` |
| 9 | LD | | `string button_link` |
| 10 | LD | | `string item_appid` |
| 11 | varint | | `int32 category_id` |
| 12 | varint | | `int32 button_link_type` |
| 13 | LD | | `string apk_name` |
| 14 | LD | | `string forum_name` |
| 15 | LD | | `ApkDetail apk_detail` |

**`PbLinkInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | | `string to_url` |
| 3 | LD | | `string pic_url` |
| 4 | LD | | `string link_from` |
| 5 | LD | | `string ext_txt` |
| 6 | varint | | `uint32 sort` |
| 7 | varint | | `int32 url_type` |

**`PostInfoContent`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | rep | `Abstract post_content` |
| 2 | varint | | `uint64 create_time` |
| 3 | varint | | `uint64 post_type` |
| 4 | varint | | `uint64 post_id` |
| 5 | varint | | `int32 is_author_view` |
| 7 | LD | | `string target_scheme` |

**`PostInfoList`** (package `tieba`) — profile post list item.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 forum_id` |
| 2 | varint | | `uint64 thread_id` |
| 3 | varint | | `uint64 post_id` |
| 4 | varint | | `uint32 is_thread` |
| 5 | varint | | `uint32 create_time` |
| 6 | LD | | `string forum_name` |
| 7 | LD | | `string title` |
| 8 | LD | rep | `PostInfoContent content` |
| 9 | LD | | `string content_thread` |
| 10 | LD | | `string user_name` |
| 11 | LD | | `string ip` |
| 12 | varint | | `uint32 is_post_deleted` |
| 13 | LD | | `string ptype` |
| 14 | LD | | `string _abstract` |
| 15 | LD | rep | `Abstract abstract_thread` |
| 16 | LD | rep | `Media media` |
| 17 | varint | | `uint32 reply_num` |
| 18 | varint | | `int64 user_id` |
| 19 | LD | | `string user_portrait` |
| 20 | LD | | `string post_type` |
| 21 | LD | | `LbsInfo lbs_info` |
| 22 | LD | | `Quote quote` |
| 23 | LD | rep | `Voice voice_info` |
| 24 | LD | | `AnchorInfo anchor_info` |
| 25 | varint | | `int32 hide_post` |
| 26 | varint | | `uint64 thread_type` |
| 27 | LD | | `ZhiBoInfoTW twzhibo_info` |
| 28 | LD | | `PollInfo poll_info` |
| 29 | LD | | `VideoInfo video_info` |
| 30 | varint | | `bool is_deal` |
| 31 | LD | | `DealInfo deal_info` |
| 32 | LD | rep | `MultipleForum multiple_forum_list` |
| 33 | varint | | `int32 freq_num` |
| 34 | varint | | `uint64 v_forum_id` |
| 35 | LD | | `string name_show` |
| 36 | LD | | `AlaLiveInfo ala_info` |
| 37 | varint | | `int32 agree_num` |
| 38 | varint | | `int32 view_num` |
| 39 | varint | | `int32 share_num` |
| 40 | LD | | `Agree agree` |
| 41 | varint | | `int32 is_remain` |
| 42 | LD | | `OriginThreadInfo origin_thread_info` |
| 43 | varint | | `int32 is_view_year` |
| 44 | varint | | `int32 is_share_thread` |
| 45 | LD | rep | `PbContent rich_title` |
| 46 | LD | rep | `PbContent rich_abstract` |
| 47 | varint | | `int32 is_ntitle` |
| 48 | LD | | `string article_cover` |
| 49 | LD | rep | `PbContent first_post_content` |
| 50 | LD | | `BaijiahaoInfo baijiahao_info` |
| 51 | LD | | `string wonderful_post_info` |
| 52 | LD | | `Item item` |
| 53 | LD | rep | `HeadItem item_star` |
| 54 | LD | rep | `PbLinkInfo pb_link_info` |
| 56 | LD | rep | `PrivSets priv_sets` |
| 57 | varint | | `int32 is_author_view` |
| 59 | varint | | `int32 is_manager` |
| 60 | varint | | `int32 is_origin_manager` |
| 61 | varint | | `int32 good_types` |
| 62 | varint | | `int32 top_types` |
| 63 | LD | | `UserPostPerm user_post_perm` |
| 66 | LD | | `string target_scheme` |

---

## 11. 帖子列表项 (ThreadInfo) — shared thread object

**`ThreadInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 id` |
| 2 | varint | | `int64 threadId` |
| 3 | LD | | `string title` |
| 4 | varint | | `int32 replyNum` |
| 5 | varint | | `int32 viewNum` |
| 6 | LD | | `string lastTime` |
| 7 | varint | | `int32 lastTimeInt` |
| 8 | varint | | `int32 threadTypes` |
| 9 | varint | | `int32 isTop` |
| 10 | varint | | `int32 isGood` |
| 18 | LD | | `User author` |
| 19 | LD | | `User lastReplyer` |
| 20 | varint | | `int32 commentNum` |
| 21 | LD | rep | `Abstract _abstract` |
| 22 | LD | rep | `Media media` |
| 27 | varint | | `int64 forumId` |
| 28 | LD | | `string forumName` |
| 38 | varint | | `int32 isNoTitle` |
| 40 | varint | | `int64 firstPostId` |
| 45 | varint | | `int32 createTime` |
| 50 | varint | | `int32 collectStatus` |
| 51 | LD | | `string collectMarkPid` |
| 52 | varint | | `int64 post_id` |
| 54 | varint | | `int32 isMemberTop` |
| 56 | varint | | `int64 authorId` |
| 61 | LD | | `string pids` |
| 72 | LD | | `ZhiBoInfoTW twzhibo_info` |
| 79 | LD | | `VideoInfo videoInfo` (optional) |
| 111 | LD | rep | `PbContent richTitle` |
| 112 | LD | rep | `PbContent richAbstract` |
| 113 | LD | | `AlaLiveInfo ala_info` (optional) |
| 120 | LD | rep | `DislikeInfo dislikeInfo` |
| 124 | varint | | `int32 agreeNum` |
| 126 | LD | | `Agree agree` |
| 135 | varint | | `int64 shareNum` |
| 141 | LD | | `OriginThreadInfo origin_thread_info` |
| 142 | LD | rep | `PbContent firstPostContent` |
| 143 | varint | | `int32 is_share_thread` |
| 148 | varint | | `int32 isTopic` |
| 149 | LD | | `string topicUserName` |
| 150 | LD | | `string topicH5Url` |
| 155 | LD | | `SimpleForum forumInfo` |
| 159 | LD | | `string tShareImg` |
| 164 | LD | | `string nid` |
| 175 | varint | | `int32 tabId` |
| 176 | LD | | `string tabName` |
| 181 | varint | | `int32 isDeleted` |
| 182 | varint | | `int32 hotNum` |

**`SimpleForum`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 id` |
| 2 | LD | | `string name` |
| 4 | LD | | `string avatar` |
| 12 | varint | | `int32 memberNum` |
| 13 | varint | | `int32 postNum` |

**`SimpleThreadInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 tid` |
| 2 | LD | | `string title` |
| 3 | varint | | `int32 reply_num` |
| 4 | varint | | `int32 last_time_int` |
| 5 | LD | rep | `Abstract _abstract` |
| 6 | LD | | `Zan zan` |
| 7 | varint | | `uint64 thread_type` |

---

## 12. 吧详情 (forum detail)

### 12.1 GetForumDetail envelope

**`GetForumDetailRequest`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `GetForumDetailRequestData data` |

**`GetForumDetailRequestData`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 forum_id` |
| 2 | LD | | `CommonRequest common` |

**`GetForumDetailResponse`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `GetForumDetailResponseData data` |

**`GetForumDetailResponseData`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `RecommendForumInfo forum_info` |
| 2 | LD | rep | `SimpleThreadInfo thread_list` |
| 4 | varint | | `int32 is_bawu_show` |
| 5 | LD | | `ManagerApplyInfo bz_apply_info` (root) |
| 6 | varint | | `int32 is_complaint_show` |
| 7 | LD | | `PriManagerApplyInfo pribz_apply_info` |
| 8 | LD | | `ManagerElectionTab election_tab` |
| 9 | varint | | `int32 is_forum_data_show` |
| 10 | LD | | `ForumDataCenter forum_data` |
| 11 | LD | rep | `BawuAction bawu_actions` |
| 12 | LD | | `ApplyStatus apply_status` |
| 13 | LD | | `BazhuUniversity bazhu_university` |
| 15 | LD | | `BazhuGrade bazhu_grade` |
| 16 | varint | | `int32 is_forum_card_enable` |
| 17 | LD | | `BawuThrones bawu_thrones` |
| 18 | LD | | `string is_bazhu_show` |
| 19 | LD | | `HotUserRankEntry hot_user_entry` |
| 20 | LD | | `ServiceArea small_app` |
| 21 | LD | | `ForumMemberInfo forum_member` |

### 12.2 Forum detail sub-messages

**`RecommendForumInfo`** (package `tieba`) — the forum card.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string avatar` |
| 2 | varint | | `uint64 forum_id` |
| 3 | LD | | `string forum_name` |
| 4 | varint | | `uint32 is_like` |
| 5 | varint | | `uint32 member_count` |
| 6 | varint | | `uint32 thread_count` |
| 7 | LD | | `string slogan` |
| 8 | LD | rep | `PbContent content` |
| 9 | varint | | `uint32 forum_type` |
| 10 | LD | | `string authen` |
| 11 | LD | | `string recom_reason` |
| 12 | varint | | `uint32 is_brand_forum` |
| 13 | LD | | `string hot_text` |
| 14 | LD | | `string abtest_tag` |
| 15 | LD | | `string source` |
| 16 | LD | | `string extra` |
| 17 | varint | | `uint32 is_private_forum` |
| 18 | LD | | `string lv1_name` |
| 19 | LD | | `string lv2_name` |
| 20 | LD | | `string avatar_origin` |
| 22 | varint | | `uint64 hot_thread_id` |
| 23 | varint | | `int32 is_recommend_forum` |

**`ManagerApplyInfo`** (package `tieba`) — used by GetForumDetail.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 manager_left_num` |
| 2 | LD | | `string manager_apply_url` |
| 3 | varint | | `int32 manager_apply_status` |

**`PriManagerApplyInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 assist_left_num` |
| 2 | LD | | `string assist_apply_url` |
| 3 | varint | | `int32 assist_apply_status` |

**`ForumMemberInfo`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string total` |
| 2 | LD | | `string title` |
| 3 | LD | rep | `User member_list` |

**`BawuAction`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string name` |
| 2 | varint | | `int32 type` |
| 3 | LD | | `string url` |

**`ApplyStatus`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 is_avatar_applying` |
| 2 | varint | | `int32 is_desc_applying` |
| 3 | varint | | `int32 next_avatar_apply_time` |
| 4 | varint | | `int32 next_desc_apply_time` |
| 5 | varint | | `int32 is_fdir_applying` |
| 6 | varint | | `int32 fdir_next_apply_time` |

**`BazhuGrade`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string grade` |
| 2 | LD | rep | `GradePoint grade_point` |
| 3 | LD | | `string estimation_left_text` |
| 4 | LD | | `string grade_text` |
| 5 | varint | | `int32 estimation_left_time` |

**`BazhuUniversity`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | rep | `UniversityInfo entrance` |
| 2 | LD | rep | `UniversityInfo banner` |
| 3 | LD | rep | `UniversityTabInfo tab` |

**`ManagerElectionTab`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint32 is_new_strategy` |
| 2 | LD | | `string new_strategy_link` |
| 4 | varint | | `uint32 new_manager_status` |
| 5 | LD | | `string new_strategy_text` |
| 6 | LD | | `string toast_text` |

**`ForumDataCenter`** (package `tieba.getForumDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 new_pv_cnt` |
| 2 | varint | | `int32 new_pv_cnt_diff` |
| 3 | varint | | `int32 new_thread_cnt` |
| 4 | varint | | `int32 new_thread_cnt_diff` |
| 5 | varint | | `int32 new_follow_cnt` |
| 6 | varint | | `int32 new_follow_cnt_diff` |
| 7 | varint | | `int32 user_duration_avg` |
| 8 | 64 | | `double user_duration_avg_diff` |
| 9 | 64 | | `double user_sign_rate` |
| 10 | 64 | | `double user_sign_rate_diff` |
| 11 | varint | | `int32 homepage_thread_cnt` |
| 12 | varint | | `int32 homepage_thread_cnt_diff` |

**`HotUserRankEntry`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | rep | `ShortUserInfo hot_user` |
| 2 | LD | | `string module_name` |
| 3 | LD | | `string module_icon` |
| 4 | varint | | `uint32 today_rank` |
| 5 | varint | | `uint32 yesterday_rank` |
| 6 | varint | | `bool is_in_rank` |

**`ServiceArea`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string servicename` |
| 2 | LD | | `string picurl` |
| 3 | LD | | `string serviceurl` |
| 4 | LD | | `string version` |
| 5 | LD | | `string service_type` |
| 6 | LD | | `SmartApp area_smart_app` |
| 7 | LD | | `string schema` |
| 8 | LD | rep | `string third_statistics_url` |

### 12.3 吧务 (Bawu) — GetBawuInfo

**`GetBawuInfoRequest`** (package `tieba.getBawuInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `GetBawuInfoRequestData data` |

**`GetBawuInfoRequestData`** (package `tieba.getBawuInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `CommonRequest common` |
| 2 | varint | | `uint64 forum_id` |

**`GetBawuInfoResponse`** (package `tieba.getBawuInfo`) — **note reversed field order**.

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `GetBawuInfoResponseData data` |
| 2 | LD | | `Error error` |

**`GetBawuInfoResponseData`** (package `tieba.getBawuInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `BawuTeam bawu_team_info` |
| 2 | LD | | `ManagerApplyInfo manager_apply_info` (getBawuInfo) |
| 3 | varint | | `int32 is_private_forum` |

**`ManagerApplyInfo`** (package `tieba.getBawuInfo`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 manager_left_num` |
| 2 | LD | | `string manager_apply_url` |
| 3 | varint | | `int32 assist_left_num` |
| 4 | LD | | `string assist_apply_url` |

**`BawuTeam`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 total_num` |
| 2 | LD | rep | `BawuRoleDes bawu_team_list` |

**`BawuRoleDes`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string role_name` |
| 2 | LD | rep | `BawuRoleInfoPub role_info` |

**`BawuRoleInfoPub`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint64 forum_id` |
| 2 | varint | | `int64 user_id` |
| 3 | varint | | `int32 role_id` |
| 4 | LD | | `string role_name` |
| 5 | LD | | `string portrait` |
| 6 | varint | | `int32 user_level` |
| 7 | LD | | `string level_name` |
| 8 | LD | | `string user_name` |
| 9 | LD | | `string name_show` |
| 10 | LD | | `BaijiahaoInfo baijiahao_info` |

**`BawuThrones`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 total_recommend_num` |
| 2 | varint | | `int32 used_recommend_num` |
| 3 | LD | | `string bazhu_level` |
| 4 | varint | | `int32 used_bcast_cnt` |
| 5 | varint | | `int32 total_bcast_cnt` |
| 6 | varint | | `int32 newest_bcast_pv` |
| 7 | varint | | `int32 has_send_bcast` |
| 8 | varint | | `int32 newest_bcast_pushuser_cnt` |

### 12.4 吧规 (ForumRuleDetail)

**`ForumRuleDetailRequest`** (package `tieba.forumRuleDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `ForumRuleDetailRequestData data` |

**`ForumRuleDetailRequestData`** (package `tieba.forumRuleDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 forum_id` |
| 2 | LD | | `CommonRequest common` |

**`ForumRuleDetailResponse`** (package `tieba.forumRuleDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `Error error` |
| 2 | LD | | `ForumRuleDetailResponseData data` |

**`ForumRuleDetailResponseData`** (package `tieba.forumRuleDetail`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 2 | LD | | `ForumInfo forum` (root) |
| 3 | LD | | `string title` |
| 4 | LD | | `string preface` |
| 5 | LD | rep | `ForumRule rules` |
| 6 | varint | | `int32 audit_status` |
| 7 | LD | | `string audit_opinion` |
| 8 | varint | | `int32 is_manager` |
| 9 | varint | | `int64 forum_rule_id` |
| 10 | LD | | `string publish_time` |
| 11 | LD | | `BawuRoleInfoPub bazhu` |
| 12 | LD | | `string cur_time` |

**`ForumRule`** (package `tieba`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string title` |
| 2 | LD | rep | `PbContent content` |
| 3 | varint | | `int32 status` |

**`ForumInfo`** (package `tieba` — root package, distinct from frsPage ForumInfo).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `uint32 forum_id` |
| 2 | LD | | `string forum_name` |
| 3 | LD | | `string avatar` |
| 4 | LD | | `string post_num` |
| 5 | LD | | `string concern_num` |
| 6 | varint | | `int32 has_concerned` |

---

## 13. 签到 (sign)

The sign **status** protos below are embedded in frs page responses. The sign
**action** endpoint (`/c/c/forum/sign` and `/c/c/forum/msign`) is **JSON**, not
protobuf (handled by `OfficialTiebaApi`, not `OfficialProtobufTiebaApi`).

**`BazhuSign`** (package `tieba`) — embedded in `User.bazhu_grade` (field 105).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `string desc` |
| 2 | LD | | `string level` |

**`SignInfo`** (package `tieba.frsPage`) — embedded in `FrsPage.ForumInfo.sign_in_info` (field 15).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | LD | | `SignUser user_info` |
| 2 | LD | | `SignForum forum_info` |

**`SignUser`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int64 user_id` |
| 2 | varint | | `int32 is_sign_in` |
| 3 | varint | | `int32 user_sign_rank` |
| 4 | varint | | `int32 sign_time` |
| 5 | varint | | `int32 cont_sign_num` |
| 6 | varint | | `int32 cout_total_sign_num` |
| 7 | varint | | `int32 is_org_disabled` |
| 8 | varint | | `int32 c_sign_num` |
| 9 | varint | | `int32 hun_sign_num` |
| 10 | varint | | `int32 total_resign_num` |
| 11 | varint | | `int32 miss_sign_num` |

**`SignForum`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 is_on` |
| 2 | varint | | `int32 is_filter` |
| 3 | LD | | `Forum forum_info` (frsPage, optional) |
| 4 | LD | | `RankInfo current_rank_info` |

**`RankInfo`** (package `tieba.frsPage`).

| # | Wire | Rep | Type |
|---|---|---|---|
| 1 | varint | | `int32 sign_count` |
| 2 | varint | | `int32 sign_rank` |
| 3 | varint | | `int32 member_count` |
| 4 | 64 | | `double dir_rate` |

---

## 14. 通知 feeds (replyme / atme / agreeme / sys)

**Not protobuf.** There are no `ReplyMe.proto`, `AtMe.proto`, `AgreeMe.proto`, or
`SystemMsg.proto` files in the source tree, and these endpoints are declared in
`NewTiebaApi.kt` with `@FormUrlEncoded` + `@Field`, returning JSON models.

- `POST /c/u/feed/replyme` (field `pn`) → JSON `MessageListBean`
- `POST /c/u/feed/atme`    (field `pn`) → JSON `MessageListBean`
- `POST /c/u/feed/agreeme` (field `pn`) → JSON `MessageListBean`
- `POST /c/s/msg`          (field `bookmark`) → JSON `MsgBean` (message center
  counts: `replyme`, `atme`, `fans`)

`MessageListBean` JSON shape (for reference): `error_code`, `time`, `reply_list`
/ `at_list` (array of `MessageInfoBean`), `page` (`current_page`, `has_more`,
`has_prev`), `message` (`replyme`, `atme`, `fans`, `recycle`, `storethread`).
`MessageInfoBean`: `is_floor`, `title`, `content`, `quote_content`, `replyer`,
`quote_user`, `thread_id`, `post_id`, `time`, `fname`, `quote_pid`,
`thread_type`, `unread`.

---

## 15. Message / field-count summary

Total distinct messages with a field table in this document: **150**.

Key wire-format reminders for the C++ codec:

1. **Field key** = `(field_number << 3) | wire_type`; read as varint.
2. **Packed repeated numerics** (`repeated int64/int32/uint64/uint32/bool/double`)
   are length-delimited (wire type 2) and must be iterated inside the
   length-delimited payload, not read as repeated standalone fields. Relevant
   examples: `FrsPageResponseData.thread_id_list` (field 8, int64 packed),
   `Zan.liker_id` (field 5, int64 packed).
3. **`double`** fields are fixed 64-bit (wire type 1), little-endian
   (e.g. `CommonRequest.scr_dip`, `RankInfo.dir_rate`, `Item.score/icon_size`).
4. **`bool`** is varint (wire type 0).
5. Unknown fields must be **skipped** (read their wire type and discard) so the
   decoder stays forward/backward compatible with the many rarely-used fields
   listed here.
6. The request envelope is `*Request{ data=1 }`; the response envelope is
   `*Response{ error=1, data=2 }` except `GetBawuInfoResponse{ data=1, error=2 }`.
