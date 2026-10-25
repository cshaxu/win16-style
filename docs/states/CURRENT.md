# Project Status

## Current Work

用户明确批准追加T16独立S撤回整个轮廓效果；当前M0 T16 S2 Owner-Reopen，
不复用S1，不新开T。前序短诊断已实测每更新155/178ms，主要屏幕读回和DwmFlush。
停止优化实验，原样恢复T16前产品源码和测试，恢复原生移动，保留治理/历史/队列。

## M0 T16 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Owner-Reopen；S1之后顺序S2，用户明确追加单独S撤回T16效果 |
| Admission And Approval | 用户“对。追加一个单独T16的S任务来做”；永久提交推送/双发布授权保持 |
| Objective | 原样撤回T16全部产品源码/测试及版本改动，恢复T15原生移动，保留历史及候选 |
| Non-goals | 不继续优化/重做轮廓，不改其他视觉/原资源/传播，不新开T，不重复大矩阵 |
| Reference Baseline | 当前5b10601；撤回目标96e47212da877bcb4e16178abea093b006bb343f，产品0.15.0.0 |
| Candidate Proposal | [局部轮廓拖动](../proposals/outline-window-move.md)，本次按用户撤回 |
| Files And ABI Surface | 恢复CMake/原7源及工具路径，删除move.c/h和outline_move.c，回执恢复；ABI3/35detours40slots，无新产品 |
| Applicable Rules | Task Reading Set/C17/GCC/Ninja/ccache；禁computer-use/兄弟改动；参照T13 S2原样撤回版本先例 |
| Verification | 全部CMake/src/tests/tools Git字节等同96e4721；干净双宽同包四文件及原资源门禁；仅聚焦包ABI/初始/传播/原生移动两次，不跑旧大矩阵 |
| Expected Markers | 撤回差异完整、197输入、0.15产品元数据、无DefWindowProc新增拦截/轮廓对象；实际包和八部署SHA一致 |
| Asset Needs | build/t16-perf保留短诊断/本S测试；build/t15-package仅本S withdraw-*子目录用于干净构建/包/备份，其他T15工作不动；先提炼再精确清理 |
| Reporting Requirements | 治理T16 S2与恢复产品T15版本区分；记录未跑全集、发布/推送/实际远端及未覆盖；不把撤回称性能优化 |
| Stop Conditions | 权限失败不扩大ACL；占用按既有已授权精确进程处置；不改原资源以掩盖源码/包检查 |
| Exit Criteria | 原样源码/测试撤回、双宽构建、有限实际包验证、双发布八SHA、提交远端/恢复清理后关闭S2/T16 |
| Original Owner Request | 移动边框很卡；用户决定取消整个效果并明确追加单独T16 S撤回 |
| Similar-Issue Sweep | T16 commit中全部CMake/src/tests/tools与回执、版本/UI声明；仅历史/队列证据保留，无遗漏新拦截 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 16
- Last allocated Td S: 14
- Owner allocation constraint: 本次用户明确准入T16；仅可自动细分其必要后续S，不自行分配下一T，其他候选待准入。
- Last delivery: M0 T16 S1 P2（P1实际远端218文件/200输入、归档双宽及四产物、204对象恢复、1400映像安全清理；S1/T16关闭）
- Open task: T16 S2原样撤回轮廓效果；T16重新开放，S1历史关闭保持；M0开放。

## Technical Baseline

T16 S2已撤回产品源码/测试，恢复0.15.0.0 / M0 T15元数据，197输入；治理仍为T16 S2。
全部196个CMake/src/tests/tools路径等同96e4721；原五头/声明/每DLL18表保持，三组件
四产物/ABI3/31命名+ordinal1、35私有detours/40slots。干净双宽all和同批四文件门禁
通过；实际包8项聚焦4.51992秒、两处双发布八SHA及安装ABI32/64通过。测试范围按
用户要求缩小，不冒称完整91/99全集或物理输入验收。原生定位各宽一次，轮廓功能已撤。
[release](../etc/release.json)为当前发布身份，[S2撤回](../history/M0-T16-S2-outline-withdrawal.md)
及[证据](../etc/evidence/outline-withdrawal.json)接收实际验证；P1远端/恢复清理待执行。

## Recent Closure

- [T16 S1轮廓移动与任务审计](../history/M0-T16-S1-outline-move.md)：0.16.0.0；99包/24经典/18安装、双发布八SHA、218远端文件、归档双宽/四文件及204对象恢复、1400映像清理通过；16单位有限覆盖审计完成，S1/T16关闭，M0开放。

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

M0 T16 S1 P1实际提交/推送、218远端文件取回和实际diff复核通过；16单位冻结全集
分别由实际验证、明确原生回退及未覆盖边界接收。归档双宽/四文件门禁、204对象
恢复、八部署/四PNG复核和1400映像占用检查完成，两个登记scratch根已清理。
P2只记录审计关闭，不改变200产品输入/四产物；S1/T16关闭，M0开放待用户验收。
既有T11并行Job计数债务仍在TODO，未宣称修复；不自动准入下一T。
