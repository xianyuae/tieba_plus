# TiebaLite — API Endpoint & Model Reference (for Qt/C++ port)

Source: `app/src/main/java/com/huanchengfly/tieba/post/` (Kotlin reference implementation).
Generated for a Qt/C++ port. Field names below are the **exact wire names** (`@SerialName` / `@SerializedName` / `@Field` / `@Query` names). Kotlin property names are shown in `camelCase` after the wire name where they differ.

---

## 1. Base URLs & API clients

| Client (Retrofit singleton) | Base URL host | Default User-Agent | Notes |
|---|---|---|---|
| `NEW_TIEBA_API` | `http://c.tieba.baidu.com/` | `bdtb for Android 8.2.2` | legacy JSON, messages/feeds/store |
| `MINI_TIEBA_API` | `http://c.tieba.baidu.com/` | `bdtb for Android 7.2.0.0` | "mini" client JSON |
| `OFFICIAL_TIEBA_API` | `http://c.tieba.baidu.com/` | `bdtb for Android 12.25.1.0` | modern JSON client |
| `WEB_TIEBA_API` | `https://tieba.baidu.com/` | `tieba/11.10.8.6 skin/default` | mobile-web (H5) JSON |
| `HYBRID_TIEBA_API` | `https://tieba.baidu.com/` | `tieba/12.35.1.0 skin/default` | H5 hybrid JSON |
| `OFFICIAL_PROTOBUF_TIEBA_API` (V11) | `https://tiebac.baidu.com/` | `bdtb for Android 11.10.8.6` | protobuf, `x_bd_data_type: protobuf` |
| `OFFICIAL_PROTOBUF_TIEBA_V12_API` | `https://tiebac.baidu.com/` | `tieba/12.52.1.0` | protobuf, `x_bd_data_type: protobuf` |
| `OFFICIAL_PROTOBUF_TIEBA_POST_API` | `https://tiebac.baidu.com/` | `tieba/12.35.1.0` | protobuf, used for add-post only |
| `SOFIRE_API` | `https://sofire.baidu.com/` | (`x6/…` set per request) | encrypted Sofire transport |
| Update check | `https://api.github.com/` | — | GitHub releases API |

Client version constants (`Enums.kt`): `TIEBA_V11 = "11.10.8.6"`, `TIEBA_V12 = "12.52.1.0"`, `TIEBA_V12_POST = "12.35.1.0"`.

---

## 2. Common (auto-injected) parameters & headers

These are **not** in the function signatures; they are added by OkHttp interceptors (`CommonParamInterceptor`, `CommonHeaderInterceptor`, `StParamInterceptor`, `SortAndSignInterceptor`) per client.

### 2.1 `defaultCommonParamInterceptor` (applied to NEW / MINI / OFFICIAL JSON + protobuf V11 + protobuf POST)

| Param (wire name) | Value |
|---|---|
| `BDUSS` | `AccountUtil.getBduss()` |
| `_client_id` | `ClientUtils.clientId` (`wappc_<initTime>_<rand>`) |
| `_client_type` | `"2"` |
| `_os_version` | `Build.VERSION.SDK_INT` |
| `model` | `Build.MODEL` |
| `net_type` | `"1"` |
| `_phone_imei` | `MobileInfoUtil.getIMEI()` |
| `timestamp` | `System.currentTimeMillis()` |

### 2.2 Per-client additions

**NEW_TIEBA_API** adds: `cuid`, `from=baidu_appstore`, `_client_version=8.2.2`. Header: `CUID`.
**MINI_TIEBA_API** adds: `cuid`, `cuid_galaxy2`, `from=1021636m`, `_client_version=7.2.0.0`, `subapp_type=mini`.
**OFFICIAL_TIEBA_API** adds (large set): `active_timestamp`, `android_id`, `baiduid`, `brand`, `cmode=1`, `cuid`, `cuid_galaxy2`, `cuid_gid`, `event_day=yyyyMdd`, `extra`, `first_install_time`, `framework_ver=3340042`, `from=tieba`, `is_teenager=0`, `last_update_time`, `mac=02:00:00:00:00:00`, `sample_id`, `sdk_ver=2.34.0`, `start_scheme`, `start_type=1`, `swan_game_ver=1038000`, `_client_version=12.25.1.0`, `c3_aid`, `oaid`.
**OFFICIAL_PROTOBUF_TIEBA_API (V11)** adds: `cuid`, `cuid_galaxy2`, `cuid_gid`, `from=tieba`, `_client_version=11.10.8.6`, `c3_aid`, `oaid` (minus `_os_version`).
**OFFICIAL_PROTOBUF_TIEBA_POST_API** adds: `_client_version=12.35.1.0`, `active_timestamp`, `android_id`, `baiduid`, `brand`, `c3_aid`, `cmode=1`, `cuid`, `cuid_galaxy2`, `cuid_gid`, `device_score`, `event_day`, `extra`, `first_install_time`, `framework_ver=3340042`, `from=tieba`, `is_teenager=0`, `last_update_time`, `mac=02:00:00:00:00:00`, `naws_game_ver=1038000`, `oaid`, `personalized_rec_switch=1`, `sample_id`, `sdk_ver=2.34.0`, `start_scheme`, `start_type=1`, `stoken`, `z_id` (minus `_os_version`).
**OFFICIAL_PROTOBUF_TIEBA_V12_API** adds **no** common params (only the `st` anti-bot params).

### 2.3 `StParamInterceptor` (anti-bot `st*` params)

Added to every request (FormBody/query) unless header `no_st_params: true` is set. A random `num` in `[100,850)` is drawn; if `num` **not** in `[100,120]`, the following are added, otherwise they are omitted:

```
stErrorNums = "1"
stMethod    = "1"  (POST) / "2" (GET)
stMode      = "1"
stTimesNum  = "1"
stTime      = <num>
stSize      = round((rand*8 + 0.4) * num)
```

### 2.4 `SortAndSignInterceptor` (MD5 request signing)

Signing secret: `"tiebaclient!!!"`.
- **Query** (`BDUSS` present in query and no `sign`): sort query params alphabetically, concatenate raw `name=value` pairs, `sign = MD5(concat + secret)`; appended to the (re-sorted) query.
- **FormBody** (contains `_client_version`, no `sign`): `sign = MD5(sortedRaw(body) + secret)` where `sortedRaw` is sorted `name=value` pairs concatenated (no separators).
- **MyMultipartBody** (contains `_client_version`, no `sign`): non-file parts sorted by name, `sortedRaw` = concat of `name=value`, `sign = MD5(sortedRaw + secret)`, appended as a form part before the file parts.
- Otherwise: no signing (protobuf V12 requests have no `_client_version` form field ⇒ unsigned).

### 2.5 Custom interceptor headers (used via `@Headers`)

| Header key | Values |
|---|---|
| `force_login` | `true` → ForceLoginInterceptor throws `TiebaNotLoggedInException` when not logged in |
| `no_common_params` | comma list of common param names to drop (e.g. `BDUSS`, `oaid`, `stoken`) |
| `drop_headers` | comma list of auto headers to drop (`Charset`, `client_type`) |
| `drop_params` | comma list of common params to drop |
| `no_st_params` | `true` → skip `st*` anti-bot params |
| `add_cookie` | `false` → skip AddWebCookieInterceptor |
| `Host` / `Origin` / `Referer` / `cookie` | explicit override |

---

## 3. Protobuf request body (`ProtobufRequest.kt`)

### 3.1 Multipart envelope — `buildProtobufRequestBody(data, clientVersion, needSToken)`

- Content type: `multipart/form-data`; boundary `--------7da3d81520810*`.
- Form fields added **in order**:
  1. `_client_version` = `clientVersion.version` — **only if** `clientVersion` is neither `TIEBA_V12` nor `TIEBA_V12_POST`.
  2. `stoken` = `AccountUtil.getSToken()` — **only if** `needSToken` is true and a token exists.
  3. File part `data` with `filename="file"`, body = `data.encode()` (Square-Wire protobuf bytes of the request message).
