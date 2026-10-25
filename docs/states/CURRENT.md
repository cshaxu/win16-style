# Project Status

## Current Work

S10滑块已完成最终193输入/83实际包427.192秒、双发布八摘要、安装28+2、真实MD5/DK
及标题292x24掩码0差异；P1提交推送/实际远端归档/恢复与安全清理待做，S10/T15/M0
保持开放。首次整包树10秒超时保留；旧/新四并发均复现，约5秒CPU接近10秒wall，
两并发旧/新均通过；树case采用已有两slot权重，原10秒及功能断言不变，最终83通过。
[本S记录](../history/M0-T15-S10-trackbar-controls.md)/[证据](../etc/evidence/trackbar-controls.json)。
微调未猜状态，后续顺序S接收；HD Tune管理员实测继续默认skip、不计通过。

## M0 T15 S10 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S9闭环后顺序准入S10，唯一活动S，不复用编号，不新开T |
| Admission And Approval | 用户“批准 自动准入s任务 到t收口为止”“继续啊”；永久提交推送/双发布授权；HD Tune管理员实测按最新要求默认skip，除非用户明确恢复 |
| Objective | 核实剩余msctls_trackbar32/msctls_updown32及必要精确框架别名的初始化协议、实际绘制/状态/通知与生命周期；原型后S10有限适配滑块native custom-draw，保持原生输入和应用通知；微调状态路径独立，接收者为顺序S11，完成后另做T15冻结全集审计 |
| Non-goals | 不猜控件私有对象/状态，不盲按T前缀，不把关主题/附着当实际像素；不改系统全局设置/原五头/声明/已确认标题/菜单/字体/滚动条/启动语义，不做既有进程注入/HD Tune提权实测，不制造新T或第五产品 |
| Reference Baseline | S9 P1 2721c043c2d1fd485b4ed8d1362974d269b2b22f同批0.15.0.0四文件已双发布；190输入/79包/安装20+2/远端203/归档双DLL+4检查及260映像清理通过；关闭P无binary change |
| Candidate Proposal | [框架完成分解](../proposals/framework-completion.md)；[S9记录](../history/M0-T15-S9-progress-controls.md)/[证据](../etc/evidence/progress-controls.json)接收剩余两族；v5进度和特殊模式原生保留边界不默默扩张 |
| Files And ABI Surface | 私有trackbar初始化角色/仅应用default时内部请求部件阶段和绘制/父通知观察及C17夹具；三组件四产物、ABI3/31命名导出+ordinal1 NONAME、31detours32slots保持；新hook/公共接口先审，不默认加产品/helper |
| Applicable Rules | EXECUTION/ARCHITECTURE/CODING/DOCUMENT/UI及来源策略；MinGW GCC/Ninja/稳定ccache，禁computer-use及兄弟仓库修改/依赖 |
| Verification | 先核实原生模块/附加字节、范围/位置/方向/滑块与通道公开几何、buddy/UDN取消及custom-draw行为；普通/特殊/禁用/按下/焦点/键盘/捕获与后续状态，双宽v5/v6实际DPI/像素/数据/字体/卸载/资源/重入/旧包负例；干净同批四文件实际包/双发布/八摘要/安装/实际远端归档；HD Tune管理员skip |
| Expected Markers | 区分模块/初始化/附着与实际绘制；正常可见窗口原生截图和像素、命中/几何/位置/通知/取消/应用绘制保持；未知或custom-draw不安全时保留原生，原图形表逐字节相同 |
| Asset Needs | build/t15-controls承接S9已审22源码并登记本S新探针/日志；build/t15-package登记新干净包/备份；先提炼后核对根/祖先/reparse/映像占用及恢复来源再清理，保留稳定.ninja/.ccache和外部PNG |
| Reporting Requirements | 每类先给原生状态/回调实际证据、最小实现路线和有限覆盖，不吞应用通知或绘制；不能证明pressed等状态时明确边界，不凭自绘猜测宣称保真；保留全部失败实验和未覆盖接收者 |
| Stop Conditions | 需私有状态解码/全局hook或设置/共享宿主/既有进程注入时停在证据与方案；不扩权限、吞业务回调/取消或换资源，不放宽失败断言；范围过大按既有授权拆顺序S，不新T |
| Exit Criteria | 完成经证实的有限族适配或有证据拆分/原生保留，双宽同批实际包、必要双发布八摘要/安装、提交推送/实际远端协调复核及安全清理后关闭S10；T15另冻结覆盖全集逐项审计后才可关闭 |
| Original Owner Request | “好 怎么办吧”“批准 自动准入s任务 到t收口为止”“继续啊”；“跳过这个测试 以后默认不测” |
| Similar-Issue Sweep | canonical/EXE超类/伪类、初始化/后续样式/位置、native custom-draw/owner-draw、范围反向与边界、buddy与原生取消、输入修饰键/焦点/捕获/按下、重入销毁/独立所有者及资源；不将消息驱动扩张为物理/IME验收 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 15
- Last allocated Td S: 13
- Owner allocation constraint: 只可自行划分后续S，不得自行分配新T；保持现有T15/M0。
- Last delivery: M0 T15 S9 P2（P1远端203/受影响双DLL归档重建4/194对象恢复与260映像清理通过；关闭S9/准入S10，四文件保持）
- Open task: T15自动续S实施/任务级审计中；S10活动，S9/S8/S7/S6/S5/S4/S3/S2/S1关闭；Td S13/S12/S11关闭；T14 S3/T14保持关闭；T13/T12/T11/T9/T8关闭；T10取消；Td S9/S10保持关闭

