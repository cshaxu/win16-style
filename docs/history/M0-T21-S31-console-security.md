# M0 T21 S31：console 与 security 基础机制

产品提交 `345238f` 按用户意见使用独立 `src/lib-win32/console` 而非 io，提取 stderr 重新绑定；`src/lib-win32/security` 提取当前进程提权状态查询。runas 调用、参数与托盘等业务仍在启动器；不是完整的通用进程启动或提权框架。两个库均不依赖产品状态，隔壁尚未适配或实测。

改名后双宽控制台测试通过；`s31-clean2` 双宽干净构建、ABI 和原资源检查通过，`s31-package1` 实包聚焦 13/13 无跳过，含双宽模拟 UAC 和控制台重绑；真实 UAC 和完整 UI 矩阵未测。`s31-publish1` 四文件发布 assets 根和 O:/winnt 并核对；远端产品及组件源码对象复核一致。

SHA-256：run16s `b4124e08653581c7672483c6333af7a2d017986f04e3ce410b66235edd035806`；ntw16s32 `19637eca32e9bf46f8df9f1be164163c0085677b0643c014af0a771fb488a33a`；ntw16s64 `d8f4fb68c689c043284e2eb1671933aac5186d634e6b2b9722337e485bff84b5`；demo `5872d969b00923e11d73059189d016ba466aedaf6d69f8e91b787e5719277e7d`。
