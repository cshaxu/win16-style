# Project Status

## Current Work

M0 T17 已按用户明确缩小后的范围收口；23 项 VB 延期项保留在 TODO。T18 S1 已冻结
Delphi VCL 74 个 UI 家族种子、框架分支、五个固定候选及逐项证据接收者；T18 S2 已将
全部种子逐项归入35个既有原生协议模型或39个明确的原生保留边界。S4 已完成首组
精确 TEdit/TListBox与TTabControl公开协议实现、干净双宽四文件包和双发布；实际 Delphi 显示集成
仍统一留给T18最终收口。S10已完成组合框与列表验证；S11已完成工具栏文字边界修复、真实DoubleKiller复测及提交推送复核；S12真实MD5summer补证及远端复核完成；S13双宽干净构建、真实树控件与VB6 actor复测、91项包测试、双发布及远端aae1565复核完成；S14纠正S12独立包闭环并完成双发布及远端f1ac9ff复核；S15核实现代VCL候选及公开状态边界，双宽包、91项低并发回归、双发布及远端4032bcf复核完成；S16追踪真实开关绘制及输入、双宽包91项通过、双发布及远端5e48716复核完成；S17修复进度观察竞争并减少字体重复选取，双宽包92项通过、双发布及远端b3c2a8b复核完成；S18完成开关公开状态调查、双宽包92项及双发布，远端2edd749已复核；S19实现可审计聚焦包回归，聚焦10项、标准93项、双发布及远端d583890复核完成；准入S20继续开关视觉适配；S20恢复隔离桌面并验证开关及VB/VCL聚焦包，未证实的开关适配移交TODO；S21完成有限复选图形与双宽96/144 DPI验证，P4远端2245db3已复核；其应用裁剪外缘和显式禁用仍列TODO；S22完成真实SpeedButton公开契约调查、14项聚焦包及双发布，cb2ffef远端已复核；无安全契约的外观接收TODO；S23已实现真实VCL v5进度条有限外框并通过22项聚焦包与双发布，33f4d0d远端已复核；S24进度客户区转TODO；S25原生状态栏有限边框、36项聚焦包及双发布完成，2be886a远端复核通过，真实Delphi待证；准入S26逐项盘点剩余家族；不得用 demo
冒充 Delphi。S26已以74项证据台账、聚焦包和双发布收口，50项真实owner尚待证实；S27真实TSpinButton有限边框已交付；S28/S29未证实的面板/标签缺口转TODO，S30准入字体测试效率修复。
以下段落为 T17 的技术基线/历史证据，不覆盖 T18 状态。

