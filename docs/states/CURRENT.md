# Project Status

## Current Work

M0 T17 S19真实图形选择图片样本审计已完成，顺序准入S20 VB 控件数组及重复实例
生命周期审计。S19确认既有真实图片样本均为CommandButton或普通选择控件，不能作为
图片CheckBox/OptionButton证据；S18的公共协议模型边界保持。用户未确认的UAC或可见
桌面观察只能跳过该项，不阻塞其他自包含S工作。
S17冻结35模块/66控件行；本机无所列官方OCX文件，故不安装/注册或从native class猜测
wrapper支持，逐行保留未认证。无产品改动、无UAC/显示依赖；后续S18审计自包含的
intrinsic图形选择default/disabled/picture变体。
S16的Data样本包含绝对数据库/绑定，MDI样本启动可写注册表并有Shell/Delete路径；
OLE/UserControl无惰性公开协议样本，均不主动接管。无产品改动、无UAC/显示依赖；
后续S17盘点官方扩展OCX的版本、可见性、公共native承载与既有renderer复用路径。
S15确认Image/Shape/Line是应用DC上的无窗口内容，Timer是零尺寸非视觉运行库helper；
不附着、不重画、不干预计时，保留既有PictureBox有限scope。无产品改动、无UAC/显示依赖；
后续S16审核Data/OLE/MDI/UserControl的真实可见承载边界。
S14固定资料唯一DriveListBox源码命中为PhotoAlbum/VB6，但同提交EXE实际导入MSVBVM50
并注册ThunderRT5类；28表单/214 archive未找到第二个真实runtime60候选，故不猜测性
扩展VB5实现。无产品改动、无UAC/显示依赖；后续S15审计Image/Shape/Line/Timer边界。
S13精确runtime-owned BS_PUSHLIKE CheckBox/OptionButton复用既有BUTTON face painter；
x86/x64选择/分组/退役对照、严格同快照四文件与assets/system32双发布通过，P1-P3
均已推送并远端复核。位图/图标/复杂owner-draw及默认变体仍原生保留，不称全集。
S12候选四文件聚焦包的x86/x64 VB5/VB6容器/图形对照通过；两轮GDI首跳后平台化、
精确字体释放和严格IAT失败阶段均有证据。生产src不变，私有桌面全包UI矩阵因本执行
会话无窗口站/桌面权限明确保留环境限制；不用current-desktop fallback伪造通过。
S13先核实真实图形CheckBox/OptionButton默认路径，M0/T17保持开放。
S11真实十个VB6图片按钮40状态内区保持；17包11.101秒/1旧负例/安装17项
12.345秒、双发布八SHA、P1远端230文件/210输入/214恢复与370映像清理通过。
生产src不变；图形Check/Option、复杂图片属性与许可/样本缺口保留后续接收，
观察器偶发失败与总GDI未定项交S12；148单位不缩小，M0/T17开放。
M0 T17 S10图形命令按钮有限交付已关闭，顺序准入S11图形选择控件/图片变体。
S10最终41包31.06秒、6旧包负例、双发布八SHA/安装11项通过，P1实际远端229
文件/210输入/214对象恢复与535映像清理通过。Check/Option及复杂图片/default
变体明确由S11继续核实，不作为已支持；其余148单位仍需独立最终审计。
S9最终33实际包26.71秒、6旧包负例、双发布八SHA、安装9项通过，P1实际远端
227文件/208输入/212对象恢复及368映像清理通过。Delphi proposal和队列编辑
已按用户回复纳入7a25f2b，仅排队不启动新T。完整GUI总GDI变化留TODO，不声称
全进程零增长。VB6 DriveListBox和更多文件变体仍交后续S/最终审计。
公共文件夹对话框仍属独立候选，不由文件列表适配暗中替换。
S1冻结148版本/控件单位，S2原生对应项，S3-S5有限实现；按真实变体连续分S、
最后独立全任务审计。T17/M0开放，不启动其他候选，不缩小全集为普通样例。
S6为测试侧补证，15项14.12秒、双发布八SHA/安装4项、215远端/204输入、177
映像安全清理通过；生产src保持S5。其他变体和微软OCX仍交后续S及最终全集审计。

