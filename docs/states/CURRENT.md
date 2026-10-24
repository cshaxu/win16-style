# Project Status

## Current Work

无活动任务包。M0 T14 S2已清理旧MZ/NE二次Shell尝试，完成24实际包、双发布、
安装入口验证及P1实际远端复核；S2/T14收口，M0开放。首次诊断/UAC/样式保持。
此前“首次看不到报错”的历史原因未复现，本轮不作已定位声明。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 10
- Last delivery: M0 T14 S2 P2 (P1 f08c7da远端150输入/四产物、四删除路径404及八部署摘要通过；两登记scratch回收)
- Open task: none；T14关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.14.0.0 / M0 T14 S2已验证/双发布及实际远端复核并收口；T14固定版本以Git/SHA区分S2。
已删除旧MZ/NE二次Shell入口/模块和专用UI测试，首次main恢复97999c0字节。
MZ/NE仅stderr193/退出4，无额外弹窗/暂停；UAC真实740分支及全部DLL/demo源保持。
150输入独立冻结/双宽干净构建，实际包24/24，旧重试版被新回归检出。
新回归覆盖MZ/NE、直接/真实cmd四场景、一次193、无额外UI、所有映像身份及整Job归零；
系统conhost作为已识别OS宿主计入，不误判为目标重试。注册70项未全跑。
八部署摘要一致，system32已安装启动器四场景及双宽ABI通过。此前用户未看到首次报错
的历史原因未复现，不声称物理Win+R持久可见。见[清理记录](../history/M0-T14-S2-cleanup.md)。

## Recent Closure

- [T14 S2清理二次启动](../history/M0-T14-S2-cleanup.md)：实际包24/双发布/安装验证通过，f08c7da实际远端150输入/四产物及四删除路径通过，两登记scratch回收；S2/T14收口。

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

M0 T14 S2 Owner-Reopen完成：删除4个多余源/测试文件及入口，main恢复97999c0，
新增纯C首次拒绝回归。150输入干净双宽构建、24实际包/双发布和安装入口验证通过；
f08c7da实际读取远端166文件，150输入/四产物/八部署摘要及四删除路径404通过。
全部DLL/demo/UAC及发布规范保持；两登记scratch已提炼安全回收。
T14全任务审计：S1二次Shell方案被用户撤回，S2清理交付完成；S2/T14关闭、M0开放。
历史缺失输出原因未复现，不新增弹窗/暂停，不改其他任务或TODO状态。
