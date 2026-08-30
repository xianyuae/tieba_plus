# 百度贴吧+

百度贴吧+ 是面向 Nokia N9 / MeeGo 1.2 Harmattan 的第三方贴吧客户端，当前版本为 1.0.0。项目使用 Qt 4.7.4、Qt Quick 1.1、SQLite 与手写 Protobuf 编解码实现，与百度官方无关。

> 本项目是 AI 项目，部分代码、文档和界面内容可能由人工智能辅助生成或修改。软件按“原样”提供，开发者不对使用过程中造成的风险、损失或后果承担责任。请自行评估风险，并遵守当地法律法规及平台规则。

## 功能

- 账号登录与扫码登录
- 关注贴吧、浏览主题和楼中楼
- 搜索、发帖、回复、点赞与收藏
- 消息、草稿、浏览历史和黑名单
- 图片查看与保存
- 浅色、深色和纯黑主题
- 图片及离线页面缓存清理
- 本地运行日志查看

## 环境要求

- Nokia Harmattan SDK / MADDE
- Qt 4.7.4
- Qt Quick 1.1
- qmake 与 MADDE make

运行时依赖包括 Qt Network、Qt SQL SQLite 和 Qt DBus。

## 构建

推荐使用 Qt Creator 打开 `tieba.pro`，选择 Harmattan 构建套件后进行构建。也可以在已安装于 `C:\QtSDK` 的标准 Qt SDK 环境中运行：

```powershell
.\rebuild-harmattan.ps1
```

脚本在项目同级的 `tieba-harmattan-build` 目录构建，不会在源码目录生成 Makefile 或目标文件。桌面 Qt 4.7.4 调试构建可使用：

```powershell
.\build-desktop.ps1
```

## 数据与隐私

设备上的主要本地数据路径如下：

- 数据库：`~/.config/TiebaLite/tieba.db`
- 图片缓存：`~/.cache/TiebaLite/images`
- 日志：`~/.config/TiebaLite/tieba.log` 和 `qml.log`

设置页的“清除缓存”会删除图片缓存、关注贴吧缓存和离线页面缓存，不会删除账号、草稿、历史或收藏。

请勿向公开仓库提交 BDUSS、STOKEN、Cookie、数据库、日志、私钥、Qt Creator 用户配置、构建目录或安装包。仓库的 `.gitignore` 已排除常见本地文件，但提交前仍应检查变更内容。

## 致谢

- [TiebaLite](https://github.com/HuanCheng65/TiebaLite)，作者 HuanCheng65
- [Tieba-Protobuf](https://github.com/n0099/Tieba-Protobuf)，协议研究参考
- Material Design 图标路径，按 Apache License 2.0 使用

## 许可证

本修改项目以 [GNU General Public License v3.0 only](LICENSE) 发布，所基于的 TiebaLite 项目同样采用 GPL v3。第三方代码、Qt、图标、SDK 和服务内容继续适用各自的许可证及权利声明。

联系方式：xianyuaa123@gmail.com

仓库地址：https://github.com/xianyuae/tieba_plus