- For V11/V11-POST clients the CommonParamInterceptor additionally injects the common params (see §2.2) into the multipart form, then the signer adds `sign`.
- For the V12 client, the multipart body is only `stoken` (when applicable) + `data` (no `_client_version`, no `sign`); the `cmd` is carried in the **query string** of the URL (e.g. `?cmd=301001`).

### 3.2 `CommonRequest` message (inside every `*RequestData.common`)

Message `CommonRequest` (`protos/CommonRequest.proto`). Field population depends on `clientVersion` (see `buildCommonRequest` in `ProtobufRequest.kt`). Common/stable fields:

| Field | V11 | V12 / V12_POST |
|---|---|---|
| `BDUSS` | `AccountUtil.getBduss()` | same |
| `_client_id` | `ClientUtils.clientId \|\| randomClientId` | same |
| `_client_type` | `2` | `2` |
| `_client_version` | `11.10.8.6` | `12.52.1.0` / `12.35.1.0` |
| `_os_version` | `Build.VERSION.SDK_INT` | same |
| `_phone_imei` | `MobileInfoUtil.getIMEI()` | same |
| `_timestamp` | `System.currentTimeMillis()` | same |
| `brand` | `Build.BRAND` | same |
| `c3_aid` | `UIDUtil.getAid()` | same |
| `cuid` / `cuid_galaxy2` | `CuidUtils.getNewCuid()` | same |
| `cuid_gid` | `""` | same |
| `from` | `1024324o` | `1020031h` |
| `is_teenager` | `0` | `0` |
| `lego_lib_version` | `3.0.0` | `3.0.0` |
| `model` | `Build.MODEL` | same |
| `net_type` | `1` | `1` |
| `oaid` | `OAID().toJson()` | `""` (V12) / `App.Config.encodedOAID` (V12_POST) |
| `pversion` | `1.0.3` | `1.0.3` |
| `sample_id` | `ClientUtils.sampleId` | same |
| `stoken` | `AccountUtil.getSToken()` | same |
| `android_id` | — | `base64(UIDUtil.getAndroidId("000"))` |
| `active_timestamp` | — | `ClientUtils.activeTimestamp` |
| `cmode` | — | `1` |
| `event_day` | — | `yyyyMdd` |
| `first_install_time` / `last_update_time` | — | `App.Config.*` |
| `framework_ver` | — | `3340042` |
| `scr_dip` / `scr_h` / `scr_w` | — | screen metrics (Double) |
| `sdk_ver` | — | `2.34.0` |
| `swan_game_ver` | — | `1038000` |
| `user_agent` | — | `getUserAgent("tieba/<ver>")` |
| `z_id` | — | account zid |
| `tbs` | — | only V12_POST |
| `personalized_rec_switch` | — | `1` (V12) / `1` (V12_POST) |

### 3.3 Protobuf response envelope

Every `*Response` is a Wire message of the form `{ data: *ResponseData, error: Error }` (the `Error` message is defined in `protos/Error.proto`, fields `errno`, `errmsg`, `usermsg`). The Qt port must decode the outer message, then its `data` sub-message.

---

## 4. Endpoint catalog

Legend for param "kind": **B** = business param, **C** = common/auth/device param (explicitly declared in the signature; interceptor-injected common params are in §2 and not repeated).

---

### A. Home feed (个性推荐 / personalized)

#### A1. `personalizedFlow` (OFFICIAL JSON) — `POST /c/f/excellent/personalized` (c.tieba.baidu.com)
Params (all `@Field`, FormUrlEncoded):
| name | default | kind | meaning |
|---|---|---|---|
| `load_type` | — | B | 1=pull-refresh, 2=load-more |
| `pn` | `1` | B | page |
| `_client_version` | `11.10.8.6` | C | client version |
| `cuid_gid` | `""` | C | |
| `need_tags` | `0` | B | |
| `page_thread_count` | `15` | B | |
| `pre_ad_thread_count` | `0` | B | |
| `sug_count` | `0` | B | |
| `tag_code` | `0` | B | |
| `q_type` | `1` | B | |
| `need_forumlist` | `0` | B | |
| `new_net_type` | `1` | C | |
| `new_install` | `0` | C | |
| `request_time` | `now` | C | |
| `invoke_source` | `""` | C | |
| `scr_dip` / `scr_h` / `scr_w` | density/height/width | C | screen |
Headers: `client_user_token` = uid; `User-Agent` = `bdtb for Android 11.10.8.6`.
Response: `PersonalizedBean`.

#### A2. `personalizedProtoFlow` — `POST /c/f/excellent/personalized?cmd=309264` (tiebac.baidu.com)
Body: `PersonalizedRequest { PersonalizedRequestData { app_pos=AppPosInfo, common=CommonRequest(V12), load_type, pn, need_tags=0, page_thread_count=11, pre_ad_thread_count=0, sug_count=0, tag_code=0, q_type=1, need_forumlist=0, new_net_type=1, new_install=0, request_times=0, invoke_source="", scr_dip, scr_h, scr_w } }`, `clientVersion=TIEBA_V12`.
Response: `PersonalizedResponse` (cmd 309264).

---

### B. Forum list (关注吧列表)

#### B1. `forumRecommend` / `forumRecommendFlow` (MINI) — `POST /c/f/forum/forumrecommend` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. All `@Field`:
| name | default | kind |
|---|---|---|
| `like_forum` | `"1"` | B |
| `recommend` | `"0"` | B |
| `topic` | `"0"` | B |
Response: `ForumRecommend`.

#### B2. `forumRecommendNewFlow` — `POST /c/f/forum/forumrecommend?cmd=303011` (tiebac.baidu.com)
Body: `ForumRecommendRequest { ForumRecommendRequestData { common, like_forum=1, recommend=1, sort_type=sortType, topic=0 } }`, V11.
Response: `ForumRecommendResponse` (cmd 303011).

#### B3. `getForumListFlow` (OFFICIAL JSON) — `POST /c/f/forum/getforumlist` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: BDUSS`. All `@Field`:
| name | default | kind |
|---|---|---|
| `BDUSS` | `AccountUtil.getBduss()` | C |
| `stoken` | `AccountUtil.getSToken()` | C |
| `user_id` | `AccountUtil.getUid()` | C |
| `_client_version` | `11.10.8.6` | C |
Headers: `User-Agent` = `bdtb for Android 11.10.8.6`.
Response: `GetForumListBean`.

#### B4. `forumHomeAsync` (WEB) — `GET /mg/o/getForumHome` (tieba.baidu.com)
`FORCE_LOGIN: true`, `Referer: https://tieba.baidu.com/index/tbwise/forum?source=index`, plus `sec-ch-ua*` headers.
`@Query`:
| name | default | kind |
|---|---|---|
| `st` | sortType | B | 0=by-level, 1=by-follow order |
| `pn` | page | B | 0-based |
| `rn` | `20` | B | page size |
| `eqid` | `""` | C | |
| `refer` | `""` | C | |
Response: `ForumHome` (web).

---

### C. Forum page (frs 吧页面)

#### C1. `forumPage` / `forumPageAsync` (MINI) — `POST /c/f/frs/page` (c.tieba.baidu.com)
All `@Field`:
| name | default | kind | meaning |
|---|---|---|---|
| `kw` | — | B | forum name (URL-encoded) |
| `pn` | `1` | B | page |
| `sort_type` | — | B | `ForumSortType` (0=reply time,1=send time,2=only-followed) |
| `cid` | `null` | B | good-classify id |
| `is_good` | `"1"` iff cid non-empty else `null` | B | |
| `q_type` | `"2"` | B | |
| `st_type` | `"tb_forumlist"` | B | |
| `with_group` | `"0"` | B | |
| `rn` | `"20"` | B | page size |
| `scr_dip` / `scr_h` / `scr_w` | density/height/width | C | |
Response: `ForumPageBean`.