## M0 T17 S20 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S19真实图片样本审计关闭后，依用户T17自动顺序S授权准入 |
| Admission And Approval | 用户T17完整控件目标、自动顺序S、永久提交推送及binary change同批四文件双发布；不新开T |
| Objective | 审计并在真实/自有模型协议足够时适配 VB 控件数组与重复实例的创建、销毁、重挂和样式隔离；不能把一个索引实例的附着状态或图像/字体资源泄露给同类兄弟 |
| Non-goals | 原标题/字体/菜单/滚动条/原glyph保持；不读VB私有对象、不替换应用图像/数据、不接管owner-draw、不装IDE/OCX、不新增T |
| Reference Baseline | S19确认真实图片选择样本缺失；既有真实VB样本含OptionButton和CommandButton控件数组，但尚未将数组索引/重复实例作为独立生命周期接受项 |
| Candidate Proposal | [VB完整适配](../proposals/vb5-vb6-controls.md)、[S19样本审计](../etc/evidence/vb-graphical-choice-samples.json)、[S6输入](../etc/evidence/vb-input-controls.json)、[冻结148矩阵](../etc/evidence/vb-controls-audit.json) |
| Files And ABI Surface | 先审计真实源码数组与现有自有C模型；只有公开HWND/owner/状态/资源协议完整时才扩展既有双宽DLL/测试，不改ABI/三组件四产物 |
| Applicable Rules | Task Reading Set/C17/GCC/Ninja/ccache；禁computer-use、兄弟修改/依赖、全局设置或非自有进程测试；无显示/UAC结果仅作对应skip，不阻塞自包含审计 |
| Verification | 记录数组实例的runtime/owner/class/style/父子关系、独立状态/字体/图片、销毁/重建/重挂与负例；实际与模型、x86实际与x64模型分开 |
| Expected Markers | 每个实例独立附着/退役且无资源串扰，或明确保持原生；应用输入/通知/字体/几何/图像所有权保持 |
| Asset Needs | 仅已有登记样本/运行库/任务scratch；不复制第三方图片/OCX/二进制、不装IDE或注册组件 |
| Reporting Requirements | 报告实际样本、模型、数组创建/销毁边界和未覆盖面；不由单实例外推，不称T17完成 |
| Stop Conditions | 真实协议/来源不满足即保持原生；重大架构/安全/新增产品先停证据；不关闭/注入用户既有进程，也不等待UAC或屏幕可见性 |
| Exit Criteria | 完成有界数组生命周期审计；若有证实最小实现则双宽/同批包/必要发布/推送远端，否则提交审计。两种结果均保留148单位，T17开放 |
| Original Owner Request | VB5/VB6控件采用本项目样式；S1全貌、S2demo、S3和多个S实施，最后独立收口；自动推进到完整适配验收 |
| Similar-Issue Sweep | runtime50/60 graphical选择、BUTTON type/PushLike、BM_GETCHECK/BM_GETIMAGE、default/disabled/pressed/focus、radio分组/通知、图片/bitmap/icon/owner-draw、角色切换/资源退役、真实与模型边界 |

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 17
- Last allocated Td S: 14
- Owner allocation constraint: 用户准入T17并授权内部自动顺序S；不准入其他T候选。
- Last delivery: M0 T17 S19 P1（图形选择真实图片样本审计，无产品二进制变化；推送远端后关闭）
- Open task: T17 S20 VB控件数组及重复实例生命周期审计；复杂图片及148单位审计保留后续接收，T17/M0开放。

## Technical Baseline

S10已发布0.17.0.0追加真实RT5/6 graphical CommandButton，精确ODT_BUTTON/DC/
runtime primitive复用既有边框，ANSI caption借用System。Native返回/error/rect、
源属性/图像/输入保留。41最终包31.06秒/6旧包负例/安装11项通过，八SHA一致；
普通双版本整窗、既有PictureBox、标题/按钮外区域RGB差异0。210输入/214对象
恢复及229实际远端、535映像清理通过。图形Check/Option/default/picture变体
仍未认证；见[S10证据](../etc/evidence/vb-graphical-controls.json)，T17/M0开放。

S9已发布0.17.0.0追加真实RT5PictureBox/RT6PictureBoxDC有限承载，runtime直接
ANSI标签在paint内采用System，原生测量/Print/应用自绘/图像保持。最终33包
26.71秒、6旧包负例、安装9项通过；八部署一致、标题top26/logo/整块Print图
RGB差异0。208输入/212对象恢复及227实际远端通过，368映像安全清理；精确私有
字体移除释放通过，但整轮GUI总GDI跳变未定位，不宣称零增长，见TODO及
[S9证据](../etc/evidence/vb-container-controls.json)。ABI/35hooks/原glyph不变。

S8已发布0.17.0.0在S7路径上追加准确runtime60 FileListBox/DirListBox身份。真实
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

- [T17 S19图形选择真实图片样本](../history/M0-T17-S19-vb-graphical-choice-samples.md)：已登记VB5/VB6样本没有图片CheckBox/OptionButton；无产品改动，S19关闭续S20，T17/M0开放。

- [T17 S18图形选择图片/图标](../history/M0-T17-S18-vb-graphical-choice-variants.md)：精确公开图片/图标借用、状态、资源与双宽包验证；S18发布后顺序准入S19，T17/M0开放。

- [T17 S11图片变体](../history/M0-T17-S11-vb-pictures.md)：真实十按钮/40状态内区保持、17包/1负例/安装17项、八SHA、230远端/210输入/214恢复/370映像清理；关闭S11续S12，M0/T17开放。

- [T17 S10图形命令按钮](../history/M0-T17-S10-vb-graphical.md)：41包/6负例/安装11项、双发布八SHA、229远端/210输入/214恢复/535映像清理；S10关闭续S11，T17/M0开放。

- [T17 S9容器与无窗口标签](../history/M0-T17-S9-vb-containers.md)：最终33包/6负例/安装9项、双发布八SHA、227远端/208输入/212恢复/368映像清理；S9关闭续S10，T17/M0开放。

- [T17 S8真实VB6文件组](../history/M0-T17-S8-vb6-files.md)：206输入/221实际远端、安装8项、双发布八SHA/三旧包负例、181映像清理；关闭S8续S9，T17/M0开放。

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

S11 P1 9adeb46de68adaf65a28bec21b504bbf5a7ce008实际远端230文件/210输入/四产物
逐字节通过，214产品对象恢复，8 before从5580bbbb复取核对，17原样本与2PNG
SHA保持；350+20映像无占用/reparse0/guard正例通过后仅清理两个登记根。
所有原来源/runtime/许可待答数据/研究/恢复对象/稳定缓存/其他任务与PNG保留。
S11关闭，顺序准入S12观察器/资源审计；图形选择/复杂图片/Drive等缺口仍需后续S，
最后另做148单位独立审计，M0/T17开放。
