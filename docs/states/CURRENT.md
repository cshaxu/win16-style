# Project Status

## Current Work

M0 T14 S2 active：按用户要求清理S1多余的旧MZ/NE二次Shell启动。
准入48b5715，工作树干净；恢复首次校验失败的stderr/193/退出4，保留UAC和样式。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 10
- Last delivery: M0 T14 S1 P2 (P1 91bba9f远端153输入/四产物及八部署摘要通过；两登记scratch回收)
- Open task: T14 S2；T14重新开放；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.14.0.0 / M0 T14 S2已验证及双发布，P1实际远端复核待完成；T14固定版本以Git/SHA区分S2。
已删除旧MZ/NE二次Shell入口/模块和专用UI测试，首次main恢复97999c0字节。
MZ/NE仅stderr193/退出4，无额外弹窗/暂停；UAC真实740分支及全部DLL/demo源保持。
150输入独立冻结/双宽干净构建，实际包24/24，旧重试版被新回归检出。
新回归覆盖MZ/NE、直接/真实cmd四场景、一次193、无额外UI、所有映像身份及整Job归零；
系统conhost作为已识别OS宿主计入，不误判为目标重试。注册70项未全跑。
八部署摘要一致，system32已安装启动器四场景及双宽ABI通过。此前用户未看到首次报错
的历史原因未复现，不声称物理Win+R持久可见。见[清理记录](../history/M0-T14-S2-cleanup.md)。

## Recent Closure

- [T14 S2清理二次启动](../history/M0-T14-S2-cleanup.md)：实际包24/双发布/安装验证通过，P1实际远端复核待完成。

- [T14系统启动错误体验（已撤回）](../history/M0-T14-S1-system-shell.md)：24实际包/双发布和安装入口通过，91bba9f远端153输入/四产物、168文件复核通过，两登记scratch已回收；S1/T14收口。

- [T13 S2原样撤回](../history/M0-T13-S2-revert.md)：原样撤回、26聚焦/23实际包及双发布通过，bcf3d08实际远端复核通过，S2/T13收口。

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

## Last Packet Closure

M0 T14 S1完成Win+R/cmd同名NE/PE差异诊断及用户明确要求的真实系统错误体验。
153输入独立冻结，双宽干净构建、两宽新单元、24实际包及已部署系统UI/ABI通过；
旧版负例有效，IOCP整Job清理保持严格归零。P1 91bba9f已实际远端复核，八摘要一致。
两登记scratch已提炼回收。T14仅S1，全任务按本轮有限支持范围审计通过；
S1/T14关闭，M0开放；既有PE/UAC/样式/资源与其他任务/TODO状态保持。

## M0 T14 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-Reopen; 已关闭T14追加下一S2，不复用S1或新分配T |
| Admission And Approval | 用户认为二次启动多余，指出首次报错缺失，明确“对没错，请你清理”；沿用完整发布及提交推送授权 |
| Objective | 删除S1旧映像Shell二次尝试及专用模块/测试/说明，恢复首次失败路径并实测stderr |
| Non-goals | 不增加自制或系统补充弹窗，不暂停控制台、不改搜索顺序/UAC/字体/菜单/资源，不修改兄弟仓库 |
| Reference Baseline | 48b5715，准入干净；首次main逻辑参考97999c0，T14版本保持0.14.0.0以符合固定版本契约 |
| Candidate Proposal | [启动差异及清理](../proposals/run-dialog-launch.md) |
| Files And ABI Surface | 删除legacy_shell.c/h和两个专用测试，main/CMake/Test-Package撤回入口；新增纯C首次拒绝回归，无ABI/产品数变化 |
| Applicable Rules | 全Task Reading Set；纯C17/GCC/Ninja/ccache；主动处理已核实发布占用；无computer-use |
| Verification | 自有隔离原生探针覆盖MZ/NE、直接及真实cmd启动、stderr仅一次193/退出4、无额外UI和精确Job进程数；实际包及双宽构建 |
| Expected Markers | LEGACY REJECTION PASS 4 cases；实际包24/24，8发布摘要一致；原main与97999c0相同 |
| Asset Needs | 无新资源，重建同批四文件并双发布assets根和system32 |
| Reporting Requirements | 删除清单/新stderr证据/旧版负例/实际包/发布/实际远端；未复现原“首次无报错”原因不声称解决 |
| Stop Conditions | 若需要系统设置或既有进程注入来补提示则停止该方案；仅清理本S范围 |
| Exit Criteria | 多余机制和现行说明已移除，首次错误验证、双宽构建/包测/双发布/提交推送及实际复核，scratch提炼回收 |
| Original Owner Request | “MZ/NE没必要做第二次启动”“之前是第一次启动连命令行报错都没有”“对没错，请你清理” |
| Similar-Issue Sweep | 搜索legacy_shell/shell-errors/shell-unit及ShellExecuteEx全部入口；只保留真实740的既有UAC分支 |

scratch：build/t14-s2-cleanup（原生观测/负例/精选证据）、build/t14-package（仅本S s2-*快照/构建/测试/发布备份）；S2提炼后回收，稳定Ninja/ccache保留。