#### C2. `frsPage` — `POST /c/f/frs/page?cmd=301001` (tiebac.baidu.com)
Header: `forum_name` = `forumName.urlEncode()`.
Body: `FrsPageRequest { FrsPageRequestData { ad_param=AdParam(load_count=0,refresh_count=4,yoga_lib_version="1.0"), app_pos=AppPosInfo, call_from=0, category_id=0, cid=goodClassifyId?:0, common=CommonRequest(V12), ctime=0, data_size=0, hot_thread_id=0, is_default_navtab=0, is_good=1iff-good, is_selection=0, kw=forumName.urlEncode(), last_click_tid=0, load_type, net_error=0, pn=page, q_type=2, rn=90, rn_need=30, scr_dip, scr_h, scr_w, sort_type, st_param=0, st_type="recom_flist", up_schema="", with_group=1, yuelaou_locate="" } }`, V12.
Response: `FrsPageResponse` (cmd 301001).

#### C3. `threadList` — `POST /c/f/frs/threadlist?cmd=301002` (tiebac.baidu.com)
Body: `ThreadListRequest { ThreadListRequestData { ad_param=AdParam(3,0,null), app_pos=AppPosInfo, common=CommonRequest(V12), scr_dip, scr_h, scr_w, forum_id, forum_name, pn, q_type=2, user_id=uid.toLongOrNull(), thread_ids, sort_type, need_abstract=0, st_type=0, last_click_tid=0 } }`, V12.
Response: `ThreadListResponse` (cmd 301002).

---

### D. Thread page (pb page 贴子页)

#### D1. `threadContent` / `threadContentAsync` (OFFICIAL JSON) — `POST /c/f/pb/page` (c.tieba.baidu.com)
Two overloads (by `pn` or by `pid`). Header: `thread_id` = threadId.
All `@Field`:
| name | default | kind | meaning |
|---|---|---|---|
| `kz` | — | B | thread id |
| `pn` | — (alt: `pid`) | B | page / post id |
| `last` | `null` | B | `"1"` when reverse |
| `r` | `null` | B | `"1"` when reverse |
| `lz` | — | B | 1=see-lz (只看楼主), 0=no |
| `st_type` | `"tb_frslist"` | B | |
| `back` | `"0"` | B | |
| `floor_rn` | `"3"` | B | |
| `mark` | `"0"` | B | collect state |
| `rn` | `"30"` | B | page size |
| `with_floor` | `"1"` | B | |
| `scr_dip` / `scr_h` / `scr_w` | density/height/width | C | |
Response: `ThreadContentBean`.

#### D2. `pbPageFlow` — `POST /c/f/pb/page?cmd=302001&format=protobuf` (tiebac.baidu.com)
Body: `PbPageRequest { PbPageRequestData { common=CommonRequest(V12), kz=threadId, pid=postId, pn=page, r=sortType, lz=seeLz?1:0, forum_id=forumId?:0, ad_param=AdParam(load_count=0,refresh_count=1,is_req_ad=1), mark, last_pid=lastPostId?:0, app_pos=AppPosInfo, back=back?1:0, banner=0, broadcast_id=0, floor_rn=4, floor_sort_type=1, from_push=0, from_smart_frs=0, immersion_video_comment_source=0, is_comm_reverse=0, is_fold_comment_req=0, is_jumpfloor=0, jumpfloor_num=0, need_repost_recommend_forum=0, obj_locate="", obj_param1="10", obj_source="", ori_ugc_type=0, pb_rn=0, q_type=2, request_times=0, rn=15, s_model=0, scr_dip, scr_h, scr_w, similar_from=0, source_type=2, st_type, thread_type=0, weipost=0, with_floor=1 } }`, V12.
Response: `PbPageResponse` (cmd 302001).

---

### E. Sub floor (楼中楼)

#### E1. `floor` (OFFICIAL JSON) — `POST /c/f/pb/floor` (c.tieba.baidu.com)
All `@Field`:
| name | default | kind | meaning |
|---|---|---|---|
| `kz` | — | B | thread id |
| `pn` | `1` | B | page |
| `pid` | — | B | post id |
| `spid` | — | B | sub-post id |
| `rn` | `20` | B | page size |
Response: `SubFloorListBean`.

#### E2. `pbFloorFlow` — `POST /c/f/pb/floor?cmd=302002&format=protobuf` (tiebac.baidu.com)
Body: `PbFloorRequest { PbFloorRequestData { common=CommonRequest(V12), forum_id, kz=threadId, pid=postId, pn=page, spid=subPostId, scr_dip, scr_h, scr_w, is_comm_reverse=0, ori_ugc_type=0 } }`, V12, `needSToken=false`.
Response: `PbFloorResponse` (cmd 302002).

---

### F. Post / reply / add post

#### F1. `addPostFlow` (OFFICIAL JSON) — `POST /c/c/post/add` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. All `@Field`:
| name | default | kind | meaning |
|---|---|---|---|
| `content` | — | B | post text |
| `fid` | — | B | forum id |
| `kw` | — | B | forum name |
| `tbs` | — | C | anti-CSRF token |
| `tid` | — | B | thread id |
| `quote_id` | `null` | B | quoted post id |
| `repostid` | `null` | B | repost id |
| `reply_uid` | `"null"` | B | replied user id |
| `name_show` | `AccountUtil.getLoginInfo()?.nameShow` | C | |
| `anonymous` | `"1"` | B | |
| `authsid` | `"null"` | C | |
| `barrage_time` | `"0"` | B | |
| `can_no_forum` | `"0"` | B | |
| `entrance_type` | `"0"` | B | |
| `from_fourm_id` | `"null"` | B | |
| `is_ad` | `"0"` | B | |
| `is_addition` | `null` | B | |
| `is_barrage` | `"0"` | B | |
| `is_feedback` | `"0"` | B | |
| `is_giftpost` | `null` | B | |
| `is_twzhibo_thread` | `null` | B | |
| `new_vcode` | `"1"` | B | |
| `post_from` | `"3"` | B | |
| `takephoto_num` | `"0"` | B | |
| `v_fid` | `""` | B | |
| `v_fname` | `""` | B | |
| `vcode_tag` | `"12"` | B | |
| `_client_version` | `11.10.8.6` | C | |
| `stoken` | `AccountUtil.getSToken()` | C | |
Headers: `client_user_token` = uid, `User-Agent` = `bdtb for Android 11.10.8.6`.
Response: `AddPostBean`.

#### F2. `addPostFlow` — `POST /c/c/post/add?cmd=309731&format=protobuf` (tiebac.baidu.com)
Body: `AddPostRequest { AddPostRequestData { anonymous="1", barrage_time="0" iff no postId, can_no_forum="0", common=CommonRequest(V12_POST, tbs), content, entrance_type="0", fid=forumId, floor_num="0", kw=forumName, is_ad="0", is_addition="0", is_barrage="0", is_feedback="0", is_giftpost="0", is_pictxt="0", is_show_bless=0, is_twzhibo_thread="0", name_show, new_vcode="1", post_from=("13" if neither postId nor subPostId; "0" if only postId; null if subPostId), quote_id=postId, reply_uid=replyUserId if postId present, repostid=postId, sub_post_id=subPostId, show_custom_figure=0, takephoto_num="0", tid=threadId, v_fid="" iff no postId, v_fname="" iff no postId, vcode_tag="12" } }`, `clientVersion=TIEBA_V12_POST`.
Response: `AddPostResponse` (cmd 309731).

#### F3. `webReply` / `webReplyAsync` (WEB) — `POST /mo/q/apubpost` (tieba.baidu.com)
Headers: `Host: tieba.baidu.com`, `Origin: https://tieba.baidu.com`, `X-Requested-With: XMLHttpRequest`.
`@Query`:
| name | default | kind |
|---|---|---|
| `_t` | `System.currentTimeMillis()` | C |
`@Field`:
| name | default | kind | meaning |
|---|---|---|---|
| `co` | — | B | content |
| `_t` | `now` | C | |
| `tag` | `"11"` | B | |
| `upload_img_info` | — | B | image info |
| `fid` | — | B | forum id |
| `src` | `"1"` | B | |
| `word` | — | B | forum name |
| `tbs` | — | C | |
| `z` | — | B | thread id |
| `lp` | `"6026"` | B | |
| `nick_name` | — | B | |
| `pid` | `null` | B | reply post id |
| `lzl_id` | `null` | B | floor-sub reply id |
| `floor` | `null` | B | floor number |
| `_BSK` | — | C | |
Header: `Referer` = `https://tieba.baidu.com/p/<tid>?lp=5028&mo_device=1&is_jingpost=0&pn=<pn>&`.
Response: `WebReplyResultBean`.

