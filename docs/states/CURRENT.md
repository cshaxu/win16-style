# Project Status

## Current Work

M0 T16 S1有限局部轮廓移动已实现；0.16.0.0完整99实际包/经典24/安装18通过，
已按同批四文件双发布、八SHA一致。当前进行P1提交推送、实际远端、任务审计
及恢复/清理闭环；S1/T16仍开放，M0固定，不启动其他候选。

## M0 T16 S1 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | New；用户明确准入队列首位实现，顺序分配T16 S1，M0固定；必要后续S顺序自动拆分直到本T可验收 |
| Admission And Approval | 用户“准入队列首位T任务开始执行到老规矩验收”；永久提交推送/双发布授权；不启动其余候选，HD Tune提权默认skip |
| Objective | 核实Win3.x原始USER/GDI/VGA移动轮廓依据，冻结可复现栅格/交互规范，再完成已附着窗口的有限局部轮廓移动并回归、发布供验收 |
| Non-goals | 不猜XOR/画刷/线宽效果，不修改全局拖动/主题设置，不全局hook/注入既有进程，不改原资源及已确认标题字体菜单滚动条，不扩张为全部缩放/MDI/吸附/任意自绘拖动 |
| Reference Baseline | 96e47212da877bcb4e16178abea093b006bb343f；现部署0.15.0.0/197输入/三组件四产物/ABI3/35detours40slots；T15及Td S14保持关闭 |
| Candidate Proposal | [Win3.x局部轮廓拖动](../proposals/outline-window-move.md)，正式研究/实现brief；[既有界面边界](../design/UI.md) |
| Files And ABI Surface | 研究后审查win16_style及私有移动模块/原生C测试；审查新增DefWindowProcA/W进程内私有拦截，由应用已进入默认过程后处理SC_MOVE，保留应用否决；35增至37detours/40slots，无公开导出/组件/第五产品；对话框/被动显式API保留原生移动 |
| Applicable Rules | 完整Task Reading Set、ARCHITECTURE/CODING/UI及来源策略；C17/GCC/Ninja/ccache；禁computer-use和兄弟修改/依赖 |
| Verification | 原二进制版本/路径/SHA/段偏移/调用及原像素证据；双宽自有原生探针对照移动中真窗口不动、提交/取消/捕获丢失/销毁/重入/DPI及资源；旧包负例、既有标题/字体/菜单/滚动条及双宽传播；每S干净同批四文件实际包/双发布/八摘要/实际远端复核 |
| Expected Markers | 先有原始实现依据与独立oracle，再有真实显示/轮廓截图及窗口矩形/消息/API/资源证明；未知模式明确原生回退，不冒称物理输入 |
| Asset Needs | 登记build/t16-outline原始研究副本/探针/日志与build/t16-package干净包/备份；验收PNG保存C:/Users/neko/.codex/visualizations/2026/10/23/01a1699f-9927-7fc3-ab34-4b38128ff811/win16style-t16-s1；不复制原系统二进制进Git；既有t15-controls49源码/稳定缓存/外部PNG保持；本T证据提炼后检查精确根/reparse/映像占用并安全清理 |
| Reporting Requirements | 先报告原始依据/最小实现/有限覆盖，再实现；区分现代API近似与原始二进制事实；版本/四文件SHA/实际包/原生截图/跳过未覆盖及任务审计完整交付 |
| Stop Conditions | 无合法可用原始实现依据时明确报告缺口，不用截图猜测；需全局设置/共享宿主/既有进程注入或额外产品时停在证据与方案；重大边界不暗改 |
| Exit Criteria | 原始证据足够、有限移动适配及全部本S验证/双发布/推送远端/安全清理闭环后关闭S；本T另冻结支持矩阵审计，完成则停待用户验收，M0保持开放 |
| Original Owner Request | 准入队列首位T任务开始执行到老规矩验收；原proposal要求效果依据Win3.x原始GDI/USER/VGA二进制，不自己猜 |
| Similar-Issue Sweep | 标题鼠标/SC_MOVE/键盘入口、WM_ENTERSIZEMOVE/MOVING/EXITSIZEMOVE、嵌套重入、捕获/取消/失活/终结、DPI多屏/约束、未附着窗口及系统设置保持 |

## 当前进度与恢复收口

