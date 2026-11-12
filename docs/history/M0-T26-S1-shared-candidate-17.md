# T26 S1 第十七批共享候选

双位宽 MinGW、Ninja、ccache 干净构建通过，源码快照清单 SHA-256 为 `9e4c1e6222397b3409c1fc78efa93d7519f1e0a3d85536c423e78711836a0073`。源码、资源、治理及两位宽公共归档边界检查通过。先发布到 `assets` 和 `O:\winnt`，以下八文件与同批包摘要一致。

本批将轨迹条和工具栏共用的自绘通知来源校验，以及 XAML 图标消息处理的样式位暂存与恢复移入公共组件，Win16 实际使用。发布后同快照 x86/x64 公共通知和外框测试、自建 Win16 外框回归通过；`O:\winnt\run16s.exe` 驱动的自建轨迹条和工具栏模型在两种位宽通过绘制、回调、重入及移除。外框测试使用同快照构建树中的 DLL，不是对真实可见 XAML 应用的测试；Win2000 尚未接入这些新接口。

真实 UWP/WinUI 宿主、真实全局会话、线程钩子错误 126、Win2000 DWM 恢复及打印夹具限制不因本批改变。S1 全范围审计、提交、推送和远端复核尚未完成；测试未创建或切换隔离桌面。

| 文件 | SHA-256 |
| --- | --- |
| run16s.exe | 97c328e98d227792fed30d44d13e271e576f67e93bc6c15281fd5251e75e7d23 |
| ntw16s32.dll | 4dad85f18e34d0346b74005da4c81f6b38dc5c02d93612d82013b1626f38b756 |
| ntw16s64.dll | 0209e069c6b339c8f10680a247c1ad44db74b26f599892a0156be34a88ab4381 |
| ntw2ks32.dll | c1b223db60b2ad007725693a2806f7751a58f95c4e084d4bd3d0d2a7f0ee35c1 |
| ntw2ks64.dll | 7db745ec76086003cb88d8c7f207f2c5584ecd06b9d15b2b1837affe05c2baed |
| ntwshk32.dll | 2206efb3c67d4452c71944225b30acb019b65d3ad29a79e80800c045538e2d53 |
| ntwshk64.dll | d34ca9daf41c0528a01d9c574f060f0dbc124c11337c563adb2d2b98c2b7dad3 |
| demo.exe | a2b7fc083c35748a6f864b79158e210f7d0a19195d1f1da2ccd62380c90c2af0 |
