# Project Status

## Current Work

S5分页原型已实现，12新增聚焦/12保留、55实际包361.218秒通过；旧包双宽四负例检出，
真实DK四页/MD5/VB正常退出且Job空、DK标题顶部24行原样。用户最新要求跳过HD Tune
管理员实测且今后默认不测；本项记skip，不计通过，不再等待或重发UAC。八部署一致、12安装ABI/分页/取消与两启动矩阵通过；本地已发布、待提交推送/实际远端复核，
S5/T15/M0保持开放，不分配新T。[本S记录](../history/M0-T15-S5-tab-controls.md)及
[证据](../etc/evidence/tab-controls.json)登记当前事实和未覆盖面。

## M0 T15 S5 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S4关闭后顺序准入S5，唯一活动S，不复用编号 |
| Admission And Approval | 用户“批准 自动准入s任务 到t收口为止”；永久提交推送/双发布授权保持；用户最新要求本次及后续默认跳过HD Tune管理员实测，不再自动弹UAC |
| Objective | 核实并适配HD Tune的SysTabControl32与DoubleKiller的TPageControl分页，保持原页面/数据/图标/选择/原生通知及输入行为；过大时明确交接自动拆续S |
| Non-goals | 本S不实现列表/表头/树/进度/滑块/微调或自绘图表；不盲按类名前缀，不猜私有对象/过程；标题/字体策略/菜单/滚动条/原资源及启动传播不改 |
| Reference Baseline | 502ec8389d20c89f80e1ee4b11a9b54ea67a39d2（严格归档/实际远端184通过）；产品5e8f880b9f893441cf61bf345218371b2c45b313四文件0.15.0.0，175输入/45包/12聚焦/八部署一致 |
| Candidate Proposal | [框架完成分解](../proposals/framework-completion.md)；[前S事实/交接](../history/M0-T15-S4-control-protocols.md)及[证据](../etc/evidence/control-protocols.json) |
| Files And ABI Surface | 唯一control renderer及私有分类/创建附着、纯C测试和可审原生探针；三组件四产物/ABI3/31导出保持，无新增产品或外部后端；新增内部hook先审容量 |
| Applicable Rules | EXECUTION、ARCHITECTURE、CODING、DOCUMENT、UI与来源策略；C17/MinGW GCC/Ninja/ccache；禁computer-use，兄弟仓库只读且不依赖 |
| Verification | 先原生只读协议/布局/绘制与有限方案，再自包含双宽裸/注入对照、旧包负例/通知及生命周期；真实DK/MD5/VB保留；HD Tune管理员实测按用户最新指令skip；双宽干净四文件实际包/双发布/实际远端 |
| Expected Markers | 正常显示标签/边框/图标/实际字形像素、Tab布局及DPI、选择/原生取消/键盘/焦点/通知数据、回调销毁/字体/资源释放；不以附着代替绘制 |
| Asset Needs | build/t15-controls承接S4七份已审观察器/复核源码并登记本S诊断；build/t15-package登记新干净包/备份；先提炼后安全清理，稳定.ninja/.ccache和外部验收PNG保留 |
| Reporting Requirements | 实测断点、原生/框架身份与初始化边界、有限方案/覆盖、旧包负例、实际包/两处发布/远端协调复核；不冒称未实现公共控件或应用页面已适配 |
| Stop Conditions | 未授权提权/共享宿主/重大架构安全边界保留证据；不得扫描注入既有进程、改系统设置、吞应用回调或放宽门禁；安全识别失败明确交接，不造成功 |
| Exit Criteria | 有限分页支持及负例/双宽回归、同批四文件实际包、binary变化双发布/八摘要/安装复验、提交推送/实际远端/协调复核及卫生后关闭S5；自动续后续S，T15另做覆盖审计 |
| Original Owner Request | “好 怎么办吧”“批准 自动准入s任务 到t收口为止”；MD5summer/HD Tune等传统程序框架及公共控件适配 |
| Similar-Issue Sweep | 标准SysTabControl32与真实超类/已验证TPageControl区别；普通/多行/按钮/图标/ownerdraw/空及隐藏控件，原生几何/通知/取消/回调生存期；未知初始化阶段不探测 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 13
- Owner allocation constraint: 只可自行划分后续S，不得自行分配新T；保持现有T15/M0。
- Last delivery: M0 T15 S4 P3（c33417117f329fa07ddeb78aa9416d82079c7873已推送/实际远端184通过；S4关闭，S5活动）
- Open task: T15自动续S实施/任务级审计中；S5活动，S4/S3/S2/S1关闭；Td S13/S12/S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.15.0.0 / T15 S4已双发布/P1远端187；P2旧回执修复后严格归档/远端184通过，S4关闭。
175输入、C17/GCC16.1/Ninja/ccache干净双宽，45实际包364.181秒/12聚焦通过；
MD5summer四角色/下拉、HD Tune84基础控件/六图标/隐藏3/菜单取消/native exit2、
DK四页及MadShow19保留；两处安装复验、八摘要一致。88注册CTest并非全部执行。
三组件四产物/ABI3/31导出及31 detours/32slots不变；原五头/声明及18表保持。
[本S记录](../history/M0-T15-S4-control-protocols.md)和[证据](../etc/evidence/control-protocols.json)
保留失败视觉候选、原生窗口DPI96及未覆盖面；175输入/八部署身份重检，旧scratch清理，
七份源码交接S5。T15/M0及其他任务/债务保持，继续分页及后续公共控件。

## Recent Closure

- [T15 S4框架角色/图像控件](../history/M0-T15-S4-control-protocols.md)：45包/12聚焦、四实际应用及八部署通过；P1远端187、P2修复旧回执后严格归档/远端184通过；安全清理/七源交接后关闭，自动续S5，T15/M0开放。

- [T15 S3对话框/菜单栏/隐藏状态](../history/M0-T15-S3-dialog-menus.md)：41实际包/12聚焦、真实HD Tune/DK/MadShow及八部署一致、P1远端184文件/协调复核通过；旧scratch安全回收，必要观察器转S4，S3关闭并自动续S4，T15/M0开放。

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

T15 S4 P1 5e8f880完成已初始化框架角色、原生图像/PushLike/client-edge适配；45包/
12聚焦及MD5四角色、HD Tune84基础控件/六图标/隐藏3/菜单/正常exit2、DK/VB保留，
双发布/安装/八摘要通过。远端187文件匹配，协调实际归档发现旧authored行尾回执；
P2 502ec83仅修正已提交LF身份，不放宽门禁/改资源；导出源码干净双宽构建/ABI和
四MZ/NE原生回归通过，实际P2严格归档/175输入等同重建证明、远端184通过。
两登记scratch绝对边界/无reparse/活exe0/110DLL占用0，八旧备份可由c5e110b恢复；
提炼证据后清除旧构建/包/日志/探针，七份必要源码留build/t15-controls转S5，稳定
Ninja/ccache及外部PNG保留。S4关闭，自动准入S5；T15/M0及其他任务状态不变。
