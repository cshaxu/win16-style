# Project Status

## Current Work

M0 T13 S1 verified locally：多语言控件原字体保护已通过双宽验证并双发布。
71项组合覆盖68通过/3跳过，实际包25/25，真实扫雷/旧DLL负例通过；待P1推送及远端复核。
用户追加授权已修正占用发布规范，关闭已核实GlassCMD后继续完成部署，无混包。
M0 T12 S1已完成缺失IAT目录的DoubleKiller启动兼容、验证、双发布及
P1实际远端复核；T12收口，M0开放。T8/T9/T11关闭、T10取消的状态保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 13
- Last allocated Td S: 10
- Last delivery: M0 Td S10 P2 (P1 11cea15实际远端6文档/四产物通过；双宽4例；24 scratch文件回收)
- Open task: T13 S1；T12/T11/T9/T8关闭；T10取消；Td S9保持关闭

## Technical Baseline

0.13.0.0 / M0 T13 S1本地验证并双发布；三组件四文件、x64 Console、双宽DLL、ABI3保持。
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

## M0 T13 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New; 调查后用户明确批准产品修复，下一未使用T13从S1开始 |
| Admission And Approval | 用户批准实现和完整交付；后续明确要求立即修正占用规范并关闭已识别GlassCMD继续发布，覆盖本S原不改rules限制 |
| Objective | 不适合System变体的字符集/实际文本保留应用字体；动态文字/字体更新重判，保护原度量 |
| Non-goals | 不改标题/菜单/滚动条策略、不造本地语言字体、不改兄弟仓库/全局设置/原资源 |
| Reference Baseline | ae9a1b5；0.12.0.0，准入工作树干净；Td S10已证明中文墨迹缩小 |
| Candidate Proposal | [多语言控件字体保护](../proposals/leaderboard-font.md) |
| Files And ABI Surface | font.c/h、controls.c、原生字体回归及构建/包入口，版本0.13.0；ABI3和四产物保持 |
| Applicable Rules | 全Task Reading Set及source-policy已读；纯C17/GCC/Ninja/ccache；用户本轮明确追加授权修正EXECUTION占用处置，不据此扩张其他规则 |
| Verification | 双宽原生中日韩/希腊/西里尔/阿拉伯/混合/组合符，字体及动态文本/编辑/列表/下拉；负例旧DLL；既有字体/菜单/标题及完整相关回归 |
| Expected Markers | MULTILINGUAL FONT PASS、FONT LAYOUT PASS、MENU CANCEL PASS；逐项原字体/像素/advance/卸载/GDI证据 |
| Asset Needs | 不新增产品资源；保留六原文件；自有窗口原生截图用于验收 |
| Reporting Requirements | 报告实际覆盖/失败/跳过/负例；源码差异统计、同批四文件测试、双位置8摘要、提交及实际远端 |
| Stop Conditions | 资源/ABI变化、未定位测试失败、系统部署占用则识别实际占用者，依用户本轮明确授权关闭并继续，不关闭无关进程；超预算文本保守原字体 |
| Exit Criteria | 根因修复与回归、干净双宽构建及实际包测试、assets根及system32双发布、远端复核、scratch提炼回收；审计后关闭本S/T |
| Original Owner Request | 按已讨论多语言字体保护方案执行，后续“老规矩”沿用完整交付规范 |
| Similar-Issue Sweep | 搜索WM_SETFONT/WM_SETTEXT、EDIT变更、LB/CB内容及源字体覆盖；只改控件，保持菜单和标题生成路径 |

scratch：build/t13-font（探索/负例/原生截图）、build/t13-package（冻结快照/构建/包测试/备份），均本S拥有；稳定build/.ninja和.ccache复用不清理。
