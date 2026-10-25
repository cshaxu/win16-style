# Project Status

## Current Work

本轮为M0 Td S12只读兼容调查；保留T15开放和S1/S2成果。不改产品输入，不预先承诺适配。

## M0 Td S12 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Governance；仅调查和证据文档，不分配新T |
| Admission And Approval | 用户要求核实同目录MD5 Summer样式未完全生效的原因；原生探针自有实例 |
| Objective | 识别本机md5summer.exe框架、控件协议、加载/附着/绘制差异并给出有限路线 |
| Non-goals | 不改产品代码/输入、系统设置或应用文件；不扫描文件、不生成校验文件、不操作用户实例 |
| Reference Baseline | a13aa6e，T15 S2四产物；主线程工作区干净 |
| Candidate Proposal | [调查说明](../proposals/md5summer-controls.md) |
| Files And ABI Surface | CURRENT、QUEUE、支持证据及历史；产品/ABI/资源不变 |
| Applicable Rules | EXECUTION、ARCHITECTURE、CODING、DOCUMENT及UI；不使用computer-use |
| Verification | 纯C观察探针；裸/已发布注入两自有隔离Job，消息超时、模块身份和真实打印截图、退出清理；治理自测和远端实际复核 |
| Expected Markers | MD5_CLASSIFIED、MD5_CLEANUP_OK；区分模块/附着/像素，不宣称全部控件支持 |
| Asset Needs | build/td-s12-md-summer登记探针源码/日志/BMP，证据提炼后安全回收；验收PNG外部保留 |
| Reporting Requirements | 实际路径/SHA/位宽/框架、每类已附着和未支持面、截图与最小路线；明确未改产品未发布 |
| Stop Conditions | 非自有实例、业务写入、无法有界退出或私有框架布局猜测即停止该路径 |
| Exit Criteria | 有证据分类和建议；文档门禁、提交推送/实际远端复核；保持M0/T15开放 |
| Original Owner Request | “同目录底下还有个MD5 Summer.exe，它的样式也没有完全地apply，是不是又有别的类型的控件需要应用” |
| Similar-Issue Sweep | 对照S2原生超类名单、owner-draw拒绝和未支持common-controls；不把注册类名前缀当协议 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 12
- Owner allocation constraint: 只可自行划分后续S，不得自行分配新T；保持现有T15/M0。
- Last delivery: M0 T15 S2 P2 (P1 330fed7实际取回183文件通过；167输入/八部署一致；两登记scratch已回收)
- Open task: T15等待用户外观验收；S2/S1关闭；Td S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.15.0.0 / T15 S2已双发布/安装复验/330fed7实际远端复核并收口，T15等待用户验收。
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

- [T15 S2 DoubleKiller适配](../history/M0-T15-S2-doublekiller.md)：35实际包/10聚焦、真实双应用及两处默认安装通过，P1远端183文件通过，scratch安全回收；S2关闭，T15/M0开放等待验收。

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

T15 S2完成原生超类分类、未知子窗口主题暂存/提交及客户端边框适配；真实DoubleKiller
四页33控件附着，实际c0c0c0填充/卸载f0f0f0及消息/清理有证据。错误原型被强化像素
负例检出且未发布；自定义分页/列表/标签/微调等仍保留原绘制。原资源声明、标题/字体/
菜单/滚动条及VB6成果、三组件四产物/ABI3/31保持。35实际包260.64秒及10聚焦通过；
八部署一致，真实DoubleKiller/MadShow、两处默认入口及安装双宽ABI通过。
P1 330fed7实际取回167输入、四产物及文档共183文件，逐项匹配；协调角色读取实际
commit/diff、请求和证据通过。源输入/八部署复核保持；证据提炼后两登记scratch按
绝对路径/无存活进程/无reparse检查回收，旧备份可从8fc6ad0 Git恢复，稳定Ninja/ccache
及外部验收PNG保留。S2关闭，T15等待用户外观验收，M0开放；其他任务/债务不变。
