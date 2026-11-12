# M0 T25 S8 P9: 统一窗口样式切换

用户反馈 Win2000 切换到 Win16 后没有变成 Win16。审计发现原适配器在旧样式仍由
全局协调器管理时直接返回 ERROR_BUSY，且原测试错误地要求这一结果。现由公共
`style_global_window_transition` 按所选操作表先移除旧样式、再应用新样式；新样式
失败时尽力恢复旧样式和原先的管理状态。产品适配器只选择两种样式的操作表。

纯 C17、MinGW GCC、Ninja、ccache 双位宽干净构建通过，快照清单 SHA-256 为
`7fbc225d0ca6938aa131ce61b1eab1f3271ba99f00cb82e3f7a46ed6a6688561`。发布前
通过包身份验证；先备份，再将被映射的旧文件移入本任务备份并逐个替换。八个
文件已发布到 assets 根目录与 `O:/winnt`，两处摘要与候选相同。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | `b6a1c71dd06321fb8c020caf81ce64244a738a48cec5fe6dc7fca96b1c75a283` |
| ntw16s32.dll | `df8fedb4d536d3c57d48596f41aa32c9297b41ca37517d0632b87b39fd4c0a8a` |
| ntw16s64.dll | `1378d3d6e601190fe5bb4dec16bed393d1244ce299785308a4f6d75b52e7e8bb` |
| ntw2ks32.dll | `15c3e04ecb362d050b7d6dd5bd5cdba185c619c7ee3e2c6f9f0e5a688a0afb56` |
| ntw2ks64.dll | `cb58567ed8f9dcd4ff3aa0a0b765c3b823ee97655d4b4a0238d3844b72435221` |
| ntwshk32.dll | `f6cd078996ef63448faf250fc1439e958c9a962f64d470f4d8d000128921a568` |
| ntwshk64.dll | `844a44fa5c7c16e066524fe529150472c055499f8412eaa938be50b3f05bd10d` |
| demo.exe | `ffe016fbf1b0f84cc9ba23f06b11420ff23a1b79b98c0fb24e5d4b13b23c97aa` |

发布后，正式托盘管理员探针对双位宽自建窗口验证了 normal → Win16、Win16 →
Win2000、Win2000 → Win16、Win16 → normal、normal → Win2000、Win2000 →
normal，及 Win2000 模式中新建窗口和托盘退出。窗口的样式附着与移除均通过；
首轮 Win16 和 Win2000 标题像素分别为 `ffffff`、`a1a1a1`（两位宽）。操作通过
辅助功能触发，不代表真实鼠标或所有应用均已验证。日志中每次约 563–568 个窗口
通知有 1 个失败或超时，旧版本 DLL 仍可能映射于先前运行的进程。

同批源码的双位宽共享协调器及产品桥接测试通过，覆盖旧样式仍受全局管理时双向
直接切换及新样式应用失败的回滚。附加的 Win2000 → Win16 后标题像素断言在包
构建后加入测试源码：首次未能打开托盘菜单，重试的管理员启动被取消；这一像素
断言未计通过，测试源码相对构建快照的变化仅涉及测试。此前原生 DWM 状态恢复
限制仍在。S8 等待补测和用户验收，T25 保持开放。