M0 T17 S27 ScrollBar范围/位置/禁用变体已完成，顺序准入S28 全148单位收口审计。
S27以自有runtime双宽见证避开私有桌面错误5，验证H/V范围、页大小、位置和禁用；
S28逐项核对后续证据，不把早期矩阵的陈旧状态当作当前实现结论。
S28干净双宽候选包已通过身份门禁，四文件在测试前后摘要一致；实际包87项中15项
在本会话通过，72项在创建私有USER桌面前返回精确`PRIVATE_DESKTOP_CREATE_FAIL error=5`
而记为skip。Test-Package现仅对该精确退出2/标记组合允许skip，其他失败仍中断；
这不构成GUI回归或物理验收通过；四文件已以同批摘要发布至assets根目录和
O:\\winnt\\system32；P1提交328a8b6已推送，远端main及发布清单/OCX证据实际复核通过。
S17冻结35模块/66控件行；本机无所列官方OCX文件，故不安装/注册或从native class猜测
wrapper支持，逐行保留未认证。S28现按用户授权仅在任务scratch中研究微软签名x86
MSCOMCTL.OCX/COMCTL32.OCX：不安装/注册/发布，已证实有限公开TreeView、旧版Slider及
旧版COMCTL32文本工具栏路径；工具栏仅接入精确嵌套原生ToolbarWindow32及无图像普通/勾选按钮，
所有图像、分隔、下拉、自绘变体原生保留。TabStrip回归即撤回、ProgressBar状态契约不足而原生保留，
MSMASK32的MaskEdBox已证实为标准EDIT superclass，沿用既有EDIT适配；DataGrid等外部OCX临时frame
位现明确拒绝，且只保留其标准USER子控件路径。随后用当前已发布x86 DLL复测官方MCI32.OCX：其私有
mciWndClass下九个USER32 Button均为BS_OWNERDRAW，故不以泛化BUTTON路径破坏OCX的WM_DRAWITEM契约；
MMControl改列原生保留。SysInfo/Inet/PictureClip亦已在自有免注册OLE宿主逐项观察，均无标准子HWND或
公开绘制协议而原生保留。当前包再验证RichTextBox、SSTab、MSFlexGrid/MSHFlexGrid和MSChart均仅有私有
OCX client surface，故同样原生保留；DBCombo/DataCombo的USER Edit和DataList的USER ScrollBar已在当前包
通过文本选择及范围位置回读，DataGrid的USER ScrollBar也已同样验证，限定复用既有renderer，外壳/绑定/
自绘不变。DBList/Adodc/MSRDC/DataRepeater无标准子HWND而原生保留；FlatScrollBar外壳、StatusBar/ListView/
MSCOMCTL Toolbar20及其余未证实行逐项保留。MSComm/Winsock/MAPI仅有私有非视觉承载而原生保留；旧COMCTL32的StatusBar/TabStrip/ListView/ProgressBar/ImageList也已由当前包的免注册自有宿主确认是私有外壳：ListView仅有prepaint且后建SysHeader32仍走既有标准路径，其他没有稳定的内容中性协议，均原生保留。COMCT232的Animation/UpDown、COMCT332的CoolBar和MSCOMCT2的Animation/UpDown/MonthView/DTPicker/FlatScrollBar都已分别确认仅为私有OCX/VB外壳、没有标准子控件，故原生保留。新版MSCOMCTL除已限量验证的TreeView外，其余八个private *20WndClass外壳也均原生保留。COMDLG32的真实ShowOpen #32770文件对话框已由自有宿主自动取消并确认附着当前renderer（OCX helper自身仍原生）；DBGRID32的公开许可路径两次精确返回CLASS_E_NOTLICENSED，未绕过许可、仍需合法样本。USACE官方历史HEC-RAS 4.1安装包已仅下载到任务scratch并验证签名/SHA，但InstallShield数据流不能在不执行安装器的边界内静态取得程序，故也不作为运行样本；分类为46项有限验证、79项原生保留、23项按用户决定延期至TODO。
无UAC/显示依赖；后续S18审计自包含的
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

## M0 T18 S54 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S53 P1 37afecf已推送，远端7文件逐字节复核 |
| Admission And Approval | 用户授权T18内自动完成可实施控件；不新开T |
| Objective | 调查真实VCL TStringGrid/TDrawGrid的窗口、内容和滚动边界，实施可证实范围 |
| Non-goals | 不覆盖应用单元格内容、编辑、数据、布局、输入或资源 |
| Reference Baseline | [S38控件审计](../history/M0-T18-S38-control-audit.md)、[S53组框边界](../history/M0-T18-S53-radio-frame.md) |
| Candidate Proposal | [Delphi控件完整审计](../proposals/delphi-win32-controls.md) |
| Files And ABI Surface | 双宽自有原生探针；有证据时修改现有DLL与测试，不增产品或ABI |
| Applicable Rules | M0、C17、GCC、Ninja、ccache；自有隔离实例限时清理 |
| Verification | 双宽真实窗口身份、滚动与输入、内容边界；如实现则验证绘制及实际包 |
| Expected Markers | 独立于应用内容的可验证边框/滚动契约；不明区域保留原生 |
| Asset Needs | 登记build/t18-s54-grid，复用合法双宽样本 |
| Reporting Requirements | 确定支持面、失败和未覆盖、双发布与远端复核 |
| Stop Conditions | 无公开、内容中性的安全边界时不泛化绘制 |
| Exit Criteria | 安全有限实现或独立可复核证据与接收者，同批包交付；T18保持开放 |
| Original Owner Request | 先完成能做的代码和测试，真实应用全量集成统一在T收口 |
| Similar-Issue Sweep | VCL字符串网格、自绘网格和原生滚动条 |

