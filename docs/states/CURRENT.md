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

## M0 T18 S31 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation；S30测试效率交付及远端复核完成，T18保持开放 |
| Admission And Approval | 用户要求自动连续S实现可做控件，不新开T；既定VB延期范围不擅自改变 |
| Objective | 调查真实 Delphi TStaticText 窗口及公开协议，安全复用或补充可验证的样式适配 |
| Non-goals | 不猜测私有对象、不干预应用自绘、原图形资源和已确认外观 |
| Reference Baseline | [S26控件台账](../history/M0-T18-S26-control-audit.md)、[S30测试效率](../history/M0-T18-S30-font-efficiency.md)及现有原生STATIC适配 |
| Candidate Proposal | [Delphi Win32控件完整审计](../proposals/delphi-win32-controls.md) |
| Files And ABI Surface | 真实样本探针、必要时现有renderer和原生测试；不增加产品文件或公开ABI |
| Applicable Rules | C17、MinGW GCC、Ninja、ccache；仅自有隔离实例、有限等待和清理；兄弟仓库只读 |
| Verification | 自有实例核实类、位宽、模块、父子关系、字体及绘制；必要时双宽正负例，同批四文件实际包测试 |
| Expected Markers | 区分真实协议、已覆盖路径和无法安全识别的边界，记录实际结果 |
| Asset Needs | 登记build/t18-s31-static-text任务scratch；既有T18探针暂留交接和最终验收 |
| Reporting Requirements | 报告真实样本、适配边界、测试与遗漏；实际包、必要双发布、提交及远端复核 |
| Stop Conditions | 缺乏安全公开契约时保留原生行为并记录原因，继续独立控件工作 |
| Exit Criteria | 完成有证据的有限适配或原生保留结论、独立双宽包及治理交付；T18/M0开放 |
| Original Owner Request | 做完可实现控件并自行测试；每S按规范构建测试、发布、提交与远端复核 |
| Similar-Issue Sweep | 相似原生STATIC超类、无窗口标签和应用自绘边界，不扩张未经验证的身份 |

S31进展：当前隔离桌面可用；同批已发布包补跑VB双宽字体和控件模型等12项，
12.96秒、零skip，不冒称真实VB集成。自有cnc-ddraw副本于144 DPI正常显示，
32位进程加载assets DLL，窗口附着，移动/最小化/恢复与进程清理通过；初始页面
未观察到TStaticText，EXE中的类名仅为静态线索。探针与记录保留于登记scratch，
后续继续寻找真实实例和公开协议，不凭类名推断支持。

