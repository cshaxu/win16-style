# Project Status

## Current Work

M0 T13 S2 active：用户不满意S1外观，要求原样revert全部代码和测试。
T13按Owner-Reopen追加S2；S1历史保留。源码/测试/构建和版本回到ae9a1b5，发布占用规范保留。
M0 T12 S1已完成缺失IAT目录的DoubleKiller启动兼容、验证、双发布及
P1实际远端复核；T12收口，M0开放。T8/T9/T11关闭、T10取消的状态保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 13
- Last allocated Td S: 10
- Last delivery: M0 T13 S1 P2 (P1 83df08c远端150输入/四产物及八发布摘要通过；两登记scratch回收)
- Open task: T13 S2；T13重新开放；T12/T11/T9/T8关闭；T10取消；Td S9保持关闭

## Technical Baseline

0.12.0.0 / T13 S2恢复ae9a1b5的完整CMake/src/tests/tools内容；产品M0 T12元数据
按用户原样撤回要求恢复，治理不重用编号。三组件四文件、x64 Console、双宽DLL、ABI3保持。
S1多语言控件字体保护及新fixture已撤回，原英雄榜字体问题不再声明修复。
标题/菜单/滚动条及原图形资源回到同一既有源；主动处理发布占用规范保留。

独立干净双宽构建；149份输入冻结，8个恢复原文件字节相同、删除1新测试，
全部148份代码/测试/工具Git内容与目标一致。双宽聚焦26/26，实际包23/23通过。
注册69项不等于本次全运行；未覆盖物理输入/IME、任意语言/DPI。8发布摘要一致，
system32实际双宽ABI通过；P1推送及实际远端复核待完成。见[回退记录](../history/M0-T13-S2-revert.md)。

## Recent Closure

- [T13 S2原样撤回](../history/M0-T13-S2-revert.md)：已验证/双发布，P1远端复核待完成。

- [T13多语言控件字体](../history/M0-T13-S1-multilingual-font.md)：83df08c远端150输入/四产物通过，71项68通过3跳过、实际包25/25、八发布摘要及真实扫雷通过；当时S1/T收口，后按用户要求由S2撤回。

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

## M0 T13 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-Reopen; 用户要求已关闭T13追加S2，撤回S1产品及测试，不复用S1编号 |
| Admission And Approval | 用户明确“所有代码和测试改动原样revert”；沿用构建/双发布/提交推送授权和主动处理占用规范 |
| Objective | S1的9个代码/测试/构建文件精确恢复ae9a1b5，删除新增fixture并同步撤回当前说明 |
| Non-goals | 不发明替代字体方案、不撤回发布占用规范、不修改历史提交/兄弟仓库或其他任务 |
| Reference Baseline | 当前0c89bc3，工作树准入干净；撤回83df08c的代码/测试差异，目标ae9a1b5 |
| Candidate Proposal | [字体任务及回退](../proposals/leaderboard-font.md) |
| Files And ABI Surface | CMake/controls/font及三版本RC、Test-Package、删除multilingual_font.c；README/UI/回执同步；ABI3和原资源不变 |
| Applicable Rules | 全Task Reading Set；原样revert按用户要求连版本恢复0.12，治理仍T13 S2；工具Task12仅校验元数据非重分配编号 |
| Verification | 全src/tests/tools/CMake逐文件Git blob/SHA等同目标；干净双宽/23项实际包；既有字体、菜单及核心生命周期回归 |
| Expected Markers | REVERT IDENTITY PASS、FONT LAYOUT PASS、MENU CANCEL PASS、实际包23/23；不存在新增测试注册 |
| Asset Needs | 无新产品资源；同批四文件重建后双发布，原资源6文件字节不变 |
| Reporting Requirements | 原样恢复清单/摘要、实际包结果、发布8摘要、版本回退例外及实际远端复核；旧S1结果仅作历史 |
| Stop Conditions | 版本或源身份不一致先纠正；占用按已授权规范核实并关闭后继续，不结束无关进程 |
| Exit Criteria | 原样撤回证明、编译/包测/双发布/推送/远端复核/登记scratch回收；S2及本T收口，M0开放 |
| Original Owner Request | “追加一个S任务把刚才S任务的所有代码和测试改动原样revert” |
| Similar-Issue Sweep | 对照ae9a1b5..83df08c全部CMake/src/tests/tools差异及S1/P2，相关当前文档撤回，规则/历史单独保留 |

scratch：build/t13-s2-revert（身份/验证/占用证据）及build/t12-package（本S独有t13-s2-*命名的快照、构建、包测试和部署备份）；S2负责提炼后回收。稳定Ninja/ccache保留。
