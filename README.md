# 百度贴吧+ 1.0.0 for Nokia N9 / MeeGo 1.2 Harmattan

第三方百度贴吧客户端，复刻自开源项目 [TiebaLite](https://github.com/HuanCheng65/TiebaLite)（作者 HuanCheng65），
为 **Nokia N9 (MeeGo 1.2 Harmattan)** 原生移植：Qt 4.7.4 + Qt Quick 1.1（`import QtQuick 1.1` / `com.nokia.meego 1.0` / `com.nokia.extras 1.1`）。

> **免责声明**：本项目仅用于学习 Qt 4.7 / QML / Protobuf 逆向接口等技术，**非商业用途**，与百度官方无关。
> 数据接口来自公开抓包与开源资料（原项目授权 LGPL-3.0 学习使用）。请在遵守当地法律法规与平台规则的前提下使用。
> 图片、内容版权归原作者及百度所有。开发者不对使用本软件造成的任何风险、损失或后果承担责任。

本项目是 AI 项目，部分代码、文档或界面内容可能由人工智能辅助生成或修改。使用者应自行审查结果并承担使用风险。

---

## 功能总览（对照原 TiebaLite）

| 模块 | 状态 | 说明 |
|---|---|---|
| 登录 | ✅ | 手动粘贴 BDUSS/STOKEN/tbs（网页登录 Cookie 提取待完善） |
| 关注吧首页 | ✅ | 查看已关注的贴吧 |
| 吧页（frs） | ✅ | Protobuf 302001，翻页、排序、关注/取关 |
| 帖子页（pb） | ✅ | Protobuf 302001 楼中楼、只看楼主、倒序、跳页、举报（打开举报网页） |
| 楼中楼 | ✅ | Protobuf 302002 |
| 发帖 / 回复 | ✅ | Protobuf 309731；支持选图上传（`/c/s/uploadPicture` 分块 multipart） |
| 搜索 | ✅ | 帖子 / 吧 / 用户 + 搜索历史 |
| 赞 / 收藏 / 取消收藏 | ✅ | JSON |
| 消息（回复 / @） | ✅ | JSON feeds |
| 用户主页 / 帖子 | ✅ | Protobuf 303012 |
| 图片查看 / 保存 | ✅ | 滑动翻页；保存到 ~/MyDocs/Pictures |
| 草稿 / 历史 / 黑名单 / 收藏 | ✅ | SQLite 本地 |
| 主题（浅/深/纯黑）+ 强调色 + 密度 + 字号 | ✅ | 运行时切换 |
| 日志查看 | ✅ | 关于页 → 日志（HTTP/错误，~/.config/TiebaLite/tieba.log） |
| 多语言 | ✅ | zh-CN/zh-TW/ja/ko/en：核心界面已用 qsTr() 包裹，`i18n/*.ts` 已提供（lupdate/lrelease 编译 .qm，按系统语言加载） |
| 动态（个性化推荐） | 🚧 | A1 personalizedFlow 响应结构待定，占位页 |
| 网页登录 Cookie 提取 / 双指缩放 | 🚧 | 见“已知限制” |

## 页面清单

`qml/tieba/`：

- `main.qml` — PageStackWindow + 全局 toast
- `MainPage.qml` — 底部 4 Tab（首页/动态/消息/我的）+ 菜单
- `HomePage.qml` — 关注吧列表
- `ExplorePage.qml` — 动态（占位）
- `NotificationsPage.qml` — 回复 / @
- `UserPage.qml` — 我的（帖子/收藏/草稿/历史/黑名单/设置/关于/退出）
- `ForumPage.qml` — 吧页（帖子列表）
- `ThreadPage.qml` — 帖子页（楼层 + 菜单）
- `SubFloorPage.qml` — 楼中楼
- `ReplySheet.qml` — 回复/发帖底部弹层（Sheet）
- `SearchPage.qml` — 搜索（3 类 + 历史）
- `UserProfilePage.qml` — 用户主页
- `PhotoViewPage.qml` — 图片查看器
- `LoginPage.qml` — 登录
- `SettingsPage.qml` — 设置
- `AboutPage.qml` — 关于（版本/日志/缓存/免责/致谢）
- `DraftPage.qml` / `HistoryPage.qml` / `StorePage.qml` / `BlacklistPage.qml`
- `LogPage.qml` — 日志查看（HTTP/错误）
- `components/` — CachedImage / Avatar / StatusView / RichContent / ImageGrid / ThreadCard / FloorCard
- `util.js` — 显示辅助（富文本 HTML、图片提取、头像解析等）

## C++ 层（src/）

| 文件 | 职责 |
|---|---|
| `json.h/cpp` | 手写 JSON 解析/序列化（QVariant 树，支持 \uXXXX） |
| `pbwire.h/cpp` | Protobuf 手写编解码（varint / fixed32/64 / length-delimited / skipField） |
| `signutil.h/cpp` | 请求签名：`md5(排序后的 k=v 无分隔拼接 + "tiebaclient!!!")` 小写 hex + st* 反爬参数 |
| `clientinfo.h/cpp` | 设备信息（cuid/客户端 ID/UA/版本号，QSettings 持久化） |
| `httpclient.h/cpp` | QNetworkAccessManager 封装，60s 超时，信号回调 |
| `db.h/cpp` | SQLite：账号/草稿/历史/搜索历史/收藏/黑名单/吧缓存/页面离线缓存 |
| `accountmanager.h/cpp` | 账号状态（BDUSS/STOKEN/tbs/uid） |
| `appsettings.h/cpp` | 设置（主题/强调色/密度/字号/开关，QSettings） |
| `thememanager.h/cpp` | 主题色/字号/间距计算，QML 上下文属性 `theme` |
| `imagecache.h/cpp` | 三级图片缓存（内存去重 + 磁盘 md5 缓存） |
| `notifier.h/cpp` | org.freedesktop.Notifications DBus 通知 |
| `util.h/cpp` | timeAgo/复制/打开链接/保存图片/文件选择器/Toast |
| `content.h/cpp` | 富文本内容模型（头像/表情/图片/语音 URL 组装） |
| `proto_messages.h/cpp` | 全部业务 Proto 请求/响应构建与解析 |
| `tiebaapi.h/cpp` | 统一 API 门面：签名 → 发送 → 解析 → QML 信号 |

## 构建（Qt Creator + MADDE / Scratchbox2）

1. 安装 Harmattan SDK（Qt 4.7.4 target）。
2. 用 Qt Creator 打开 `tieba.pro`（选择 Harmattan 构建套件）。
3. 构建生成 `tieba` 二进制 + `.deb`（`qtc_packaging/debian_harmattan/` 已就绪）。
4. 部署：`scp` 到 N9 后 `dpkg -i tieba_*.deb`，或直接用 Creator 远程部署。

依赖：`libqt4-network`、`libqt4-sql-sqlite`、`libqt4-dbus`（运行时随包安装）。

### Windows 本地构建

在安装 QtSDK/MADDE 后，可以使用仓库中的脚本或直接在 Qt Creator 中打开 `tieba.pro`：

```powershell
./rebuild-harmattan.ps1
```

推荐使用单独的构建目录。构建产物和本地配置不会提交到仓库。

## 数据与隐私

- 登录凭据、账号设置、草稿、历史、收藏和黑名单保存在设备本地，不会由本项目主动上传到第三方服务器。
- SQLite 数据库位于 `~/.config/TiebaLite/tieba.db`。
- 图片缓存位于 `~/.cache/TiebaLite/images`，日志位于 `~/.config/TiebaLite/tieba.log` 和 `qml.log`。
- 设置页的“清除缓存”只删除图片缓存、贴吧缓存和离线页面缓存，不删除账号、草稿、历史或收藏。
- 请勿将本地数据库、日志、Cookie、密钥、构建目录或安装包上传到公开仓库。

## 技术要点

- **签名**：所有业务请求先按 key 排序拼 `k=v`（无分隔符），末尾追加 `tiebaclient!!!`，MD5 小写 hex 作为 `sign` 参数；`stErrorNums/stMethod/stMode/stTimesNum/stTime/stSize` 反爬参数在签名**之后**追加。
- **Protobuf 传输**：`multipart/form-data`，boundary `--------7da3d81520810*`，`data` 部分为外层 `*Request{data=1}` 的裸字节；`cmd` 走 query（301001/302001/302002/309731/303012/303021）。
- **数据**：SQLite（`~/.config/TiebaLite/tieba.db`），图片缓存（`~/.cache/TiebaLite/images`，md5 文件名）。
- **低资源**：图片按需加载 + 磁盘缓存；列表懒加载；单核 1GHz 下可流畅滚动。
- **离线缓存**：吧列表/帖子楼层/楼中楼每页成功加载后写入 SQLite（`page_cache`，14 天过期）；断网时自动回退展示缓存页并提示"离线模式"。
- **多语言**：核心界面文案已用 `qsTr()` 包裹；`i18n/tieba_{zh_CN,zh_TW,ja,ko,en}.ts` 已提供，构建时 `lrelease` 生成 `.qm` 部署到 `/opt/tieba/i18n`，按系统语言自动加载。
- **日志**：`LogStore` 记录 HTTP/错误到 `~/.config/TiebaLite/tieba.log`（内存保留 500 条），关于页可查看。

## 已知限制 / TODO

- 图片上传：`ReplySheet` 选图（原生文件选择器）→ 分块上传 `/c/s/uploadPicture`（512000B/块，multipart，无签名；`resourceId=文件MD5+块大小`）→ 内容追加 `#(pic,picId,宽,高)`。上传 Cookie 用登录时保存的 `BAIDUID`（登录页可选填）；超大图（>5MB）自动压缩到 ≤1080px JPEG。
- 网页登录的 Cookie 自动提取未实现（需自定义 QNetworkCookieJar）；当前用手动粘贴 BDUSS/STOKEN/tbs/BAIDUID。
- 个性化推荐（A1 personalizedFlow）响应结构未确定，动态页为占位。
- Qt Quick 1.1 无 `PinchArea`，图片查看器不支持双指缩放（支持滑动 + 保存）。
- 内联表情以 `[名称]` 文本展示（StyledText 无法加载远程 `<img>`）。
- 离线缓存覆盖吧列表 / 帖子 / 楼中楼各页（断网自动回退）；图片本身未做离线保存。

## 致谢

- 原项目 [TiebaLite](https://github.com/HuanCheng65/TiebaLite)（HuanCheng65）
- [Tieba-Protobuf](https://github.com/n0099/Tieba-Protobuf)（n0099）协议解析参考
- 文档参考：`docs/REF-api-endpoints.md`（65 个接口）、`docs/REF-protobuf.md`（150 条消息）、`docs/REF-content-model.md`