S53 P1 `37afecf`远端7文件逐字节复核通过；组框原型会覆盖标题，未合入，
独立留TODO；不以此阻止其余控件调查。T18/M0保持开放。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 18
- Last allocated Td S: 14
- Owner allocation constraint: 用户准入T18队列首项；T18内可按其明确的阶段计划细分S，不准入其他T候选。
- Last delivery: M0 T18 S53 P1：12项聚焦包、双发布及37afecf远端7文件复核。
- Open task: T18 S54 VCL字符串及自绘网格调查；M0/T18开放。

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

- [T18 S48日期GDI测试](../history/M0-T18-S48-date-gdi-test.md)：双宽精确区域与泄漏负例、16项并发包及e60569b远端复核；总数波动仍未定位。

- [T18 S47图标末分区](../history/M0-T18-S47-status-icon.md)：双宽真实窗口、26项包、双发布及a6baff4远端复核；首轮GDI波动留TODO。

- [T18 S46空末分区](../history/M0-T18-S46-empty-status-pane.md)：双宽真实窗口、26项包、双发布及1b7650f远端复核。

- [T18 S45状态栏末分区](../history/M0-T18-S45-status-last-pane.md)：双宽真实分区、26项包、双发布及a129ce5远端复核。

- [T18 S44 VCL状态栏](../history/M0-T18-S44-vcl-status.md)：双宽真实首分区、24项包、双发布及0df464d远端复核。

- [T18 S43真实VCL状态栏调查](../history/M0-T18-S43-vcl-status-audit.md)：双宽真实分区/自绘证据、18项包及5a5089e远端复核。

- [T18 S42 VCL日期按钮](../history/M0-T18-S42-vcl-date-picker.md)：双宽真实样本、16项包、双发布与b6db3e6远端复核。

- [T18 S41 VCL日期控件调查](../history/M0-T18-S41-date-picker-audit.md)：双宽真实弹层与键盘、按钮试绘、12项聚焦包与双发布；首轮探针异常未定位。

- [T18 S40颜色框屏幕调查](../history/M0-T18-S40-color-screen.md)：旧/新双宽绘图DC对照、12项聚焦包及双发布；具体屏幕机制留TODO。

- [T18 S39 VCL颜色组合框](../history/M0-T18-S39-vcl-color-combo.md)：20项聚焦包、双发布、双宽像素与通知对照及P1远端12文件复核；偶发屏幕差异交S40。

- [T18 S38 VCL控件调查](../history/M0-T18-S38-control-audit.md)：七类真实窗口与颜色框试绘、12项聚焦包、双发布及P1远端七文件复核。

- [T18 S37运行时VCL滑块](../history/M0-T18-S37-vcl-trackbar.md)：24项聚焦包、双发布、真实双宽像素对照及P1远端17文件复核。

- [T18 S36滑块调查](../history/M0-T18-S36-trackbar-audit.md)：双宽真实样本、后绘制可行性、18项聚焦包、双发布及P1远端七文件复核；其余审计未完成。

- [T18 S35对话框测试效率](../history/M0-T18-S35-dialog-efficiency.md)：双宽范围/显示、完整对话框与消息框提速，双发布及P1远端11文件复核。

- [T18 S34运行时模块TEdit](../history/M0-T18-S34-vcl-module-edit.md)：双宽真实样本、22项聚焦包、双发布及P1远端16文件复核。

- [T18 S33集成进度](../history/M0-T18-S33-integration-progress.md)：127项完整包、双发布及真实双宽控件调查；未完成审计转后续S。

- [T18 S32菜单测试效率](../history/M0-T18-S32-menu-efficiency.md)：双宽菜单5.55/4.97秒、20项聚焦包、双发布及P1远端八文件复核。

- [T18 S31 VCL静态文本](../history/M0-T18-S31-static-text.md)：双宽模型与真实VCL、18项聚焦包、双发布及P1远端复核；限定ASCII和普通绘制。

