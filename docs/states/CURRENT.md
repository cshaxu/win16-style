# Project Status

## Current Work

无活动任务包。M0 T13 S1多语言控件字体保护已完成验证、双发布、P1实际远端复核及清理。
S1及T13收口，M0开放；用户追加要求的占用发布规范已修正并实际执行。
M0 T12 S1已完成缺失IAT目录的DoubleKiller启动兼容、验证、双发布及
P1实际远端复核；T12收口，M0开放。T8/T9/T11关闭、T10取消的状态保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 13
- Last allocated Td S: 10
- Last delivery: M0 T13 S1 P2 (P1 83df08c远端150输入/四产物及八发布摘要通过；两登记scratch回收)
- Open task: none；T13关闭；T12/T11/T9/T8关闭；T10取消；Td S9保持关闭

## Technical Baseline

0.13.0.0 / M0 T13 S1已验证、双发布及实际远端复核并收口；三组件四文件、x64 Console、双宽DLL、ABI3保持。
控件原字体非ANSI或实际文本不适用CP1252时保留原HFONT及原生回退；动态文字/字体重判，
列表/下拉包含未选中项目和编辑字段；不按系统UI语言替换字体，不改控件矩形。
可编辑下拉内部EDIT局部观察，重入移除/销毁、DLL卸载后原生控件存活及GDI不增长通过。

GCC16.1.0/Ninja/共享ccache独立干净双宽构建，150份构建/审查输入冻结；实际包25/25。
冻结注册全集71项逐项组合68通过/3跳过，实际包8项fixture与余下CTest63项合并，不重复计数。
跳过为双宽private clipboard及可选installed-run16兼容；物理输入/IME和任意DPI不计覆盖。
8脚本原字形/墨迹、96逻辑及实际144 DPI通过；旧0.12两宽DLL均被新测试检出。

assets根及system32八摘要一致；system32实际真实扫雷正常显示/关闭通过，有原生截图。
旧/新英雄榜标题24行RGB差异0，frame/menu源字节保持；原资源六文件及两DLL18表、ABI保持。
已授权发布包含关闭核实的文件占用者，按用户追加要求修正规范并处理GlassCMD实际占用。
T12旧IAT、T11 UAC、T9原生cmd及已有传播的相关回归保持。NE间歇边界、UWP取消及
TODO的独立DWM/回滚计数债务不扩张为已解决。证据见[记录](../history/M0-T13-S1-multilingual-font.md)。

## Recent Closure

- [T13多语言控件字体](../history/M0-T13-S1-multilingual-font.md)：83df08c远端150输入/四产物通过，71项68通过3跳过、实际包25/25、八发布摘要及真实扫雷通过；S1/T收口。

- [Td S10英雄榜字体调查](../history/M0-Td-S10-leaderboard-font.md)：11cea15远端6文档/四产物通过，中文墨迹缩小已定位；无产品修复/发布。

- [T12缺失IAT启动修复](../history/M0-T12-S1-legacy-iat.md)：507847c实际远端139输入/四产物、8发布摘要及真实DoubleKiller通过；S1/T12收口，M0开放。

- [T11按需UAC](../history/M0-T11-S1-uac.md)：d77017f实际远端139输入/四产物及8发布摘要通过；S1/T11收口，M0开放。

- [T10取消研究](../history/M0-T10-S1-xaml-research.md)：用户取消，无产品支持发布；T8按用户明确要求收口。

- [Td S9构建与测试效率](../history/M0-Td-S9-test-efficiency.md)：425b7df实际远端复核通过；S9收口，M0/T8开放。

- [T9 S1原生嵌套cmd及外部导入表](../history/M0-T9-S1-native-cmd-staging.md)：P1实际远端复核及清理完成；仅本轮S/T9收口，T8开放。

- [Td S8传播诊断](../history/M0-Td-S8-vdm-propagation.md)：ce20f15实际远端复核通过；原NE间歇失败不作修复声明，独立staging缺陷移交当前产品任务。

- [Td S7 README编辑](../history/M0-Td-S7-readme-edit.md)：894e012实际远端复核通过；指定沿革段落已删除。

- [Td S6英文README与真实截图](../history/M0-Td-S6-english-readme.md)：02df11a实际远端复核通过；8张公开PNG保留，scratch已安全回收。

- [S4菜单比例修复](../history/M0-T8-S4-menu-proportions.md)：64e3fd3实际远端复核通过，S4收口；T8保持开放。
- [S3菜单厚黑边修复](../history/M0-T8-S3-popup-shadow.md)：aa28427实际复核通过；S3收口。
- [S2 System字形修复](../history/M0-T8-S2-system-font.md)：2a0c7dd实际复核通过；S2收口。
- [S1字体大小修复](../history/M0-T8-S1-font-layout.md)：1c82c2e收口。
- [菜单取消修复](../history/M0-T7-menu-cancellation.md)与[历史产品审计](../history/M0-T6-completion.md)保留历史边界。