---

### G. Image upload

#### G1. `uploadPicture` (OFFICIAL JSON) — `POST /c/s/uploadPicture` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: swan_game_ver,sdk_ver`. Body: raw `RequestBody` (binary), not form-encoded.
Header: `cookie` = `ka=open;BAIDUID=<baiduId>` (or `ka=open`).
Response: `UploadPictureResultBean`.

#### G2. `webUploadPic` (WEB) — `POST /mo/q/cooluploadpic` (tieba.baidu.com)
`FORCE_LOGIN: true`. `@Query`: `type=ajax` (default `"ajax"`), `r=Math.random()`. `@Field`: `pic` = base64 image string.
Response: `WebUploadPicBean`.

---

### H. Search

#### H1. `searchPost` / `searchPostAsync` (MINI) — `POST /c/s/searchpost` (c.tieba.baidu.com)
All `@Field`:
| name | default | kind | meaning |
|---|---|---|---|
| `word` | — | B | keyword |
| `kw` | — | B | forum name |
| `pn` | `1` | B | page |
| `rn` | `30` | B | page size |
| `only_thread` | `0` | B | 1=threads only |
| `sm` | `1` | B | sort mode (1=time desc, 2=relevance) |
Response: `SearchPostBean`.

#### H2. `searchForum` (WEB) — `GET /mo/q/search/forum` (tieba.baidu.com)
`@Query`: `word` = keyword. Response: `SearchForumBean`.

#### H3. `searchForumFlow` (HYBRID) — `GET /mo/q/search/forum` (tieba.baidu.com)
`no_st_params: true`, `no_common_params: BDUSS,STOKEN`. `@Query`: `word`. Header `Referer` = urlencoded `https://tieba.baidu.com/mo/q/hybrid/search?keyword=<kw>&_webview_time=<now>`.
Response: `SearchForumBean`.

#### H4. `searchThread` (WEB) — `GET /mo/q/search/thread` (tieba.baidu.com)
`@Query`:
| name | default | kind | meaning |
|---|---|---|---|
| `word` | — | B | |
| `pn` | — | B | page |
| `st` | — | B | order (SearchThreadOrder: 1=new,0=old,2=relevant) |
| `tt` | — | B | filter (SearchThreadFilter: 1=only-thread,2=all) |
| `ct` | `"2"` | B | |
Response: `SearchThreadBean`.

#### H5. `searchThreadFlow` (HYBRID) — `GET /mo/q/search/thread` (tieba.baidu.com)
`no_st_params: true`, `no_common_params: BDUSS,STOKEN`. `@Query`:
| name | default | kind | meaning |
|---|---|---|---|
| `word` | — | B | |
| `pn` | — | B | page |
| `st` | — | B | sort (0=old,2=relevant,5=new) |
| `tt` | `1` | B | filter |
| `rn` | `null` | B | page size |
| `fname` | `null` | B | forum name (forum-scoped search) |
| `ct` | `1` (2 for forum search) | B | |
| `is_use_zonghe` | `1` (null for forum search) | B | |
| `cv` | `99.9.101` (12.35.1.0 for forum search) | C | client version |
Header `Referer`: hybrid search URL (or `hybrid-usergrow-search/searchGlobal?...&forumName=...&forumId=...` for forum search).
Response: `SearchThreadBean`.

#### H6. `searchUser` (MINI) — `GET /mo/q/search/user` (c.tieba.baidu.com)
`@Query`: `word`, `_client_version=8.0.8.0`, `cuid_gid=""`. Header: `client_user_token`=uid, `User-Agent`=`bdtb for Android 8.0.8.0`.
Response: `SearchUserBean`.

#### H7. `searchUserFlow` (HYBRID) — `GET /mo/q/search/user` (tieba.baidu.com)
`no_st_params: true`, `no_common_params: BDUSS,STOKEN`. `@Query`: `word`. Header `Referer` = hybrid search URL.
Response: `SearchUserBean`.

#### H8. `searchSuggestionsFlow` — `POST /c/s/searchSug?cmd=309438&format=protobuf` (tiebac.baidu.com)
Body: `SearchSugRequest { SearchSugRequestData { common=CommonRequest(V12), word, isforum=isForum.booleanToString() } }`, V12, `needSToken=true`.
Response: `SearchSugResponse` (cmd 309438).

---

### I. Sign-in (single + one-key)

#### I1. `signAsync` / `signFlow` (MINI) — `POST /c/c/forum/sign` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `kw` = forumName, `tbs` = tbs. (No `fid`.)
Response: `SignResultBean`.

#### I2. `signFlow` (OFFICIAL JSON) — `POST /c/c/forum/sign` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. `@Field`:
| name | default | kind |
|---|---|---|
| `fid` | — | B |
| `kw` | — | B |
| `tbs` | — | C |
| `_client_version` | `11.10.8.6` | C |
Headers: `client_user_token`=uid, `User-Agent`=`bdtb for Android 11.10.8.6`.
Response: `SignResultBean`.

#### I3. `mSign` (OFFICIAL JSON) — `POST /c/c/forum/msign` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`:
| name | default | kind |
|---|---|---|
| `forum_ids` | — | B | comma-separated fids |
| `tbs` | — | C |
| `_client_version` | `11.10.8.6` | C |
| `authsid` | `"null"` | C |
| `stoken` | `AccountUtil.getSToken()` | C |
| `user_id` | `AccountUtil.getUid()` | C |
Headers: `User-Agent`=`bdtb for Android 11.10.8.6`.
Response: `MSignBean`.

#### I4. `oneKeySignIn` (MINI) — `POST http://tieba.baidu.com/tbmall/onekeySignin1`
`NO_COMMON_PARAMS: BDUSS` (full URL override via `@Url`). `@Field`:
| name | default | kind |
|---|---|---|
| `BDUSS` | `AccountUtil.getBduss()` | C |
| `ie` | `utf-8` | C |
| `tbs` | `AccountUtil.getLoginInfo()!!.tbs` | C |
Response: `OneKeySignInBean`.

---

### J. Like / unlike forum

#### J1. `likeForum` / `likeForumFlow` (MINI) — `POST /c/c/forum/like` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `fid`, `kw`, `tbs`.
Response: `LikeForumResultBean`.

#### J2. `unlikeForum` / `unlikeForumFlow` (MINI) — `POST /c/c/forum/unlike` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `fid`, `kw`, `tbs`.
Response: `CommonResponse`.

#### J3. `unfavolike` (OFFICIAL JSON) — `POST /c/c/forum/unfavolike` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. `@Field`: `fid`, `kw`, `tbs`, `_client_version=11.10.8.6`, `stoken`. Headers: `client_user_token`=uid, `User-Agent`=`bdtb for Android 11.10.8.6`.
Response: `CommonResponse`.

---

### K. Agree (点赞/点踩)

#### K1. `agree` / `opAgreeFlow` / `disagree` / `disagreeFlow` (MINI) — `POST /c/c/agree/opAgree` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`:
| name | agree/opAgree default | disagree default | kind | meaning |
|---|---|---|---|---|
| `post_id` | — | — | B | |
| `thread_id` | — | — | B | |
| `_client_version` | `8.0.8.0` | `8.0.8.0` | C | |
| `cuid_gid` | `""` | `""` | C | |
| `agree_type` | `2` | `2` / `5` | B | 2=agree, 5=disagree |
| `obj_type` | `3` | `3` | B | |
| `op_type` | `0` | `1` | B | 0=apply, 1=cancel |
| `tbs` | `AccountUtil.getLoginInfo()?.tbs` | same | C | |
| `stoken` | `AccountUtil.getSToken()` | same | C | |
Header: `client_user_token`=uid, `User-Agent`=`bdtb for Android 8.0.8.0`.
Response: `AgreeBean`.

