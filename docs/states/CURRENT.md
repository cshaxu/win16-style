# Project Status

## Current Work

T15按用户最新授权自动续S直到任务级收口；当前准入S3，保持M0且不分配新T。

## M0 T15 S3 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；在S2后顺序分配S3，不复用已关闭编号 |
| Admission And Approval | 用户批准所述S3/S4/公共控件后续分解，自动准入执行直到T15收口；永久提交推送/双发布授权保持 |
| Objective | 定位并修复HD Tune主窗漏附着，查清关闭验证失败；建立可自包含检出旧包的原生回归 |
| Non-goals | 不执行磁盘业务，不干预用户既有实例/共享宿主/全局设置；不先扩展其他控件或改已确认标题字体资源 |
| Reference Baseline | da2631f，T15 S2四文件；Td S13真实高权限主窗root_attached=0及未定位关闭失败 |
| Candidate Proposal | [实现分解](../proposals/framework-completion.md) |
| Files And ABI Surface | 私有创建scope早显修复；window-aware菜单栏快照保留应用owner-draw测绘回调及重入销毁；隐藏控件的瞬时重绘保护及双宽C测试；三组件四产物/ABI3/31 detours/32 slots不变 |
| Applicable Rules | EXECUTION、ARCHITECTURE、CODING、DOCUMENT、UI；纯C17/GCC/Ninja/ccache，不使用computer-use |
| Verification | 首先原生property-sheet/对话框创建回归与旧包负例，再双宽focused/full实际包、真实HD Tune正常显示/标题/菜单/关闭及原应用回归 |
| Expected Markers | 每种A/W、modal/modeless、双宽及异常返回独立状态；主窗附着和像素须同时验证；清理区分正常/强制 |
| Asset Needs | build/t15-framework、build/t15-package登记原始探针/日志/包备份，先提炼证据后安全回收；稳定Ninja/ccache保留 |
| Reporting Requirements | 实测首个断点/最小方案、保留状态/输入、负例/覆盖/失败、双发布摘要、提交和实际远端复核；自动续S |
| Stop Conditions | 越过权限/共享宿主或重大架构边界时保留证据；不得以弱化测试/误报附着掩盖失败 |
| Exit Criteria | 本S有限修复/回归/同批四文件实际测试/双发布/提交推送及实际复核，再续S4；最终T15另审计覆盖全集 |
| Original Owner Request | “批准 自动准入s任务 到t收口为止”，对应先主窗和关闭、基础控件、再逐类公共控件支持方案 |
| Similar-Issue Sweep | 对照CreateWindow/CreateDialog/MessageBox/PropertySheet创建初始化边界、ANSI/Unicode和modal/modeless；不凭类名猜根因 |

## S3 Verification Progress

170输入冻结/干净双宽/来源资源/ABI3，41实际包352.435秒（4槽、无skip）、12聚焦、
真实HD Tune/DK/MadShow通过。HD Tune主窗attached1；三个原隐藏按钮在显示和恢复后
均隐藏，菜单打开/截图/取消正常，原生关闭2与裸对照一致、Job空；提权探针已退出。
原资源、标题/字体/菜单/滚动条保持；已双发布八摘要一致，两处真实双宽启动及ABI复验通过。
P1待提交推送/实际远端复核；S3尚未关闭，之后自动准入S4。其他控件未称已支持。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 13
- Owner allocation constraint: 只可自行划分后续S，不得自行分配新T；保持现有T15/M0。
- Last delivery: M0 Td S13 P2 (P1 3d43ff7实际取回10文件通过，HD Tune有限调查关闭；产品仍为T15 S2)
- Open task: T15自动续S实施/任务级审计中；S3活动，S2/S1关闭；Td S13/S12/S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.15.0.0 / T15 S3四文件已本地验证、双发布及安装复验，P1提交/远端复核待完成。
170输入、C17/GCC16.1/Ninja/ccache干净双宽，41实际包352.435秒/12聚焦无skip；
真实HD Tune主窗/隐藏按钮/菜单取消及两款旧应用保留有证据。84注册CTest并非全部执行。
三组件四产物/ABI3/31导出及31 detours/32slots不变，原五资源头/声明和18嵌入表保持。
支持面及保留边界见[本S记录](../history/M0-T15-S3-dialog-menus.md)和
[精选证据](../etc/evidence/framework-completion.json)。T15/M0开放，继续S4控件适配；
T15 S1/S2成果、T14 S3启动语义及其他任务/债务状态不变。

## Recent Closure

- [Td S13 HD Tune Pro调查](../history/M0-Td-S13-hdtune-controls.md)：经批准UAC取得正常显示对照，主窗未附着/标准控件支持缺口明确；关闭检查失败如实保留，无产品修复；P1远端10文件通过，scratch回收，S13关闭。

- [Td S12 MD5summer控件调查](../history/M0-Td-S12-md5summer-controls.md)：原生双对照/4自有实例通过，主窗和内部Edit有效；框架类识别缺口/TreeView边界已明，无产品改动；P1远端10文件通过，scratch回收，S12关闭。

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
