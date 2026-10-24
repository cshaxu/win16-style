# Project Status

## Current Work

无活动任务包。M0 Td S11 MadShow调查已完成并实际远端复核收口，M0开放。
已证实VB6类名单兼容缺口、client-edge门槛及无窗口标签边界；仅调研，无产品变更。
后续VB6控件适配列为未编号候选，尚未准入实施；T14及其他任务/债务状态保持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 14
- Last allocated Td S: 11
- Last delivery: M0 Td S11 P2 (P1 bb59d0a实际远端六文档通过；154产品输入/八部署未变；登记scratch已回收)
- Open task: none；Td S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.14.0.0 / T14 S3已验证/双发布/实际远端复核并收口；固定T14版本，由Git/SHA区分S3。
154输入冻结、GCC16.1/Ninja/ccache干净双宽构建，27/27实际包253.77秒通过，
双宽新单元/聚焦通过。MZ/NE系统Shell对照、独占/共享cmd/显式隐藏及无窗口共12路径；
系统UI一次且stderr空，其他一次193无UI，退出4，精确映像/内核Job归零。
S2无UI和S1双错误通道旧包均被检出；80份DLL/demo/UAC源逐字节保留。
两处八摘要一致，已安装启动器12路径及双宽ABI3/31通过；原图形/声明/18表保留。
73注册CTest未全部执行；物理Win+R/所有终端及历史两框来源不作已覆盖声明。
新控制台创建瞬间仍可能短暂闪现，错误UI由Windows持有。详见
[S3记录](../history/M0-T14-S3-console-errors.md)及[证据](../etc/evidence/console-error-routing.json)。

## Recent Closure

- [Td S11 MadShow控件调查](../history/M0-Td-S11-madshow-controls.md)：3有效对照/6自有实例，VB6的19控件分类及13个client-edge断点、无窗口标签边界已明；无产品改动；bb59d0a远端六文档通过，S11关闭。

- [T14 S3条件错误路由](../history/M0-T14-S3-console-errors.md)：27实际包/双发布/安装入口及47785d1实际远端复核通过；S3/T14关闭，M0开放。

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

M0 Td S11纯调查完成：本机MadShow为x86 VB6，实际DLL/四个入口detour已生效、
主窗附着；4按钮/13文本框/2分组仍被标准类名单跳过，13个client-edge为第二门槛，
标签无独立HWND。有限按钮状态往返/原生截图及注册模块身份已查，未伪称已实现适配。
3有效批次/6自有实例均正常退出、Job0；P1 bb59d0a实际取回六份文档通过，154产品
输入及八部署摘要未变，无产品构建/发布。证据提炼后精确登记scratch已安全回收，
提升只读进程预检无存活探针，稳定Ninja/ccache保留；治理自测通过。
Td S11关闭，M0开放；产品T14等状态保持；VB6适配在QUEUE候选等待后续用户准入。