#### K2. `agreeFlow` (OFFICIAL JSON) — `POST /c/c/agree/opAgree` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. `@Field`:
| name | default | kind |
|---|---|---|
| `thread_id` | — | B |
| `post_id` | `null` | B |
| `op_type` | `0` | B |
| `obj_type` | `1` | B |
| `agree_type` | `2` | B |
| `cuid_gid` | `""` | C |
| `forum_id` | `""` | B |
| `personalized_rec_switch` | `1` | C |
| `tbs` | `AccountUtil.getLoginInfo()!!.tbs` | C |
| `stoken` | `AccountUtil.getSToken()` | C |
Header: `client_user_token`=uid.
Response: `AgreeBean`.

---

### L. Thread store (收藏贴)

#### L1. `threadStore` (NEW) — `POST /c/f/post/threadstore` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `rn`=pageSize, `offset`=`pageSize*page`, `user_id`=uid.
Response: `ThreadStoreBean`.

#### L2. `threadStoreFlow` (OFFICIAL JSON) — `POST /c/f/post/threadstore` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. `@Field`: `rn`=pageSize, `offset`=`pageSize*page`, `_client_version=11.10.8.6`, `stoken`, `user_id`=uid. Headers: `client_user_token`=uid, `User-Agent`=`bdtb for Android 11.10.8.6`.
Response: `ThreadStoreBean`.

#### L3. `removeStore` (NEW) — `POST /c/c/post/rmstore` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `tid`=threadId, `tbs`=tbs.
Response: `CommonResponse`.

#### L4. `removeStoreFlow` (OFFICIAL JSON) — `POST /c/c/post/rmstore` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. `@Field`: `tid`, `fid="null"`, `tbs=AccountUtil.getLoginInfo()!!.tbs`, `stoken`, `user_id`=uid. Header: `client_user_token`=uid.
Response: `CommonResponse`.

#### L5. `addStore` (NEW) — `POST /c/c/post/addstore` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `data` = JSON array `[CollectDataBean{threadId,postId,"0","0"}]`, `tbs`.
Response: `CommonResponse`.

#### L6. `addStoreAsync` / `addStoreFlow` (OFFICIAL JSON) — `POST /c/c/post/addstore` (c.tieba.baidu.com)
`FORCE_LOGIN: true`, `NO_COMMON_PARAMS: oaid`. `@Field`: `data` = JSON array `[NewCollectDataBean{threadId,postId,status=1}]`, `stoken`. Header: `client_user_token`=uid.
Response: `CommonResponse`.

---

### M. User profile / user post / getuserinfo

#### M1. `profile` / `profileFlow` (MINI) — `POST /c/u/user/profile` (c.tieba.baidu.com)
`@Field`: `uid`, `need_post_count=1`.
Response: `ProfileBean`.

#### M2. `profileFlow` (OFFICIAL JSON) — `POST /c/u/user/profile` (c.tieba.baidu.com)
`@Field`:
| name | default | kind |
|---|---|---|
| `stoken` | `AccountUtil.getSToken()` | C |
| `tbs` | `AccountUtil.getLoginInfo()!!.tbs` | C |
| `uid` | `AccountUtil.getUid()` | B |
| `is_from_usercenter` | `"1"` | B |
| `need_post_count` | `"1"` | B |
| `page` | `"1"` | B |
| `pn` | `"1"` | B |
| `_client_version` | `11.10.8.6` | C |
Headers: `cookie: ka=open`, `User-Agent`=`bdtb for Android 11.10.8.6`.
Response: `Profile`.

#### M3. `userProfileFlow` — `POST /c/u/user/profile?cmd=303012&format=protobuf` (tiebac.baidu.com)
Body: `ProfileRequest { ProfileRequestData { common=CommonRequest(V12), friend_uid=uid.takeIf{!isSelf}, friend_uid_portrait="", has_plist=1, is_from_usercenter=1, is_guest=(isSelf?0:1), need_post_count=1, page=1, pn=1, q_type=0, rn=20, scr_dip, scr_h, scr_w, uid=selfUid } }`, V12.
Response: `ProfileResponse` (cmd 303012).

#### M4. `myProfileAsync` (WEB) — `GET /mg/o/profile` (tieba.baidu.com)
`FORCE_LOGIN: true`, `Referer: https://tieba.baidu.com/index/tbwise/mine?source=index`, `sec-ch-ua*`. `@Query`: `format=json`, `eqid=""`, `refer=""`.
Response: `web.Profile`.

#### M5. `userPost` (MINI) — `POST /c/u/feed/userpost` (c.tieba.baidu.com)
`@Field`: `uid`, `pn=1`, `is_thread` (1/0), `rn=20`, `need_content=1`.
Response: `UserPostBean`.

#### M6. `userPostFlow` — `POST /c/u/feed/userpost?cmd=303002&format=protobuf` (tiebac.baidu.com)
Body: `UserPostRequest { UserPostRequestData { uid, rn=20, is_thread=isThread?1:0, need_content=1, pn=page, common=CommonRequest(V12), scr_w, scr_h, scr_dip, q_type=1, is_view_card=isThread?1:0, subtype=(if isThread 0 else null) } }`, V12, `needSToken=true`.
Response: `UserPostResponse` (cmd 303002).

#### M7. `getUserInfoFlow` — `POST /c/u/user/getuserinfo?cmd=303024&format=protobuf` (tiebac.baidu.com)
Body: `GetUserInfoRequest { GetUserInfoRequestData { common=CommonRequest(V12, bduss, stoken), uid, scr_w } }`, V12, `needSToken=true`.
Response: `GetUserInfoResponse` (cmd 303024).

---

### N. Feeds (replyme / atme / agreeme / sys msg)

#### N1. `replyMe` / `replyMeAsync` / `replyMeFlow` (NEW) — `POST /c/u/feed/replyme` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `pn` (default `0`).
Response: `MessageListBean`.

#### N2. `atMe` / `atMeAsync` / `atMeFlow` (NEW) — `POST /c/u/feed/atme` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `pn` (default `0`).
Response: `MessageListBean`.

#### N3. `agreeMe` (NEW) — `POST /c/u/feed/agreeme` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `pn` (default `0`).
Response: `MessageListBean`.

#### N4. `msg` / `msgFlow` (NEW) — `POST /c/s/msg` (c.tieba.baidu.com)
`FORCE_LOGIN: true`. `@Field`: `bookmark=1`.
Response: `MsgBean`.

---

### O. Forum detail / rule / bawu / level

All are protobuf on `tiebac.baidu.com`, V12, `needSToken=true`, request data = `{ common=CommonRequest(V12), forum_id }`.

| # | Endpoint (fn) | URL | cmd | Request msg | Response msg |
|---|---|---|---|---|---|
| O1 | `getForumDetailFlow` | `POST /c/f/forum/getforumdetail?cmd=303021&format=protobuf` | 303021 | `GetForumDetailRequest{GetForumDetailRequestData{common,forum_id}}` | `GetForumDetailResponse` |
| O2 | `getBawuInfoFlow` | `POST /c/f/forum/getBawuInfo?cmd=301007&format=protobuf` | 301007 | `GetBawuInfoRequest{GetBawuInfoRequestData{common,forum_id}}` | `GetBawuInfoResponse` |
| O3 | `getLevelInfoFlow` | `POST /c/f/forum/getLevelInfo?cmd=301005&format=protobuf` | 301005 | `GetLevelInfoRequest{GetLevelInfoRequestData{common,forum_id}}` | `GetLevelInfoResponse` |
| O4 | `getMemberInfoFlow` | `POST /c/f/forum/getMemberInfo?cmd=301004&format=protobuf` | 301004 | `GetMemberInfoRequest{GetMemberInfoRequestData{common,forum_id}}` | `GetMemberInfoResponse` |
| O5 | `forumRuleDetailFlow` | `POST /c/f/forum/forumRuleDetail?cmd=309690&format=protobuf` | 309690 | `ForumRuleDetailRequest{ForumRuleDetailRequestData{common,forum_id}}` | `ForumRuleDetailResponse` |
| O6 | `getHistoryForumFlow` | `POST /c/f/forum/gethistoryforum?cmd=309601&format=protobuf` | 309601 | `GetHistoryForumRequest{GetHistoryForumRequestData{common,history}}` | `GetHistoryForumResponse` |