## Technical Baseline

0.15.0.0 / 本机S10滑块同批四文件已双发布，193输入SHA由release.json保存；83实际包
427.192秒、安装28聚焦+2自动矩阵、真实MD5/DK、标题292x24黑掩码0差异通过。
P1提交推送/实际远端归档及安全清理尚待完成，当前Git交付基线仍S9 P2。普通v5/v6
滑块/精确TTrackBar使用原生custom-draw矩形及CDIS_SELECTED；应用自身绘制/通知、
范围/字体/布局/输入保持。特殊滑块及微调保持原生，微调交后续顺序S。树测试四并发
旧/新均复现10秒超时，已有两slot权重限流后最终83通过，未改等待/功能断言。
既有有限树/报告行/进度能力及原生保留边界继续，HD Tune管理员默认skip。
纯C17/GCC/Ninja/ccache、原五头/声明/18表、已确认标题/字体/菜单/滚动条算法、
三组件四产物、ABI3/31命名导出+ordinal1/31detours32slots保持；S10/T15/M0开放。
[本S记录](../history/M0-T15-S10-trackbar-controls.md)/[证据](../etc/evidence/trackbar-controls.json)。

## Recent Closure

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

S9 P1 2721c04完成有限v6进度与受控树消息输入，190输入、79实际包/双发布八摘要、安装
20聚焦+2矩阵、真实MD5/DK和标题292x24掩码0差异通过；旧输入4/冻结旧产品2负例有效，
未追认S8未记录的历史状态。实际远端203匹配，严格归档/原资源门禁、受影响双DLL定向
重建4通过，避免重编无关测试。194 Git对象证明190源及八旧备份可恢复，八部署再次
匹配；原程序自有副本未变。精确根/祖先/子目录reparse0，原生守卫正例拒绝运行EXE/
加载DLL C0000043，卸载成功；260任务映像含两个长路径无占用，守卫退出后清理。
保留22已审源码及稳定Ninja/ccache/外部PNG。本关闭P不改产品输入或binary；S9关闭并
按既有授权顺序准入S10滑块/微调，M0/T15开放，HD Tune管理员默认skip。
