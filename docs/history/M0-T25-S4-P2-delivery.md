# M0 T25 S4 P2：默认标题图标

带系统菜单但没有应用或窗口类图标的普通窗口现在使用系统默认图标，并为窗口设置的大图标预留优先级；变更依据及原版几何限制见 [S4 审计](M0-T25-S4-window-audit.md)。这里使用运行系统的 WINLOGO 图标，不声称它与 Windows 2000 原始图标一致。

源码快照 SHA-256 `0418a4f84750a142bde4befe9543d683cce1cfc83652a611962b58510f078434` 使用纯 C17、MinGW GCC 16.1.0、Ninja 和 ccache 4.13.6 双位宽干净构建。同批产品的 34 项隔离桌面重点测试无跳过并通过，包括 x86/x64 的默认图标像素及设置大图标后的变化、普通和工具窗口、标题状态、启动与传播；未测试物理输入或完整集成矩阵。普通和工具窗口同进程连续创建的先前私有桌面清理失败仍按 [审计](M0-T25-S4-window-audit.md)记录，不能算通过。

发布至 `assets` 根目录和 `O:\winnt`，两处八文件与测试包摘要一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 0ca1ce74d2b04663991c3ebd5b563b5296e9a502297590d7c4ff84b186b6bd13 |
| ntw16s32.dll | 510f2cedea31544d3496f7e744768bcbe66da8004084248141ff575d9dfb8283 |
| ntw16s64.dll | 53e292735ac0876ddc2d5c95ee72c317a98ae78c42b0b9486826eefdf6ec382d |
| ntw2ks32.dll | e5e6f56db4c3a0ff353a746fe51ac6cac30f9d433fa2af74f83b86d2ba08909e |
| ntw2ks64.dll | ddd5aa2bde2345d07dedd523fca04b338a86636ca0131e8f2187ac026de2037a |
| ntwshk32.dll | 36dd69bd7c0c6df5c38952b9125134b72b9981eb67f5cc8c65407e0509199dbe |
| ntwshk64.dll | d2779851ec056aa5e2f2909f2953c1203285111964472d965461f909646224b5 |
| run16s.exe | a82e61671830330bdac1587047f884a058d383633e9681356ecafa19acfdf4b8 |

S4 和 T25 保持开放；Win2000 非客户区的尺寸、按钮及窗口交互尚未完整验证。