---

### P. Hot topic / thread (热榜)

#### P1. `hotThreadListFlow` — `POST /c/f/forum/hotThreadList?cmd=309661` (tiebac.baidu.com)
Body: `HotThreadListRequest { HotThreadListRequestData { common=CommonRequest(V11), tabCode, tabId="1" } }`, V11.
Response: `HotThreadListResponse` (cmd 309661).

#### P2. `topicListFlow` — `POST /c/f/recommend/topicList?cmd=309289` (tiebac.baidu.com)
Body: `TopicListRequest { TopicListRequestData { common, call_from="newbang", list_type="all", need_tab_list="0", fid=0 } }`, V11.
Response: `TopicListResponse` (cmd 309289).

#### P3. `topicDetailFlow` (HYBRID) — `GET /mo/q/newtopic/topicDetail` (tieba.baidu.com)
`@Query`:
| name | default | kind |
|---|---|---|
| `topic_id` | — | B |
| `topic_name` | — | B |
| `is_new` | `0` | B |
| `is_share` | `1` | B |
| `pn` | — | B |
| `rn` | `10` | B |
| `offset` | `0` | B |
| `derivative_to_pic_id` | `""` | B |
Response: `TopicDetailBean`.

#### P4. `hotTopicMain` (WEB) — `GET /mo/q/hotMessage/main` (tieba.baidu.com)
`@Query`: `topic_id`, `yuren_rand`, `topic_name`, `pmy_topic_ext`.
Response: `HotTopicMainBean`.

#### P5. `hotTopicThread` (WEB) — `GET /mo/q/hotMessage/thread` (tieba.baidu.com)
`@Query`: `topic_id`, `yuren_rand`, `topic_name`, `pmy_topic_ext`, `page`, `num=30`, `forum_id=""`.
Response: `HotTopicThreadBean`.
(Also available: `hotTopicForum` → `GET /mo/q/hotMessage/forum`, and `hotTopic` → `GET /mo/q/hotMessage?fr=newwise`.)

#### P6. `hotMessageList` (WEB) — `GET /mo/q/hotMessage/list?fr=newwise` (tieba.baidu.com)
No query params. Response: `HotMessageListBean`.

---

### Q. Update check

#### Q1. GitHub latest release — `GET https://api.github.com/repos/min09577/TiebaLite/releases/latest`
Header: `Accept: application/vnd.github+json`. Response JSON (relevant keys): `tag_name`, `html_url`, `name`, `assets[]` (`name`, `browser_download_url`).
(Not Retrofit — raw `HttpURLConnection`.)

---

## 5. JSON bean field reference (exact wire names)

All fields are nullable `String?` unless a type is noted. Wire name → Kotlin type (property name in parens where different).

### 5.1 `ForumPageBean` (extends `ErrorBean`: `error_code`, `error_msg`)
| wire name | type |
|---|---|
| `forum` | `ForumBean` |
| `anti` | `AntiBean` |
| `user` | `UserBean` |
| `page` | `PageBean` |
| `thread_list` | `List<ThreadBean>` |
| `user_list` | `List<UserBean>` |

`ForumBean`: `id`, `name`, `is_like`, `user_level`, `level_id`, `level_name`, `is_exists`, `cur_score`, `levelup_score`, `member_num`, `thread_num`, `theme_color`(`ThemeColors`), `post_num`, `managers`(`List<ManagerBean>`), `zyqTitle`, `zyqDefine`, `zyqFriend`, `good_classify`(`List<GoodClassifyBean>`), `slogan`, `avatar`, `tids`, `sign_in_info`(`SignInInfo`).
`ThemeColors`: `day`,`dark`,`night` (each `ThemeColor`: `common_color`,`dark_color`,`font_color`,`light_color`).
`GoodClassifyBean`: `class_id`, `class_name`.
`SignInInfo.UserInfo`: `is_sign_in`.
`AntiBean`: `tbs`, `ifpost`, `forbid_flag`, `forbid_info`.
`UserBean`: `id`, `name`, `name_show` (alternate `nick`), `portrait` (PortraitAdapter).
`PageBean`: `page_size`, `offset`, `current_page`, `total_count`, `total_page`, `has_more`, `has_prev`, `cur_good_id`.
`ThreadBean`: `id`, `tid`, `title`, `reply_num`, `view_num`, `last_time`, `last_time_int`, `create_time`, `agree_num`, `is_top`, `is_good`, `is_ntitle`, `author_id`, `video_info`(`VideoInfoBean`), `media`(`List<MediaInfoBean>`), `abstract`(`List<AbstractBean>`).
`AbstractBean`: `type`, `text`.
`MediaInfoBean`: `type`, `show_original_btn`, `is_long_pic`, `is_gif`, `big_pic`, `dynamic_pic`, `src_pic`, `post_id`, `origin_pic`.
`VideoInfoBean`: `video_url`, `thumbnail_url`, `origin_video_url`.

### 5.2 `TopicDetailBean` (kotlinx-serialization)
| wire name | type |
|---|---|
| `no` | Int (errorCode) |
| `error` | String (errorMsg) |
| `data` | `TopicDetailDataBean` |

`TopicDetailDataBean`: `topic_info`(`TopicInfoBean`), `user`(`UserBean`), `tbs`(String), `relateForum`(`List<RelateForumBean>`), `special_topic`(`List<SpecialTopicBean>`), `relate_thread`(`RelateThreadBean`), `has_more`(Boolean).
`RelateThreadBean`: `thread_list`(`List<ThreadBean>`).
`ThreadBean`: `feed_id`(Long), `source`(Int), `thread_info`(`ThreadInfoBean`), `user_agree`(Int).
`TopicInfoBean`: `topic_id`, `topic_name`, `candle`, `topic_desc`, `discuss_num`, `topic_image`, `share_title`, `share_pic`, `is_video_topic`(Int).
`UserBean`: `is_login`(Boolean), `id`(Long), `uid`(Long), `name`, `name_show`, `portrait`(portraitUrl).
`RelateForumBean`: `forum_id`(Long), `forum_name`, `avatar`, `desc`, `member_num`(Long), `thread_num`(Long), `post_num`(Long).
`SpecialTopicBean`: `title`, `thread_list`(`List<ThreadInfoBean>`).
`ThreadInfoBean`: `id`(Long), `feed_id`(Long), `title`, `tid`(Long, threadId), `forum_id`(Long), `forum_name`, `create_time`(Long), `last_time`, `last_time_int`(Long), `abstract`(abstractText), `media`(`List<MediaBean>`), `media_num`(`MediaNumBean`), `agree_num`(Long), `reply_num`(Long), `share_num`(Long), `user_id`(Long), `first_post_id`(Long), `user_agree`(Int).
`MediaNumBean`: `pic`(Int).
`MediaBean`: `type`, `width`, `height`, `small_pic`, `big_pic`, `water_pic`, `is_long_pic`(Int), `bsize`(bSize).

### 5.3 `SubFloorListBean`
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `subpost_list` | `List<PostInfo>` |
| `post` | `PostInfo` |
| `page` | `PageInfo` |
| `forum` | `ForumInfo` |
| `anti` | `AntiInfo` |
| `thread` | `ThreadInfo` |

`PostInfo`: `id`, `title`, `floor`, `time`, `content`(`List<ThreadContentBean.ContentBean>`), `author`(`ThreadContentBean.UserInfoBean`).
`ThreadInfo`: `id`, `title`, `author`, `reply_num`, `collect_status`.
`AntiInfo`: `tbs`.
`PageInfo`: `current_page`, `total_page`, `total_count`, `page_size`.
`ForumInfo`: `id`, `name`, `is_exists`, `first_class`, `second_class`, `is_liked`.