- [T18 S30字体测试效率](../history/M0-T18-S30-font-efficiency.md)：双宽聚焦14项零skip、双发布及f079892远端十文件复核；完整压力测试保留T18收口。

- [T18 S29无窗口标签调查](../history/M0-T18-S29-label.md)：真实标签身份、字体度量和Unicode绘制边界、14项聚焦包及双发布；安全契约未证实，P1远端七文件复核通过。

- [T18 S28真实面板调查](../history/M0-T18-S28-panel.md)：两个真实样本、公开颜色属性、16项聚焦包及双发布；安全契约待证并保留TODO，P1远端七文件复核通过。

- [T18 S27真实微调有限边框](../history/M0-T18-S27-spin-button.md)：真实四状态像素/输入、双宽模型、30项聚焦包、双发布及P1远端复核；真实键盘和其他应用仍待验证。

- [T18 S26逐项证据台账](../history/M0-T18-S26-control-audit.md)：74项分类、50项真实owner缺口、12项聚焦包及双发布；P1远端八文件复核通过。真实TSpinButton原型交S27，不视为已发布实现。

- [T18 S10组合框与列表超类](../history/M0-T18-S10-vcl-list-superclass.md)：双宽自有模型、87项包测试、双发布及远端复核；真实Delphi及owner-draw语义另验。

- [T18 S9编辑超类](../history/M0-T18-S9-vcl-edit-superclass.md)：双宽多行原生模型、87项包测试、双发布及远端复核；掩码和RichEdit语义仍待独立验证。

- [T18 S8选择控件超类](../history/M0-T18-S8-vcl-choice-protocol.md)：双宽自有超类模型及87项包测试、双发布和远端复核；真实Delphi各版本尚未证明。

- [T18 S7范围控件审计](../history/M0-T18-S7-vcl-range-audit.md)：既有协议核对及87项私有桌面包测试、双发布与实际远端复核；真实Delphi集成和物理输入仍待独立验证。

- [T18 S5页签公开协议](../history/M0-T18-S5-vcl-tabs.md)：TTabControl补入既有TPageControl
  协议、两种伪类拒绝、干净双宽包和同批双发布；日期/月历/热键因无renderer保留，真实Delphi集成仍留T18收口。

- [T18 S4基础VCL公开协议](../history/M0-T18-S4-basic-vcl-protocols.md)：精确TEdit/TListBox
  的公开消息、模块/注册身份、旧实现负例、干净双宽包和同批双发布；真实Delphi集成仍留T18收口。

- [T18 S3基础VCL探针转交](../history/M0-T18-S3-vcl-probe-transfer.md)：当前自有隔离桌面error=5
  发生于目标启动前；按用户授权将真实应用集成后移T18收口，S3转交S4，T18/M0开放。

- [T18 S2 Delphi demo对应](../history/M0-T18-S2-delphi-demo-mapping.md)：74项种子精确分为35个
  原生协议模型及39个保留边界；demo观察器私有桌面error=5精确skip，无产品二进制变化，S2关闭续S3，T18/M0开放。

- [T18 S1 Delphi控件全貌审计](../history/M0-T18-S1-delphi-control-audit.md)：74项VCL
  UI种子逐项接收者、五固定候选与VCL/FMX边界；无产品二进制变化，S1关闭续S2，T18/M0开放。

- [T17 S28 VB全148单位最终审计](../history/M0-T17-S28-vb-final-audit.md)：46项有限
  公开协议、79项原生保留、23项按用户决定延期TODO；无产品二进制变化，S28/T17关闭，M0开放。

- [T17 S27 ScrollBar变体](../history/M0-T17-S27-vb-scrollbar-variants.md)：自有双宽runtime
  见证H/V范围、页大小、位置和禁用；无产品二进制变化，S27关闭续S28，T17/M0开放。

- [T17 S26 ListBox变体](../history/M0-T17-S26-vb-listbox-variants.md)：真实普通/extended选择与未见样式边界；无产品改动，S26关闭续S27，T17/M0开放。

