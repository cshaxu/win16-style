# Project Status

## Current Work

M0 T17 S8真实VB6 FileListBox/DirListBox已有限实现、同批包/双发布与安装8项通过，
待P1实际远端复核；VB6 DriveListBox及更多变体仍未认证，交后续S/最终审计。
公共文件夹对话框仍属独立候选，不由文件列表适配暗中替换。
S1冻结148版本/控件单位，S2原生对应项，S3-S5有限实现；按真实变体连续分S、
最后独立全任务审计。T17/M0开放，不启动其他候选，不缩小全集为普通样例。
S6为测试侧补证，15项14.12秒、双发布八SHA/安装4项、215远端/204输入、177
映像安全清理通过；生产src保持S5。其他变体和微软OCX仍交后续S及最终全集审计。

## M0 T17 S8 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S7 P1实际远端218文件/210对象恢复/182映像清理后按用户自动顺序S授权准入 |
| Admission And Approval | 用户T17完整控件目标、自动顺序S、永久提交推送及binary change同批四文件双发布；不新开T |
| Objective | 取得独立真实VB6 FileListBox/DirListBox/DriveListBox注册和公开协议，有限接入；核对文件组选择/过滤/路径通知及popup/可见状态边界；148单位总体目标不变 |
| Non-goals | 标题/字体算法/菜单/原glyph保持，不读VB私有对象，不以C模型冒称真VB；不暗装IDE/注册OCX，不替换公共目录对话框或生成近似图标 |
| Reference Baseline | S7 P1 f058ffb实际远端218文件/206输入，0.17.0.0；19项首轮18通过/demo串行复测通过、三旧包负例、安装5项及双发布八SHA；ABI3/31命名+ordinal1、35detours/40slots |
| Candidate Proposal | [VB完整适配](../proposals/vb5-vb6-controls.md)、[冻结矩阵](../etc/evidence/vb-controls-audit.json)、[S7文件组证据](../etc/evidence/vb-file-controls.json) |
| Files And ABI Surface | 双宽DLL内部精确角色/绘制接入与原生C探针/测试，先真实根因和最小方案；三组件四产物/公开ABI保持，新图形需来源审查而非随意替换 |
| Applicable Rules | Task Reading Set/C17/GCC/Ninja/ccache；禁computer-use、兄弟修改/依赖、全局设置或非自有进程测试；发布占用按用户明确执行规范处理 |
| Verification | 原作者真实VB6样本身份/启动逻辑及许可审查，真实注册/runtime/公开协议；必要有限产品接入/双宽正负例/短同批包/双发布/安装/实际远端；诊断既有demo即时GDI观察不稳定，不放宽断言 |
| Expected Markers | PE/PID/创建者/runtime/DLL实际路径SHA，加载/初始化/识别/绘制分开；列表对象/路径/选择/键盘通知不变、图像所有权、取消/关闭active0及资源归还 |
| Asset Needs | 保留build/t17-audit既有研究/runtime/原样本和s7-filelist-samples，登记其s8-filelist-samples用于真实VB6来源/源码/许可/身份留存；build/t17-controls本S探针/日志/截图，build/t17-package同批构建/包/备份；外部win16style-t17-s8（本会话visualizations根）原生PNG；其他任务/缓存/旧图保持 |
| Reporting Requirements | 真VB5/6与x64 C模型、native保持/已适配/失败/skip分开；真实键盘事件消息不冒称物理输入；未认证变体逐项有接收者，不假称T完成 |
| Stop Conditions | 公开协议或来源/许可不成立保留原生并明确缺口；重大架构/安全/新增产品先停证据方案；不通过外部未审组件安装或私有对象布局补证 |
| Exit Criteria | 真实VB6文件组有限适配及公开协议补证，逐项明确已覆盖/原生保留/有证据阻断与接收；干净双宽/同批包/双发布/推送/实际远端/恢复清理后关闭S8续后续S；T17仍需148单位独立审计 |
| Original Owner Request | VB5/VB6控件采用本项目样式；S1全貌、S2demo、S3和多个S实施，最后独立收口；自动推进到完整适配验收 |
| Similar-Issue Sweep | VB5/6文件/目录/驱动器列表的准确owner/原生子控件、owner-draw与图像数据、路径选择和Change/Click通知、setter/晚创建/销毁；未知协议保留原生且列明确缺口 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 17
- Last allocated Td S: 14
- Owner allocation constraint: 用户准入T17并授权内部自动顺序S；不准入其他T候选。
- Last delivery: M0 T17 S7 P2（19项首轮18通过/demo串行复测通过、三旧包负例、安装5项、双发布八SHA、218远端/206输入、182映像清理；0.17.0.0）
- Open task: T17 S8真实VB6文件组及文件协议边界；T17/M0开放，已有关闭/取消保持。

## Technical Baseline