### 5.4 `ThreadContentBean` (extends `BaseBean`)
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `post_list` | `List<PostListItemBean>` |
| `page` | `PageInfoBean` |
| `user` | `UserInfoBean` |
| `forum` | `ForumInfoBean` |
| `display_forum` | `ForumInfoBean` |
| `has_floor` | String |
| `is_new_url` | String |
| `user_list` | `List<UserInfoBean>` |
| `thread` | `ThreadBean` |
| `anti` | `AntiInfoBean` |

`AntiInfoBean`: `tbs`.
`ThreadBean`: `id`, `title`, `thread_info`(`ThreadInfoBean`), `origin_thread_info`(`OriginThreadInfo`), `author`(`UserInfoBean`), `reply_num`, `collect_status`, `agree_num`, `create_time`, `post_id`, `thread_id`, `agree`(`AgreeBean`).
`ThreadInfoBean`: `thread_id`, `first_post_id`.
`AgreeBean`: `agree_num`, `disagree_num`, `diff_agree_num`, `has_agree`.
`UserInfoBean`: `is_login`, `id`, `name`, `name_show`, `portrait`(PortraitAdapter), `type`, `level_id`, `is_like`, `is_bawu`, `bawu_type`, `ip_address`.
`ForumInfoBean`: `id`, `name`, `is_exists`, `avatar`, `first_class`, `second_class`, `is_liked`, `is_brand_forum`.
`PageInfoBean`: `offset`, `current_page`, `total_page`, `has_more`, `has_prev`.
`OriginThreadInfo`: `title`, `content`(`List<ContentBean>`, ContentMsgAdapter).
`PostListItemBean`: `id`, `title`, `floor`, `time`, `content`(`List<ContentBean>`), `agree`(`AgreeBean`), `author_id`, `author`(`UserInfoBean`), `sub_post_number`, `sub_post_list`(`SubPostListBean`, SubPostListAdapter).
`SubPostListBean`: `pid`, `sub_post_list`(`List<PostListItemBean>`).
`ContentBean` (content fragment): `type`, `text`, `link`, `src`, `uid`, `origin_src`, `cdn_src`, `cdn_src_active`, `big_cdn_src`, `during_time`, `bsize`, `c`, `width`, `height`, `is_long_pic`, `voice_md5`.
**Content `type` values** (from `ContentMsgAdapter` semantics): `0`=text, `1`=link, `2`=emoticon, `3`=image, `4`=@user, `5`=video, `10`=voice, `20`=image(alt).

### 5.5 `SearchThreadBean` (dual Gson+kotlinx annotations; `@SerialName` = `@SerializedName`)
| wire name | type |
|---|---|
| `no` | Int (errorCode) |
| `error` | String (errorMsg) |
| `data` | `DataBean` |

`DataBean`: `has_more`(Int), `current_page`(Int), `post_list`(`List<ThreadInfoBean>`).
`ThreadInfoBean`: `tid`, `pid`, `cid`(="0"), `title`, `content`, `time`, `modified_time`(Long), `post_num`, `like_num`, `share_num`, `forum_id`, `forum_name`, `user`(`UserInfoBean`), `type`(Int), `forum_info`(`ForumInfo`), `media`(`List<MediaInfo>`), `main_post`(`MainPost`), `post_info`(`PostInfo`).
`MediaInfo`: `type`, `size`, `width`, `height`, `water_pic`, `small_pic`, `big_pic`, `src`, `vsrc`, `vhsrc`, `vpic`.
`MainPost`: `title`, `content`, `tid`(Long), `user`, `like_num`, `share_num`, `post_num`.
`PostInfo`: `tid`(Long), `pid`(Long), `title`, `content`, `user`.
`ForumInfo`: `forum_name`, `avatar`.
`UserInfoBean`: `user_name`, `show_nickname`, `user_id`, `portrait`.

### 5.6 `SearchForumBean` (Gson)
| wire name | type |
|---|---|
| `no` | Int (errorCode) |
| `error` | String (errorMsg) |
| `data` | `DataBean` |

`DataBean`: `has_more`(Int=0), `pn`(Int=0, page), `fuzzyMatch`(`List<ForumInfoBean>`, ForumFuzzyMatchAdapter), `exactMatch`(`ForumInfoBean`, ExactMatchAdapter).
`ForumInfoBean`: `forum_id`(Long), `forum_name`, `forum_name_show`, `avatar`, `post_num`(="0"), `concern_num`(="0"), `has_concerned`(Int=0), `intro`, `slogan`, `is_jiucuo`(Int).

### 5.7 `SearchUserBean` (Gson)
| wire name | type |
|---|---|
| `no` | Int (errorCode) |
| `error` | String (errorMsg) |
| `data` | `SearchUserDataBean` |

`SearchUserDataBean`: `pn`(Int, pageNum), `has_more`(Int=0), `exactMatch`(`UserBean`), `fuzzyMatch`(`List<UserBean>`).
`UserBean`: `id`, `intro`, `user_nickname`, `show_nickname`, `name`, `portrait`, `fans_num`, `has_concerned`(Int=0).

### 5.8 `UserPostBean` (Gson)
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `hide_post` | String |
| `post_list` | `List<PostBean>` |

`PostBean`: `agree`(`AgreeBean`), `forum_id`, `thread_id`, `post_id`, `is_thread`, `create_time`, `is_ntitle`, `forum_name`, `title`, `user_name`, `is_post_deleted`, `reply_num`, `freq_num`, `user_id`, `name_show`, `user_portrait`(PortraitAdapter), `post_type`, `content`(`List<ContentBean>`, UserPostContentAdapter), `abstract`(`List<PostContentBean>`).
`AgreeBean`: `agree_num`, `disagree_num`, `diff_agree_num`, `has_agree`.
`ContentBean`: `post_content`(`List<PostContentBean>`), `create_time`, `post_id`.
`PostContentBean`: `type`, `text`.

### 5.9 `Profile` (Gson, OFFICIAL JSON)
Top: `anti`(`Anti{tbs}`), `anti_stat`(`AntiStat`), `block_info`(`BlockInfo`), `error_code`, `nickname_info`(`NicknameInfo`), `user`(`User`), `user_agree_info`(`UserAgreeInfo`).
`AntiStat`: `block_stat`, `days_tofree`, `has_chance`, `hide_stat`, `vcode_stat`.
`BlockInfo`: `is_auto_pay`, `is_ban`, `is_permanent_ban`.
`NicknameInfo`: `left_days`.
`User`: `bg_pic`, `birthday_info`(`BirthdayInfo`), `bookmark_count`, `bookmark_new_count`, `can_modify_avatar`, `concern_num`, `display_auth_type`, `each_other_friend`, `editing_nickname`, `fans_num`, `favorite_num`, `friend_num`, `gift_num`, `has_concerned`, `id`, `intro`, `ip_address`, `is_default_avatar`, `is_fans`, `is_invited`, `is_mask`, `is_mem`, `is_nickname_editing`, `likeForum`(`List<LikeForum>`), `like_forum_num`, `modify_avatar_desc`, `my_like_num`, `name`, `name_show`, `portrait`, `portraith`, `post_num`, `priv_sets`(`PrivSets`), `repost_num`, `seal_prefix`, `sex`, `tb_age`, `thread_num`, `total_agree_num`, `total_visitor_num`, `user_growth`(`UserGrowth`), `user_pics`(`List<UserPic>`), `visitor_num`.
`BirthdayInfo`: `age`, `birthday_show_status`, `birthday_time`, `constellation`.
`LikeForum`: `forum_id`, `forum_name`.
`PrivSets`: `bazhu_show_inside`, `bazhu_show_outside`, `friend`, `group`, `like`, `live`, `location`, `post`, `reply`.
`UserGrowth`: `level_id`, `score`, `target_score`, `tmoney`.
`UserPic`: `big`, `small`.
`UserAgreeInfo`: `total_agree_num`.

