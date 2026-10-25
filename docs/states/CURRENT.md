# Project Status

## Current Work

S7普通报告列表文字已实现：保留真实DK custom-draw字体/颜色/返回值，经准确绘制DC
暂借现有保细笔画System字形；11px E/8实际可读，标准控件/标题/菜单及原资源不变。
最终同批四文件69/69实际包、双发布八摘要、安装12聚焦+2首次注入矩阵和真实DK/MD5/VB
通过。旧包4负例及丢笔画原型4负例均检出。修正过x86测试未初始化rc假设，失败不隐去。
待实际推送/远端/归档复核与scratch安全清理后才关闭S7；M0/T15保持开放，HD Tune提权
实测默认skip、不计通过。[S7记录](../history/M0-T15-S7-report-list.md)和
[证据](../etc/evidence/report-list.json)保留有限范围及未覆盖面。

## M0 T15 S7 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S6关闭后顺序准入S7，唯一活动S，不复用编号 |
| Admission And Approval | 用户“批准 自动准入s任务 到t收口为止”；永久提交推送/双发布授权保持；HD Tune管理员实测依最新要求默认skip，除非另行恢复 |
| Objective | 核实SysListView32/TListView报告行的公开协议、native/custom-draw阶段及应用已有回调，先原型验证再有限接入现有renderer；保留数据/图标/选中/焦点/几何/原生通知，过大自动拆续S |
| Non-goals | 不把既有边框当行绘制，不猜VCL对象或私有lParam，不任意填充真实应用的私有行对象；不改标题/字体生成/菜单/滚动条/原资源/启动，不实现树/进度/滑块/微调/任意自绘或虚拟数据模式，不做HD Tune提权实测 |
| Reference Baseline | S6 P1 937fab06eec57c6ffd458bca18a45d3d51ca1a33四文件0.15.0.0已双发布；P2 82e9135ea612f0abd508035ce0facec28a3af3fd严格回执修正/远端192通过；183输入/65包/八部署保持 |
| Candidate Proposal | [框架完成分解](../proposals/framework-completion.md)；[S6记录](../history/M0-T15-S6-header-controls.md)/[证据](../etc/evidence/header-controls.json)含列表/表头真实交接 |
| Files And ABI Surface | 私有列表角色/绘制接入/受控父通知观察器与C17测试；保持三组件四产物、ABI3/31导出/现有detour容量，无额外运行产品/兄弟仓库依赖；新增内部hook先审 |
| Applicable Rules | EXECUTION、ARCHITECTURE、CODING、DOCUMENT、UI及来源策略；MinGW GCC/Ninja/稳定ccache；禁computer-use与兄弟仓库修改/依赖 |
| Verification | 先原生双宽/COMCTL/DPI与真实自有DK/MD5只读绘制观察，区分应用自绘；自包含填充行/选中/图标/通知及应用回调对照、旧包负例/生命周期；双宽干净四文件实际包/双发布/安装/实际远端归档；HD Tune实测skip |
| Expected Markers | 正常显示和填充行实际字形/颜色/图标/原生几何；应用custom-draw原返回与额外阶段边界、命令/数据/选中/焦点/回调销毁、DC/字体/观察器释放；实际目标DLL路径/摘要 |
| Asset Needs | build/t15-controls登记S7原型/日志并承接七份已审源码；build/t15-package登记新干净包/备份；先提炼后清理，稳定.ninja/.ccache和外部PNG保留 |
| Reporting Requirements | 先报告实际断点和验证过的有限路线，明确应用自绘/虚拟/特殊视图保留；包/双发布/远端协调复核和未覆盖面，不用空列表或附着代替有内容绘制证明 |
| Stop Conditions | 无法保留应用回调/数据或需私有对象、共享宿主/既有进程注入/全局设置时保留证据与方案；不吞通知/改源图/放宽门禁；未验证模式维持原生并明确交接 |
| Exit Criteria | 可证有限报告行接入或有证据的进一步分解；对应双宽构建/同批四文件实际包/必要双发布与八摘要/安装复验、提交推送/实际远端/协调复核和清理后关闭S7；自动续S，T15另做有限覆盖审计 |
| Original Owner Request | “好 怎么办吧”“批准 自动准入s任务 到t收口为止”；传统程序框架/公共控件适配；“跳过这个测试 以后默认不测” |
| Similar-Issue Sweep | 原生SysListView32与真实TListView、报告/非报告/ownerdraw/ownerdata、父custom-draw返回和额外阶段、图像/状态图/子项/编辑与隐藏、ANSI/Unicode、回调变更/销毁/独立所有权及借用DC/字体 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 13
- Owner allocation constraint: 只可自行划分后续S，不得自行分配新T；保持现有T15/M0。
- Last delivery: M0 T15 S7 P1（本地69实际包/双发布/安装及真实应用通过；实际推送/远端与归档复核待完成，不提前关闭S7）
- Open task: T15自动续S实施/任务级审计中；S7活动，S6/S5/S4/S3/S2/S1关闭；Td S13/S12/S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.15.0.0 / S7普通报告列表文字四文件已双发布，184输入、69实际包406.962秒、安装12
聚焦+2启动矩阵及真实DK/MD5/VB通过；旧包4负例/丢笔画原型4负例有效。报告行仅新建
已拥有的初始化列表和同步paint/print准确DC，保留原生数据/图像/字体属性/列几何/
custom-draw；新列表复用现有保笔画租约，标题/字体生成/菜单/滚动条/原五头/声明/
18表、三组件四产物/ABI3/31导出/31detours32slots不变。当前P1仍待实际远端/归档
复核与安全清理，不关闭S7/T15；HD Tune提权默认skip。详情见[S7记录](../history/M0-T15-S7-report-list.md)
及[证据](../etc/evidence/report-list.json)。S6已关闭，其他任务状态和改动保持。

## Recent Closure

- [T15 S6普通表头](../history/M0-T15-S6-header-controls.md)：65实际包/双发布及P1远端195，P2严格回执修正后远端192/归档重建12、安全清理通过；HD Tune提权skip，关闭S6/自动续S7，M0/T15开放。

- [T15 S5分页/回调所有权](../history/M0-T15-S5-tab-controls.md)：55实际包/双发布、远端193/严格归档重建12、八摘要/安装及安全清理通过；HD Tune提权按用户要求skip，S5关闭、自动续S6，M0/T15开放。

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

S6 P1 937fab06完成普通文字表头及特殊格式原生保留；65实际包/10聚焦/12保留、真实
DK/MD5/VB、八部署/安装通过，远端195匹配。严格归档查出新测试工作区CRLF回执，
P2 82e9135只修正原Git LF身份，导出源双宽重建12、实际P2严格归档/远端192通过；
已测四文件不变。183输入/八当前/八旧备份可恢复，精确两root无reparse/活exe0、
67DLL占用0，提炼证据后清理，仅七份观察器源码转S7；稳定缓存/外部PNG保持。
S6关闭、本P准入S7报告行研究/有限适配，M0/T15及其他任务保持，HD Tune管理员实测默认skip。
