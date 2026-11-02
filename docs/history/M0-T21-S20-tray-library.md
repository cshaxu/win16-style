# M0 T21 S20: LibTray

用户准入的内部公共托盘库，不增加产品文件。`src/lib/tray` 接收调用者创建的菜单及命令回调，管理通知图标、鼠标消息、Explorer 重启通知和窗口资源。库内不含产品名称；窗口类为 `LibTrayWindow`。启动器控制器保留菜单文字、图标资源及原全局业务动作；删除旧 `src/run16s-exe/tray.c`。ntw16s DLL 没有托盘调用点，未添加无用的 DLL 依赖或改动注入、样式逻辑。

双位宽同快照干净构建、ABI 与包检查通过，GCC 16.1.0、Ninja、ccache 4.13.6；来源与资源身份门禁通过。x86/x64 LibTray 原生消息测试各通过，覆盖左右键单击、双击、选择与取消，使用模拟通知和选择。10 项同批四文件聚焦包测试通过、无跳过；首次沙箱内有 5 项因私有桌面错误 5 跳过，沙箱外重试通过。私有桌面控制器替身验证默认关闭、菜单、图标、开关状态和退出；替身不执行全局挂钩。同源码测试通知夹具的默认菜单探针通过；真实 Explorer 点击、真实 UAC、全局接入和恢复未由本次证明。

首次发布时 `O:/winnt/ntw16s64.dll` 被占用，回退后逐项摘要一致。由于系统宿主、Codex 等多个进程映射旧 DLL，保留旧映像于 `build/t21-package/s20-release-backup/held-ntw16s64.dll` 后完成替换；已有进程可能继续使用旧映像。该文件仍被映射，不能作为普通临时文件清理。

测试包及 assets、O:/winnt 两处 SHA-256：

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | `1c2e0edcf1fcf975afc6abdeb70a1976725ce6ffadeddb35d47df9347178075f` |
| ntw16s32.dll | `783ee5a93355d4146ae4bc6038d4916c237e853bf3631f8546f63f0489b7965a` |
| ntw16s64.dll | `c1c2aa930089eceeb884434503a137d597d7b0aa443525632da58ec835a2dc94` |
| demo.exe | `4590e62b13e20a958acab468394c0f1f7a9f02d9988ae248264c7cf747ecaf9e` |

候选、包测试与探针暂存分别为 `build/t21-package/s20-clean2`、`build/t21-package/s20-package-retry`、`build/t21-package/s20-tray-integration`。取消的 `s20-clean1`、被环境回收的 `s20-tray-package` 和首次包测试的跳过不能计为通过。保持 S20 开放，等待用户验收。

产品提交 `8f258925d237a25e7143d20c3afebd28eb35e48b` 已推送；独立读取远端 main 及该提交的四个资产文件和两份 LibTray 源码，SHA-256 与本地逐项一致。P1 文本变更增加 370 行、删除 123 行、净增加 247 行（不含四个二进制文件）；旧托盘机制文件删除，公共库负责托盘对象生命周期，启动器仍负责菜单和业务控制器状态。