S31后续：scratch内官方DelphiVCL4Python 1.0.7真实64位VCL样本已创建两个
TStaticText（正常及禁用）。自有实例144 DPI，ntw16s64加载且根窗附着；两个
独立HWND的real class为TStaticText、owner为DelphiVCL.pyd，均未附着，源字体
Segoe UI；正常显示截图可见字体未改。公开文档确认TStaticText为窗口化控件，
旧台账的无窗口分组有误。跨进程观察器线程钩子返回126；改为在自有样本UI线程
直接调用原生观察器取得上述结果，目标关闭退出。真实控件绘制API、生命周期、
布局及安全接入尚需核实，不能把这次观察当作适配完成。
受控重绘期间对真实VCL模块临时观察ExtTextOutW、DrawTextW、DrawTextExW
导入（各成功接入），三者调用数均为零；导入全部恢复。此有限结果不能证明
不存在其他绘制途径，下一步继续识别框架主题/间接绘制与精确字体边界。
独立字体往返实验暂用WM_SETFONT送入既有System租约：正常控件客户区724像素
变化，恢复原HFONT后逐像素差异0，观察导入全部恢复。该实验主动更改控件字体，
只能证明此样本受字体影响，尚不能证明产品级透明、重入和布局安全。
进一步追踪确认该样本调用的WM_PAINT目标指针与当前系统注册STATIC类过程完全
相等，位于系统COMCTL32；受控原型仅在该调用期间借入字体并恢复，客户区相对
原始724像素变化且临时IAT观察全部恢复。此为单一64位样本的实验，不证明其他
VCL版本、控件变体、重入、字体消息透明及生命周期安全，不作为产品实现。
真实样本的父WM_CTLCOLORSTATIC重入负例证明朴素原型会暴露临时字体且覆盖
应用重入WM_SETFONT；scratch原型增加WM_GETFONT转发原字体和重入新字体继承后，
首次回调可见原HFONT、重入新HFONT在绘制后保留。该受控测试的后续探针自身
主动切换字体，不应把后续状态归因于应用；销毁、嵌套多个控件/线程、双宽及
产品级识别仍未验证，未改产品代码。
自有64位样本父WM_CTLCOLORSTATIC中销毁目标的负例：DestroyWindow成功、窗口
不再有效；原型跳过恢复到失效HWND、导入恢复、进程退出。另一负例在相同回调
重绘第二个TStaticText：第二窗口实际进入相同原生STATIC过程，但原型的单个
线程忙标志使其跳过字体替换。应设计逐窗口嵌套作用域和销毁/重入保护，不能
把该原型当作可交付实现；不声称已覆盖32位或其他VCL版本。
后续scratch原型改用按窗口的线程内嵌套栈：使用Segoe UI与Tahoma两个不同
HFONT的真实TStaticText，父回调嵌套重绘时两控件均采用原型且查询各自源字体；
嵌套栈改动后首次重入设字体与销毁负例重测通过，导入恢复、进程退出。仅为
有限64位原型，未覆盖跨线程、窗口句柄复用、负面类身份及完整产品交付。
已从官方渠道将Python 3.13.1 x86免安装包及DelphiVCL4Python 1.0.7 x86轮子
下载到登记scratch：解释器签名有效，VCL pyd未签名；不安装、不作为产品依赖。
32位真实VCL两个不同原字体控件也完成嵌套、重入设字体及销毁负例，过程与
导入恢复、进程退出。另在自有进程注册同名TStaticText伪类，真实类owner为VCL
pyd、伪类owner为探针DLL；受控VCL模块导入原型只作用于真实控件，伪类未附着。
此负例没有证明可在产品创建时可靠识别所有同名伪类，句柄复用仍未覆盖。
scratch原型增加仅本控件生命周期的作用域标识：32位目标销毁后匹配为0，未向
失效窗口写回；双控件嵌套时均匹配1并完成恢复。未实际制造相同HWND复用，且
属性被外部篡改、跨线程与资源寿命仍待单独验证，不能据此宣称生命周期完备。
32位标识移除负例实际发现旧原型会释放仍留在活窗口中的临时HFONT（GetObject
返回0）。scratch改以子类实例数据验证身份后，同一负例恢复源字体且有效；
若子类本身在回调中被移除，原型保留临时租约避免悬空，但原字体未恢复且租约
没有退役时机，这是明确未解决的资源泄漏和语义缺口，不可进入产品。
双凭据scratch实验：只移除子类时剩余作用域属性验证成功，原HFONT恢复且有效；
同时移除子类和属性时身份无法确认，原型保留临时HFONT避免悬空，但恢复及
退役仍未解决。该负例发生于自有32位样本；不能把避免悬空当作资源管理通过。
继续追踪真实32位样本发现系统COMCTL32的STATIC过程经自身DrawTextW导入绘制，
源Segoe UI度量25高/110宽。受控进程内仅在该DC调用期间选择并恢复System租约，
客户区相对原始724像素变化、DC字体恢复、观察入口恢复，未修改窗口字体。
这为避开上面的窗口字体租约难题提供候选路径；仍须核实字形裁切、禁用/多行、
调用者与目标身份、嵌套、重入/销毁、双宽及无泄漏后才可提出产品实现。
后续双宽真实VCL对照：正常控件DC方案各724像素变化；禁用控件经内存DC绘制，
WindowFromDC为空，单靠HWND映射原型未生效。受控原型限定系统STATIC模块调用者
及当前测试窗口作用域后，双宽禁用控件各383像素变化，DC字体与观察导入恢复，
对照颜色集合不增加。该作用域与调用者绑定尚未移植或证明对任意嵌套、打印、
非目标绘制和资源生命周期安全，未修改产品。
32位真实窗口重入绘制同名伪STATIC的负例检出DC原型误改伪窗口；scratch以当前
绘制窗口、系统STATIC调用模块及已知样本类owner联合校验后，同一负例只对真实
窗口替换DC字体，伪窗口不再命中。样本owner限定为DelphiVCL.pyd，不能直接
外推到静态链接的Delphi EXE，产品级身份绑定仍需设计与负例验证。
新的scratch DC原型不再硬编码样本模块名：在自有VCL的CallWindowProcW路径中，
核对调用地址所属模块等于窗口类owner、被调用过程等于当前注册STATIC过程，
按线程/窗口建立嵌套作用域；DC替换同时核对系统绘制调用者和作用域。32/64位
真实VCL正常及禁用控件嵌套均命中，另一模块的同名伪类嵌套不命中。当前探针
只观察VCL pyd的导入，尚未验证静态EXE、产品trampoline共存、布局/打印、
移除/销毁及资源计数；不得把动态模块实验冒充产品覆盖。
WM_PRINTCLIENT双宽实际对照发现初版DC作用域错误地改变打印输出724像素；scratch
将作用域缩为原生WM_PAINT后，32/64位打印输出对照均0像素变化、输出DC字体
恢复，正常屏幕绘制仍724像素变化。仅验证自有内存打印DC；真实打印机、复杂
打印及动态EXE接入仍未覆盖。
DC原型双宽真实样本各重复8次正常重绘，源HFONT未变，进程GDI计数均38->36；
32位真实样本WM_CTLCOLORSTATIC重入设字体后，新HFONT在绘制后仍保留。短序列
计数不证明全部资源零泄漏，也未验证64位重入设字体或所有异常退出。
64位DC原型补验：父颜色回调内设置的新HFONT在返回后保留；另一个自有实例
回调内销毁目标成功、窗口失效，观察导入恢复且进程退出，该次没有文字DC命中。
仅覆盖销毁发生在文本输出前，不等于绘制输出中途销毁或所有异常路径已验证。
对现有cnc-ddraw/JoyToKey二进制中TStaticText长度前缀做有界扫描，各两处命中
均处于Vcl.StdCtrls类元数据邻近，未发现据此可认定为已创建实例的DFM记录；
不把类元数据当作静态链接EXE的运行证据。首轮扫描脚本错误重复读取文件，已
立即中断并改为每文件一次读取；不计为测试成功或产品性能结果。
开始编写自包含C回归tests/ntw16s-dll/vcl_static.c。最初模型使用旧版Common
Controls时报告Static，不能模拟真实样本；加入v6清单后双宽模型注册类和实际类
均为TStaticText、源HFONT不变，已发布旧包均明确因未附着退出1。此为准确的类
身份及附着负例，不证明像素、字体透明和伪类检查；测试尚未接入构建/包测试，
后续需补完整断言并实现后再执行正式包回归。
增加屏幕/WM_PRINTCLIENT像素对照时，v6模型初次在私有桌面BitBlt返回错误6；
已定位为隔离worker隐藏启动导致父窗及子窗均不可见。显式在私有桌面显示自有
窗口后，双宽未注入基线均通过：两相同控件屏幕和打印像素差异各0，源字体及
颜色回调一致；已发布旧包在双宽仍由目标未附着断言检出。测试草稿尚未接入
构建，尚无新实现的正例，不能将原生基线冒充产品通过。
S31实现中：已在候选DLL加入创建期候选观察、实际CallWindowProcW/STATIC过程
动态验证和仅WM_PAINT的DrawTextW DC字体租约；限制ASCII及安全方向，应用字体、
打印和非候选保持原生。增量双宽C模型通过屏幕599像素差异、打印0差异、回调
重设字体和伪类未认证；32/64位真实DelphiVCL样本的正常/禁用控件实际附着、
源HFONT保持，候选截图有System外观。增量包为临时混合文件，不是正式同批包；
清洁双宽、完整负例、实际包、双发布与远端复核尚未完成，不能声明交付。
代码自审发现新绘制线程局部作用域在窗口过程异常展开时尚未证明安全；当前
增量实现不得发布，需补异常清理及嵌套屏障负例。来源回执尚未随源码更新，
manifest自检当前报local content digest mismatch；待实现稳定后按来源策略更新。
后续已为窗口绘制作用域和借用的DC字体增加原生异常展开清理，加入非目标禁用
控件嵌套绘制负例，双宽增量通过。clean1因共享测试库缺少unwind链接失败，已
修正；clean2随后通过但之后发现并补入DC字体借用异常清理，故clean2不用于
交付。来源回执已更新并自检通过；clean3通过但随后调整异常引用清理和测试，
故clean3不用于交付，最新clean4正在重建。真实窗口颜色回调抛异常的隔离实验
双宽均由Windows以0xC000041D终止，不能作为异常恢复通过；该实验单列为可选
负例，正常测试不依赖。异常清理使用项目既有展开机制，实际异常复用尚未证明。

S31本地交付：[验证记录](../history/M0-T18-S31-static-text.md)；clean4双宽及
18项实际包、双发布八摘要、两个安装入口和真实双宽VCL对照通过。首批VB超时
与旧包同环境复现、后续串行通过；菜单通过但约73秒，均已列TODO。真实窗口回调
异常为0xC000041D，未证明恢复。当前待提交、推送及实际远端复核，T18保持开放。

## Identifier State

- Milestone: M0 (fixed)
- Last allocated T: 18
- Last allocated Td S: 14
- Owner allocation constraint: 用户准入T18队列首项；T18内可按其明确的阶段计划细分S，不准入其他T候选。
- Last delivery: M0 T18 S30：14项聚焦包46.32秒、双发布，f079892远端十文件复核；x64压力与全量集成留最终验收。
- Open task: T18 S31 TStaticText调查；M0/T18开放。

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