S8候选0.17.0.0在S7路径上追加准确runtime60 FileListBox/DirListBox身份。真实
原作者Compress-Z-It经私有清单免注册COM激活；实际VB6类型/模块和两类控件核实，
正常显示/System文字、End/Home/原生通知/选择归还及active0通过，未运行压缩或
文件写入。双宽双版本C模型通过，16次VB6绘制GDI delta0/目录重挂退出；206输入。
23项首轮22通过，VB5样本缺BMP的测试输入补齐后两对照通过；三旧包负例检出。
双发布八SHA/安装8项/标题top26差异0通过，demo本次103->103/91->91，但S7偶发
GDI瞬时计数根因仍未定。见[S8证据](../etc/evidence/vb6-file-controls.json)。

S7已发布0.17.0.0新增准确runtime50文件列表角色与父DRAWITEM DC作用域；目录/
驱动器保留原生图像、数据、项目布局和回调，复用既有边框/滚动/箭头/字体。
206输入，双宽all干净构建；19项首轮18通过，demo即时GDI断言首轮失败、串行
5.01秒复测通过，原因未定；其余包括双宽新文件组/真实VB5及旧普通VB5/6通过。
双宽模型和真实旧包三负例检出缺口，24次绘制GDI delta0；八部署SHA一致、安装
5项通过，标题top26对S6像素差异0。有限范围见[S7证据](../etc/evidence/vb-file-controls.json)。

T17 S6产品0.17.0.0，204输入/三组件四产物，生产src保持S5；真实runtime50/60准确普通Combo/
List/HScroll/VScroll复用既有renderer、字号策略和原glyph。原生Combo EDIT按
公开COMBOBOXINFO和类元数据接入，32私有字体租约上界，回调/重入移除/重挂
归还。paint外WM_GETFONT源字体保持；paint内嵌套查询可见私有字体，不冒称全透明。
原始标题/菜单/字体算法与其他生产路径不变，五原头/来源声明及每DLL18表保持。
S6最终干净双宽all、15项聚焦实际包14.12秒、双宽见证正负例和4安装复测通过，八部署
SHA一致。真实两版本每版Combo2/List1/Scroll2正常显示，combo32黑点/scroll225
内区源像素通过，标题top26 RGB对S4差异0。S5 scroll仅getter/像素，实际位置
变化/通知已由S6队列见证验证：Combo1->2/List1->8、scroll End32766/Home0，
原生父通知一致。15项实际包14.12秒、双宽错误tag/无hook负例和安装4项通过；
其他变体继续后续S。真实VB仅x86，x64模型不是64位VB。
[release](../etc/release.json)拥有包身份，[S5外观](../etc/evidence/vb-composite-controls.json)
及[S6输入](../etc/evidence/vb-input-controls.json)拥有有限覆盖/未完成面；不能由局部
样例证明148全集完成，无物理键鼠完成声明。

## Recent Closure

- [T17 S7真实VB5文件组](../history/M0-T17-S7-vb-files.md)：206输入/218实际远端、安装5项、双发布八SHA/三旧包负例、182映像清理；关闭S7续S8，T17/M0开放。

- [T17 S6真实输入](../history/M0-T17-S6-vb-input.md)：15项14.12秒/双宽队列正负例/安装4项，双发布八SHA、215远端/204输入/177映像清理；关闭S6续S7，T17/M0开放。

- [T17 S5列表/滚动包装层](../history/M0-T17-S5-vb-composites.md)：13项15.46秒/2负例/安装4项、双发布八SHA、214远端/201输入/333映像清理；关闭S5续S6，T17/M0开放。


- [T17 S4真实内建](../history/M0-T17-S4-vb-intrinsics.md)：VB5 Command/Check/Option/Frame/Text与VB6 Option，13项23.28秒、三负例、双发布/安装、213远端/201输入、178映像清理；关闭S4/续S5，T17开放。

- [T17 S3 VB6 CheckBox](../history/M0-T17-S3-vb-checkbox.md)：真实Actor两框各182源像素、11项17.08秒、两个旧包负例、双发布/安装、212远端/200输入、340映像清理；关闭S3/续S4，T17开放。

- [T17 S2原生demo](../history/M0-T17-S2-native-demo.md)：八组对应项、6项实际包8.64秒、旧包负例、双发布八SHA和安装ABI、210远端/199输入及192映像清理；S2关闭/续S3，T17开放。

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

T17 S6 P1 d5ea84be97b65e7b1f2bcc192b6fafab512e9cc1实际远端215文件/204输入/
四产物逐字节通过；208产品对象恢复、8 before备份可从94a1170复取、8部署及
4外部PNG核对。原作者两EXE仍等于固定ZIP条目，官方runtime50 SHA保持。
177映像全部无占用，reparse0及guard正例通过，仅清理两登记任务根；保留原
样本/源/许可/tree和S1研究/其他任务/稳定缓存/全部PNG。S6关闭顺序准入S7，
文件列表及更多变体继续适配；完整148单位仍交后续S及最后独立全任务审计。