- [T17 S25 ComboBox变体](../history/M0-T17-S25-vb-combo-variants.md)：真实三种公共组合框样式与双宽runtime witness；无产品改动，S25关闭续S26，T17/M0开放。

- [T17 S24 TextBox特殊变体](../history/M0-T17-S24-vb-textbox-variants.md)：真实多行/Locked边界与双宽runtime witness；无产品改动，S24关闭续S25，T17/M0开放。

- [T17 S23 CommandButton状态变体](../history/M0-T17-S23-vb-command-button-variants.md)：真实default/cancel/flat边界与双宽default runtime witness；无产品改动，S23关闭续S24，T17/M0开放。

- [T17 S22 Frame容器](../history/M0-T17-S22-vb-frame-containers.md)：真实Frame/caption/嵌套子控件与既有有限路径复核；无产品改动，S22关闭续S23，T17/M0开放。

- [T17 S21 Label变体](../history/M0-T17-S21-vb-label-variants.md)：真实源码/运行时无窗口DC边界；无产品改动，S21关闭续S22，T17/M0开放。

- [T17 S20控件数组](../history/M0-T17-S20-vb-control-arrays.md)：真实数组源码依据和双宽独立成员生命周期回归；无产品改动，S20关闭续S21，T17/M0开放。

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

T17 S28 P15 待本次治理验证、提交、推送及远端复核：全148单位以46项有限公开协议、
79项原生保留及23项用户延期TODO收口；无产品二进制变化，现有四文件不重新发布。

S49候选四文件已双宽干净构建，20项聚焦包无跳过；旧包双宽负例检出，144 DPI
真实TBitBtn双宽与自有模型通过，assets/system32八文件摘要一致，系统入口双宽通过。
异常展开、外国调用者及嵌套绘制负例仍未覆盖，拟在S50继续；等待S49提交推送及远端复核。

S51自有真实DelphiVCL 1.0.7双宽144 DPI：TRadioGroup容器和两个TGroupButton均初始
未附着；选项真实WM_PAINT由runtime转发系统BUTTON，BCM_GETIDEALSIZE成功，BM_CLICK
后由0变1。候选仅针对同runtime父组、原生BS_RADIOBUTTON与实际转发证据，双宽候选
附着并保留合成点击；容器本身仍原生。候选尚未提交/发布，须继续核验字体、分组、
正负模型、重入/清理及完整干净包；此状态不得作为最终交付。

S51双宽自有模型与真实Delphi单选组候选均通过，旧包负例双宽检出；独立干净包
18项聚焦测试零跳过，assets/system32双发布与系统入口双宽通过。外层组框和动态
换父/角色未验证，已移交TODO；待提交推送及远端复核。

S52旧包双宽换父负例确认附着未退出；候选退役并恢复字体、调度重绘，双宽模型
覆盖换父和owner-draw类型变化。最终干净包16项聚焦测试无跳过、双发布及安装
入口双宽通过；等待提交推送与远端复核。组框与其他变体仍留T18。

S53自有双宽144 DPI真实TRadioGroup主题part 4由runtime绘制；父组自身窗口DC及
子项借用的内存DC都出现同类调用。自有临时原型只改父窗口DC的边框，实际截图中
黑线穿过RadioGroup标题，故拒绝；已观察的DrawTextW/DrawThemeText窗口DC未捕获标题
边界，不得据此推断无标题绘制。x64探针首次硬编码32 DLL导致假阴性，修正后确认。
当前未修改产品；需继续界定标题/剪裁才能安全实施。

S53独立干净包12项聚焦测试无跳过，二进制摘要变化故双发布，系统入口双宽
通过；主题组框原型因覆盖标题未进入产品，待提交推送及远端复核。

S54双宽真实空网格及模型通过：客户区对照无变化，水平消息滚动有效，旧包负例
检出；最终干净包16项聚焦回归无跳过，双发布及system32双宽入口通过。垂直滚动、
非空/自绘单元、编辑、物理拖拽及其他DPI仍未覆盖，已记录TODO；待提交推送复核。
