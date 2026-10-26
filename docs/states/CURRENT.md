# Project Status

## Current Work

M0 T17 S1控件全貌审计已交付关闭；当前顺序准入S2，按用户要求扩充demo.exe。
S1冻结148版本/控件单位，真实应用/OCX/变体缺口交后续S，不冒称已全部适配。
S2用纯C64位原生对应项展示控件、状态和交互；32位VB/OCX真实验证仍在独立探针。
S3起连续分S实施，最后独立收口S逐项审计；T17/M0开放，其他候选不启动。

## M0 T17 S2 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；用户指定S2加入demo，S1完成实际远端后顺序准入 |
| Admission And Approval | 用户T17整任务及S顺序授权，永久提交推送/有binary change双发布保持 |
| Objective | 将S1目录的可见控件族以真实原生对应项加入64位demo，提供状态/数据/交互展示和明确非视觉/应用内容边界 |
| Non-goals | 不把native demo冒称真实VB或32位OCX，不内建renderer，不改DLL视觉/字体/资源/传播；不新组件/第五产物或VB编译器依赖 |
| Reference Baseline | S1 P1 944402a已实际远端208文件/197输入，产品仍0.15.0.0；S2产品版本转T17，原四文件构建发布 |
| Candidate Proposal | [VB完整适配](../proposals/vb5-vb6-controls.md)及[S1矩阵](../etc/evidence/vb-controls-audit.json)；用户指定S2 |
| Files And ABI Surface | demo gallery/main/版本、必要CMake/native系统导入门禁及对应测试；demo仍64位纯C native API，不导入ntw16s；可按需链接COMCTL32/GDI32和固定系统RichEdit；公开DLL ABI不变 |
| Applicable Rules | Task Reading Set/架构/编码/UI/资源来源；C17/GCC/Ninja/ccache；禁computer-use/兄弟依赖/全局注册 |
| Verification | 新gallery每页一次正常/样式对照、有限状态/事件/退出，原有demo交互保留；双宽ABI、干净同批四文件实际聚焦包/系统导入/原资源门禁与双发布SHA；短测试不重复大矩阵 |
| Expected Markers | native样例实际类/数量/页面、可交互状态和通知、资源释放/正常显示截图、裸/挂钩差异及DLL实际路径；不把展示当所有VB通过 |
| Asset Needs | build/t17-audit保留S1原生研究/官方VB5 DLL按SHA交接未来实现；build/t17-package登记本T构建/包/探针/备份，先提炼后精确清理；保留其他任务49源码/缓存/外部图 |
| Reporting Requirements | 明确native对应项、无视觉部件、应用图像/数据/嵌入对象保留和无合适对应项；实际测试时长/跳过面、四文件及双发布/远端；T17保持开放 |
| Stop Conditions | 不能将32位OCX加载进64位demo；如需第四组件/第五产品/安全边界则先证据方案；不静默改变已确认视觉或新造图形资源 |
| Exit Criteria | demo展示交付、双宽同包和有限实际测试、双发布八SHA、提交/实际远端/资产交接清理后关闭S2并续S3；不凭展示关闭T17 |
| Original Owner Request | S1审计控件全貌，S2将这些控件加入demo.exe，S3和多个S负责实施，最后收口S |
| Similar-Issue Sweep | 原demo Apply/列表/焦点/菜单保留，新增每控件族、native系统库所有者/有无HWND/自绘内容边界；实际VB证据与C对应项分开 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 17
- Last allocated Td S: 14
- Owner allocation constraint: 用户明确准入T17并指定S顺序；可在T17内自动顺序拆S实施到独立收口，不准入其他候选。
- Last delivery: M0 T17 S1 P2（148单位审计；基线双宽/三项包2.03秒、208远端文件、147映像清理和研究资产交接；无产品变化）
- Open task: T17 S2原生demo控件展示；T17/M0开放，既有关闭/取消保持。

## Technical Baseline

T16 S2已收口并撤回产品源码/测试，恢复0.15.0.0 / M0 T15元数据，197输入；治理仍为T16 S2。
全部196个CMake/src/tests/tools路径等同96e4721；原五头/声明/每DLL18表保持，三组件
四产物/ABI3/31命名+ordinal1、35私有detours/40slots。干净双宽all和同批四文件门禁
通过；实际包8项聚焦4.51992秒、两处双发布八SHA及安装ABI32/64通过。测试范围按
用户要求缩小，不冒称完整91/99全集或物理输入验收。原生定位各宽一次，轮廓功能已撤。
[release](../etc/release.json)为当前发布身份，[S2撤回](../history/M0-T16-S2-outline-withdrawal.md)
及[证据](../etc/evidence/outline-withdrawal.json)接收实际验证；P1远端/恢复清理均通过。

## Recent Closure

- [T17 S1 VB全貌审计](../history/M0-T17-S1-vb-control-audit.md)：148单位、原始VB5/VB6目录/真实类型库与源码缺口；三项基线包2.03秒、208远端文件和147映像清理完成，研究资产交接S2/后续；S1关闭，T17开放。

- [T16 S2轮廓效果撤回](../history/M0-T16-S2-outline-withdrawal.md)：产品0.15.0.0，196代码/197输入恢复T15，8项实际包4.52秒、双发布八SHA、远端213文件及三404、201对象恢复、169映像精确根清理通过；S2/T16关闭（撤回），M0开放。

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

T17 S1 P1 944402a实际远端208文件/197输入及四现发布文件通过；实际diff/目录/原始
请求复核完成。S1仅审计，197产品输入/八部署SHA不变，不虚构新增适配或发布。
干净基线双宽及三项实际包2.03秒，147映像无占用/reparse0后清理精确基线根；
T17研究C源码/官方VB5 DLL按SHA交接后续S，继续登记。S1关闭，用户指定S2顺序准入，
T17/M0保持开放；真实必测缺口仍由后续实施S接收，不可据native demo消除。
