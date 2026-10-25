# Project Status

## Current Work

M0 T15 S2 verified/published：DoubleKiller原生超类适配，35项实际包及10项双宽聚焦通过，
真实DoubleKiller/MadShow及两处默认安装入口复验通过；待提交推送、实际远端复核及scratch回收。
用户仅授权自行划分S，不自行分配新T。T15 S1保持关闭，T15/M0开放等待外观验收。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 11
- Owner allocation constraint: 只可自行划分后续S，不得自行分配新T；保持现有T15/M0。
- Last delivery: M0 T15 S1 P2 (P1 6246df6实际取回177文件通过；163输入/八部署一致；登记scratch已回收)
- Open task: T15 S2 active；T15等待用户外观验收；S1关闭；Td S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.15.0.0 / T15 S2本地已验证/双发布，尚待实际远端复核，不作关闭声明。
167输入冻结；纯C17/GCC16.1、Ninja/ccache干净双宽构建；35/35实际包260.64秒，
另10项双宽聚焦、真实DoubleKiller/MadShow、两处默认启动及系统双宽ABI通过，无skip。
79注册CTest未全部执行。DoubleKiller四页28按钮(含复选)/3编辑/2下拉共33控件附着；
Run真实填充c0c0c0，卸载f0f0f0，1843像素变化，关闭exit0/自有Job空。
标题顶部24行逐像素不变，客户区29509像素变化。自绘分页/列表/标签/微调等保持原应用。
暂存主题不更改InitWndProc，原生初始化后提交；生产主题分配配对归零、失败/取消/终结/
最新主题恢复通过。旧包双宽和“附着假阳性”原型均被强化测试检出，未发布错误原型。
八部署摘要一致；ABI3/31、无新增hook/导出，原五头/声明/18表及三组件四产物保持。
范围见[本S记录](../history/M0-T15-S2-doublekiller.md)和[证据](../etc/evidence/doublekiller-controls.json)。
T15 S1的VB6成果、T14 S3启动语义及既有标题/字体/菜单/滚动条保持；其他任务/债务状态不变。

## Recent Closure

- [T15 S1 VB6适配](../history/M0-T15-S1-vb6.md)：31实际包/双发布/安装复验及P1远端177文件通过，scratch安全回收；S1关闭，T15等待用户外观验收。

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

T15 S1完成MadShow按钮/文本框/分组与窗口less文字适配；目标19控件全部附着，
真实截图、消息、双宽/DPI协议、私有资源释放及旧包/旧采样负例通过。标准标题/
字体/菜单/滚动条、原图形声明和架构保持。31实际包250.62秒及10聚焦通过；
两处八摘要一致、安装ABI/协议/真实程序复验通过。P1 6246df6实际取回163输入、
四产物及变更文档共177文件，协调复核通过；源输入/部署摘要保持。证据提炼后
两登记scratch按绝对路径/无活进程/无reparse检查回收，稳定Ninja/ccache和验收
PNG保留。S1关闭，T15等待用户外观验收，M0开放；其他任务/债务状态保持。

## M0 T15 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation |
| Admission And Approval | 用户明确禁止自行分配新T，只可自行分S；本次沿用T15的S2，撤销未提交的T16误分配。用户请求让DoubleKiller应用本项目样式；允许范围内调查、原型、实现和完整交付，不关闭其他开放任务。 |
| Objective | 定位本机DoubleKiller内部控件未换肤原因，以精确框架契约适配可验证控件。 |
| Non-goals | 不进行重复文件扫描/删除/移动；不承诺任意自绘框架；不改标题、菜单、标准字体策略、原资源或兄弟仓库。 |
| Reference Baseline | 8fc6ad0 / 0.15.0.0，干净工作区；T12启动修复及T15框架适配保持。 |
| Candidate Proposal | [DoubleKiller控件](../proposals/doublekiller-controls.md) |
| Files And ABI Surface | DLL私有分类/绘制及原生C测试、构建元数据、来源回执和交付文档；预期ABI3/31及三组件四产物保持。 |
| Applicable Rules | 本次Task Reading Set全部已读；EXECUTION/ARCHITECTURE/CODING/DOCUMENT及UI/来源规范。 |
| Verification | 自有Job/私有桌面裸与注入对照、真实类/注册者/附着/绘制证据；自包含双宽正负例/消息/清理；干净双宽同包回归、双发布、实际远端复核。 |
| Expected Markers | DK_CLASSIFIED、DK_RENDER_OK、DK_CLEANUP_OK，有限等待/非零失败；未覆盖不计通过。 |
| Asset Needs | 五原始图形头及声明字节保持；截图原生生成；scratch仅build/t15-doublekiller、build/t15-package，执行者负责提炼后安全回收；稳定.ninja/.ccache保留。 |
| Reporting Requirements | 实际断点、最小修复、类别覆盖、负例、资源释放、包SHA/测试/发布/远端事实；其他任务改动保留。 |
| Stop Conditions | 需私有对象布局猜测、额外产品、共享/既有进程注入、安全边界调整时停在证据与方案；不能完整验证则如实记录。 |
| Exit Criteria | 本次有限控件闭环、经典/VB回归、35项实际包及聚焦回归通过，四文件双发布/提交推送/实际复核；用户外观验收另列。 |
| Original Owner Request | 好 接下来看看 你能不能让doublekiller也应用我们样式。 |
| Similar-Issue Sweep | 检索分类/控件scope/自动附着三处及自绘/容器路径，逐生产命中处置；不用应用名或仅类前缀判断。 |
