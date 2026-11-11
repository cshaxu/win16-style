# M0 T25 S4 P9：工具窗口标题字号

只读对照 Windows 2000 辅助功能向导资源表中的默认窗口度量，工具标题字体高度为 -8（96 DPI），不同于普通标题的 -11。工具窗口现使用按 DPI 缩放的较小标题字体，普通标题保持原有计算。原版来源、SHA、几何默认值及未覆盖面见 [S4 审计](M0-T25-S4-window-audit.md)。使用相同增量探针，旧包工具标题字形高 9 像素、未满足新测试；新 x86/x64 均为 5 像素。字体栅格化由当前系统提供，不代表原版逐像素一致。

源码快照 SHA-256 `6ac610acfdde4cd1eb1706f4b5a158a72991c023cef6500572844ab28405102a` 使用纯 C17、MinGW GCC 16.1.0、Ninja 和 ccache 4.13.6 双位宽干净构建；正式包 36 项重点回归通过、无跳过。正式包未测试物理鼠标；也未验证全部 DPI、真实工具窗口、原版几何或用户视觉验收。

同批产品发布到 assets 根目录和 `O:\winnt`，与测试包 SHA-256 一致：

| 文件 | SHA-256 |
| --- | --- |
| demo.exe | 4d197c425f53eec7d3c948747986f03b757f62c912bc9c6349117ce09fdb2d29 |
| ntw16s32.dll | 1279876d3025b7d8b5e75e95933b8cd10b6469d4d4b8cc9118d4207588a864c9 |
| ntw16s64.dll | 3810f7e1c3216adb84a1e82d7e60637a1275af73c2b0c343f7fd378b4ccb5f9e |
| ntw2ks32.dll | 4d6d9eb8d361c1e1d15700298e1ef8243aa9b4dfedca6a9926901cf15c291439 |
| ntw2ks64.dll | f4675a9442cef96b2509aa28d07524707cfe81839a87919e220e9f91bfe9d90e |
| ntwshk32.dll | 29b70af3d6abe0044864360f9df573eaa9287bc685bbd21fa38497338c96e844 |
| ntwshk64.dll | 1f67e3865eff91dfa77153c220f06e8d1a61ce6a3a0e753482a0b04130a35500 |
| run16s.exe | 9ab6fa5d63437501f5f51b211d2bde8dbbe9cb09b42a747b867bbe22768b707c |

S4 和 T25 保持开放。