用户最新要求当前拖拽T收口，原生桌面Default，已恢复完整显示验证。最终包
四显示配置共256场景，厚/细、A/W、actual144/logical96，独立GDI/原生定位对照、
取消/捕获/销毁/重入/资源均通过。新旧两宽同位置整图各614400像素RGB0差异。
200输入已冻结，原五头/声明/18表保持。源码和测试结论由[交付审计](../history/M0-T16-S1-outline-move.md)
及[证据](../etc/evidence/outline-delivery.json)接收；此前失败保留在验证登记/TODO，
未修复T11 Job计数债务，不抹掉历史。所有当前自有显示测试已退出。
Actor只读诊断和用户要求的VB5/VB6专项已列队列第3位，未夹带控件修复或预占T/S。
完成实际远端、归档/恢复及精确根清理后才关闭；验收PNG在已登记外部目录保留。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 16
- Last allocated Td S: 14
- Owner allocation constraint: 本次用户明确准入T16；仅可自动细分其必要后续S，不自行分配下一T，其他候选待准入。
- Last delivery: M0 Td S14 P2（三项候选与队列登记；治理13负例/源197和八部署不变/P1实际远端207文件通过；仅文档收口）
- Open task: T16 S1原始依据核实及有限轮廓移动实现；Td S14候选文档登记关闭；T15 S12/S11/S10/S9/S8/S7/S6/S5/S4/S3/S2/S1及T15关闭，M0开放等待用户验收；Td S13/S12/S11、T14 S3/T14、T13/T12/T11/T9/T8及Td S9/S10保持关闭；T10取消。

## Technical Baseline

0.16.0.0 / M0 T16 S1，200产品输入、三组件四产物、ABI3/31命名+ordinal1，
37进程内detours/40slots。干净双宽、99实际包、24经典及18安装复核、双发布八SHA
一致；原五头/声明和每DLL18表不变。标题/字体/菜单/滚动条算法保持，同场景旧/新
双宽整图RGB0差异。纯C17/GCC/Ninja/ccache；有限普通默认过程轮廓移动，回退与
未覆盖见[UI](../design/UI.md)。源码快照及四SHA由[release](../etc/release.json)承载。
P1提交推送/实际远端与最后恢复清理尚待执行；不将本阶段标记为关闭，M0开放。

## Recent Closure

- [Td S14候选队列登记](../history/M0-Td-S14-candidate-queue.md)：轮廓拖动/对话框/全局模式按1/2/3登记；治理13负例、197产品输入及八部署不变、P1实际远端207文件通过；仅文档收口，不准入候选实现。

- [T15 S12冻结全集审计](../history/M0-T15-S12-framework-audit.md)：18单位明确处置，独立91包/14安装、八部署/真实MD5-DK、实际远端207/197输入、归档双DLL+4/201对象恢复及258映像清理通过；49源码保留，S12/T15关闭，M0开放待用户验收。

- [T15 S11原生微调](../history/M0-T15-S11-updown-controls.md)：91实际包/38安装、八部署、远端210/197输入、归档双DLL+4/201对象恢复及277映像安全清理通过；S11关闭/续S12冻结全集审计，M0/T15开放。

- [T15 S10滑块/树并发](../history/M0-T15-S10-trackbar-controls.md)：83包/八部署/安装28+2、真实MD5/DK、远端205/受影响双DLL归档重建4、197对象恢复与588映像清理通过；S10关闭/自动续S11，M0/T15开放。

- [T15 S9进度/受控输入](../history/M0-T15-S9-progress-controls.md)：79包/八部署/安装20+2、真实MD5/DK/标题掩码、远端203/受影响双DLL归档重建4及260映像清理通过；S9关闭/自动续S10，M0/T15开放。

- [T15 S8原生树/早显frame](../history/M0-T15-S8-tree-controls.md)：75包/八部署/安装16+2、实际MD5/DK、远端201/归档串行10及377映像清理通过；原生键盘偶发交S9，S8关闭/自动续S9，M0/T15开放。

- [T15 S7报告列表](../history/M0-T15-S7-report-list.md)：69包/八部署、安装12+2及真实DK/MD5/VB、远端195/严格归档重建12、739映像安全清理通过，S7关闭；自动续S8，M0/T15开放，HD Tune提权skip。

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

Td S14将三份候选及队列1/2/3完整提交。P1 6f77f08实际远端207文件/197输入匹配，
读取实际diff/队列及完整提案复核通过；治理SelfTest13负例/diff检查通过。原两份
候选工作区内容原样保留；本S未改产品/ABI/资源/二进制，197源与八部署摘要保持。
P2关闭文档S；T15保持关闭，M0开放，无活动包或新数值T，不自动启动队列候选。