### 5.10 `ProfileBean` (Gson, MINI JSON)
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `anti` | `AntiBean{tbs}` |
| `user` | `UserBean` |

`UserBean`: `id`, `name`, `name_show`, `portrait`, `intro`, `sex`, `post_num`, `repost_num`, `thread_num`, `tb_age`, `my_like_num`, `like_forum_num`, `concern_num`, `fans_num`, `has_concerned`, `is_fans`.

### 5.11 `MsgBean` (extends `ErrorBean`)
| wire name | type |
|---|---|
| `message` | `MessageBean` |
`MessageBean`: `replyme`(replyMe), `atme`(atMe), `fans`.

### 5.12 `MessageListBean` (extends `BaseBean`)
| wire name | type |
|---|---|
| `error_code` | String |
| `time` | Long |
| `reply_list` | `List<MessageInfoBean>` (MessageListAdapter) |
| `at_list` | `List<MessageInfoBean>` (MessageListAdapter) |
| `page` | `PageInfoBean` |
| `message` | `MessageBean` |

`UserInfoBean`: `id`, `name`, `name_show`, `portrait`.
`ReplyerInfoBean`: `id`, `name`, `name_show`, `portrait`, `is_friend`, `is_fans`.
`MessageInfoBean`: `is_floor`, `title`, `content`, `quote_content`, `replyer`(`ReplyerInfoBean`), `quote_user`(`UserInfoBean`), `thread_id`, `post_id`, `time`, `fname`(forumName), `quote_pid`, `thread_type`, `unread`.
`MessageBean`: `replyme`, `atme`, `fans`, `recycle`, `storethread`.
`PageInfoBean`: `current_page`, `has_more`, `has_prev`.

### 5.13 `ThreadStoreBean`
| wire name | type |
|---|---|
| `error_code` | String |
| `error` | `ErrorInfo{errno,errmsg}` |
| `store_thread` | `List<ThreadStoreInfo>` |

`ThreadStoreInfo`: `thread_id`, `title`, `forum_name`, `author`(`AuthorInfo`), `media`(`List<MediaInfo>`), `is_deleted`, `last_time`, `type`, `status`, `max_pid`, `min_pid`, `mark_pid`, `mark_status`, `post_no`, `post_no_msg`, `count`.
`MediaInfo`: `type`, `small_Pic`(smallPic — note capital P), `big_pic`, `width`, `height`.
`AuthorInfo`: `lz_uid`, `name`, `name_show`, `user_portrait`.

### 5.14 `SignResultBean`
| wire name | type |
|---|---|
| `user_info` | `UserInfo` |
| `error_code` | String |
| `error_msg` | String |
| `time` | Long |

`UserInfo`: `user_id`, `is_sign_in`, `cont_sign_num`, `user_sign_rank`, `sign_time`, `sign_bonus_point`, `level_name`, `levelup_score`, `all_level_info`(`List<AllLevelInfo{id,name,score}>`).

### 5.15 `OneKeySignInBean`
| wire name | type |
|---|---|
| `data` | `SignInData` |
`SignInData`: `signedForumAmount`, `unsignedForumAmount`.

### 5.16 `FollowBean`
| wire name | type |
|---|---|
| `error_code` | Int |
| `error_msg` | String |
| `status` | String |
| `info` | `Info{toast_text, is_toast}` |

### 5.17 `LikeForumResultBean`
| wire name | type |
|---|---|
| `error_code` | String |
| `error` | `ErrorInfo{errno,errmsg,usermsg}` |
| `info` | `Info{cur_score, levelup_score, level_id, level_name, member_sum}` |
| `userPerm` | `UserPermInfo{level_id, level_name}` |

### 5.18 `AgreeBean`
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `data` | `AgreeDataBean{agree: AgreeInfoBean{score}}` |

### 5.19 `AddPostBean`
| wire name | type |
|---|---|
| `anti_stat` | `AntiStat` |
| `contri_info` | `List<Any>` |
| `ctime` | Int |
| `error_code` | String |
| `exp` | `Exp` |
| `info` | `Info` |
| `logid` | Long |
| `msg` | String |
| `opgroup` | String |
| `pid` | String |
| `pre_msg` | String |
| `server_time` | String |
| `tid` | String |
| `time` | Int |

`AntiStat`: `block_stat`, `days_tofree`, `has_chance`, `hide_stat`, `vcode_stat`.
`Exp`: `color_msg`, `current_level`, `current_level_max_exp`, `old`, `pre_msg`.
`Info`: `access_state`(`List<Any>`), `confilter_hitwords`(`List<Any>`), `need_vcode`, `pass_token`, `vcode_md5`, `vcode_prev_type`, `vcode_type`.

### 5.20 `ReplyResultBean`
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `info` | `InfoBean` |
| `pid` | String |

`InfoBean`: `need_vcode`, `vcode_md5`(vcodeMD5), `vcode_pic_url`, `pass_token`.

### 5.21 `UploadPictureResultBean`
| wire name | type |
|---|---|
| `error_code` | String |
| `error_msg` | String |
| `resourceId` | String |
| `chunkNo` | String |
| `picId` | String |
| `picInfo` | `PicInfo` |

`PicInfo`: `originPic`, `bigPic`, `smallPic` (each `PicInfoItem`).
`PicInfoItem`: `width`, `height`, `type`, `picUrl`.

### 5.22 `ForumHome` (web) — extends `WebBase<ForumHomeData>`
`WebBase<Data>`: `errno` (alternate `no`) Int errorCode, `errmsg` (alternate `error`) String errorMsg, `data`.
`ForumHomeData`: `like_forum`(`LikeForum`).
`LikeForum`: `list`(`List<ListItem>`), `page`(`Page{currentPage, totalPage}`).
`ListItem`: `avatar`, `forum_id`(Long), `forum_name`, `hot_num`(Long), `is_brand_forum`(Int), `level_id`(Int).

### 5.23 `web.Profile` — extends `WebBase<ProfileData>`
`ProfileData`: `is_login`(Int), `sid`, `user`(`User`).
`User`: `intro`, `name`, `name_show`, `portrait`, `sex`(Int), `show_nickname`(showNickName).

### 5.24 `HotTopicMainBean` (Java) — extends `WebBaseBean<HotTopicMainDataBean>`
`WebBaseBean<Data>`: `no`(int errorCode), `error`(String errorMsg), `data`.
`HotTopicMainDataBean`: `best_info`(`BestInfoBean{ret: List<BestInfoRetBean>}`).
`BestInfoRetBean`: `common_type`, `module_name`, `module_recoms`(`List<String>`), `thread_list`(`Map<String,ThreadBean>`), `recom_type`, `topic_id`.
`ThreadBean`: `abstract`(abstracts), `agree_num`, `avatar`, `create_time`, `forum_id`, `forum_name`, `media`(`List<MediaBean>`), `name_show`, `post_num`, `thread_id`, `user_id`, `title`.
`MediaBean`: `big_pic`, `height`(int), `width`(int), `small_pic`, `type`, `water_pic`.

### 5.25 `HotMessageListBean` (Java) — extends `BaseBean`
| wire name | type |
|---|---|
| `no` | int (errorCode) |
| `error` | String (errorMsg) |
| `data` | `HotMessageListDataBean` |
`HotMessageListDataBean`: `list`(`DataListBean{ret: List<HotMessageRetBean>}`).
`HotMessageRetBean`: `mul_id`, `mul_name`, `topic_info`(`TopicInfoBean{topic_desc}`).

---

## 6. Supporting enums

- `ForumSortType`: `REPLY_TIME=0`, `SEND_TIME=1`, `ONLY_FOLLOWED=2`.
- `SearchThreadOrder`: `NEW=1`, `OLD=0`, `RELEVANT=2`.
- `SearchThreadFilter`: `ONLY_THREAD=1`, `ALL=2`.
- `ClientVersion`: `TIEBA_V11="11.10.8.6"`, `TIEBA_V12="12.52.1.0"`, `TIEBA_V12_POST="12.35.1.0"`.
