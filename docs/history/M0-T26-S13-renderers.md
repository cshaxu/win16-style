# M0 T26 S13 呈现与图形边界终整

## 准入与基线

按用户批准14个S顺序自动准入；M0/T26不变。S12 P1 `a683cc2e7648ec97ac06704b2be48db98183a078`已实际fetch核对38个变更blob及八文件，两处发布相同；37项有限case通过、6项旧接入/实际宿主观察未通过并保留3个上版控制，不能称平台全部通过。S14接收原生hook126/计算器AFH前提、原激活和DWM债务。

[初始清单](../etc/evidence/t26-s13-renderer-audit.json)冻结171项源码/头/构建/测试根，0项已审。这是有限入口盘点，不等于全部传递调用已覆盖；遗漏消费者显式补录。这是准入时的冻结状态；当前实施和开发验证见下方能力批次及开发证据。

## 完整能力批次

1. 按path顺序审计所有既有appearance和graphics源码，复用摘要不变的既有证据，检查frame_view/style_present、control_dispatch/special_install/special_paint、present_control和window_titles实际消费者。逐项明确完整状态/字体/字形/DC/bitmap/lease所有者与保留原因；不能以私有类型为理由留下公共机制。
2. 统一既有renderer注册及完整呈现操作，清理字段指针view和空转发，让产品只提供现有风格/协议装配。沿用已交付instance/control/window/menu/dialog根；原有支持资格、年代布局/绘制和资源选择仍属于appearance，不引入未来风格占位或第二套协调器。
3. 对字体对/临时字体/borrow-return/字形缓存、VB绘制与原生preflight的正常和既有异常清理做整组梳理，复用graphics已有唯一实现；中间层不增加无限修复/重试。覆盖实际consumer、失败保留、同步重入/退役与资源释放，区分原限制与回归；禁止私有桌面/computer-use。
4. 合批增量双宽验证；整S满足边界后才一次同快照干净八文件构建，两处先发布后实际包测试、提交推送/实际远端复核及scratch清理。纯文档准入不虚构二进制发布。完成后自动S14，T26保持开放。

## 退出与非目标

公共application/access不反向依赖产品/年代头，两个已有风格相同能力真实共用；旧字段视图/副本/空接线已按有限清单处置，原资源/来源/ABI/产品UX保持，所有改动有对应实际测试/消费者和真实净代码量。未具备物理卸载不改名冒称支持。

不新增Win2000控件全套、Luna/Aero、主题文件加载器、XAML客户区或新框架支持；不扩目标名单、权限、安全设置，不修改/依赖兄弟仓库。当前S12未通过观察不是S13已修复，也不重复UAC游戏；有限终审由S14承接。

## 恢复检查与第一能力批次设计

上一goal轮为progress：S12实际代码/双宽构建/发布/测试及远端复核已改变权威状态。当前工作区干净、171冻结摘要全部相同，未发现本仓库活动构建或S12/S13测试进程；没有重启或覆盖工作。171中126项存在逐项摘要完全相同的既有审计，复用静态证据后仍复核S13接收项，不把既有分类直接等同本S完成。

第一批完整操作为字体缓存/字形映射与单源字体快照借还。VB32槽/懒分配glyph map、容量/失败/清理，以及status来源HFONT/LOGFONT/TEXTMETRIC/ASCII宽度签名、invalid/borrows及释放迁至application/graphics/font_cache。公共根直接拥有缓存，不传字段地址view；provider只给acquire/retain/release与既有字形范围，不硬编码Win3x字体生成。状态栏原替换失败即释放旧缓存的契约保持，不擅改成保留旧font；无新重试或异常恢复。

实际consumer为attachment_state/attachment、vb_text provider、global/vb_text和window_titles；删除ntw16s_vb_fonts/clear及status cache/borrow-return私有实现，调用公共根。Win3x保留绘制scope资格、旋转report过滤、stroke/System生成和ASCII支持策略；这些是具体呈现要求。字体variant池、frame_view/style_present与原生呈现/注册继续作为本S后续能力批次，不将首批当全部171完成。

新增独立公共native测试覆盖32边界/失败清理、glyph decode及来源保留、快照借还/invalid/替换失败与持有引用；既有VB字体故障/实际像素和status消费者测试保留。core-vb-font继续将公共C源码作为真实生产路径编译并使用现有forced-include分配观测，不能让移出后新allocation逃过原断言。

登记`build/t26-package/s13-font-cache-01`为S13首批开发scratch，所有者S13，用于Build-Core双宽增量日志/工具链/JUnit/结果及有限原生调用；每宽180秒、每原生测试最多35秒且job清理。先审各worker入口，禁止computer-use或创建/切换私有桌面。测试结束提炼证据后核验占用/reparse清理本目录，仅保留稳定.ninja/.ccache；本批不完整打包发布。

首批增量双宽构建及公共font-cache/font-pair的4项CTest通过，构建约12.50/17.03秒。当前scratch的development-package仅用于有界产品消费者验证：六个未改动的S12正式伙伴加本批两宽ntw16s，明确不是同快照正式交付；不发布assets/O:/winnt。验证完成即归档并清理，不复用为最终包。

首批16/16原生调用通过（每宽公共cache/pair、VB故障与32槽/32图、global文字、font isolation/stock、真实状态栏simple和stock-live）；每宽新增公共归档undefined符号门禁通过。原日志与来源/包说明已存开发证据；无在用进程/reparse后已清理开发scratch及其临时包5,604,262字节。正式assets/O:/winnt仍是S12，未把开发包称发布。

第二批将font.c内单一20px资源引用和variant链表/源font归一化/共享或隔离/引用计数下沉同一graphics组件。公共memory-font与font-pool完整根管理已发布对象的生命期；Win3x仅保留System字形/FNT构建工厂及dpi/stroke key选择。工厂保留原calloc顺序和失败局部资源清理，成功时转移完整entry，不添加额外metadata分配，也不为stock借用制造节点。原retain与try_retain边界分开保留，不新增强制卸载/池全清理功能。

`status_font_stock`夹具继续观测真正公共pool的释放路径和stock借用不改链表；既有VB forced-include分配观测覆盖公共根。新增公共pool/legacy单资源原生测试及原Font20的224字符像素/引用/GDI断言保留。新增consumer闭包包括直接编译font.c的vcl_combo_list_module，接入同一公共库。

登记`build/t26-package/s13-font-pool-01`，所有者S13，增量构建/工具链/JUnit/原生结果与开发包（仅验证，非正式交付），每宽180秒、原生每例35秒job有界；按首批同样规则归档后立即清理。完成两批字体根后再处理frame_view/style_present及renderer注册，不能提前收口。

第二批双宽增量构建及2项公共CTest通过，约14.57/20.28秒；20/20原生调用通过。原Font20在普通/dpi96均224字形39,100像素exact、GDI7/7；原VB32lease/32map/source保留和OOM、stock/isolation及真实状态栏重入/字体切换断言仍通过。每宽公共归档符号边界通过，无新增反向依赖。原始结果已提炼开发证据；第二批scratch/开发包5,610,564字节核验后回收，无活动任务或临时保留。

效率检查：16+20是两批调用次数，含重测，不宣称36个互不重叠case。两次分别合并字体cache/borrow和resource/pool完整所有者与全部实际消费者，持续复用Ninja/ccache，未按文件发包；scope不扩展到S12旧宿主/126问题。下一完整批次为frame_view/style_present/native_presentation与renderer注册，重点看实际字体借用/原生callback异常路径，不能拿spin/BitBtn断言替代。原171顺序全表、传递闭包和S13整组交付仍未完成。

第三批将控件screen present与原生print操作改为借用已有完整style_instance根及完整绘制操作，删除style_present_view的detached字段指针及独立classification上下文。操作只同步借用根/renderer，不新增附着或第二份生命周期；外层既有dispatch继续持有根，重入退役后仍检查同一根。Win3x control_dispatch直接选择实际painter并调用公共present，删除产品present_context/present_painter/present_group/present_control及服务表的present转发槽。原group背景、父窗口打印、dirty保存、SaveDC失败、内存DC失败直接绘制及打印顺序保持，不新增异常恢复能力。

登记build/t26-package/s13-present-01，所有者S13；双宽增量构建/CTest和有界原生实际消费者日志，构建每宽180秒、每原生调用35秒。只运行已审无私有桌面入口，归档后清理scratch，保留稳定Ninja/ccache；不做逐文件正式发包。

第三批双宽构建19.27/25.55秒、6项CTest与12次原生调用全部通过；实际control-present的父背景/兄弟像素、故障下无额外复制/GDI平衡和frame/menu DC失败/重入测试通过。公共归档两宽反向依赖门禁通过。85,079字节scratch在提炼原始日志后核验回收，正式包仍S12。

第四批合并原生控件renderer注册和短期字体lease/原生回退操作：application/control/native借用已有style_control_state与style_instance，从状态现有font operations获取/归还同一font；Win3x仅选择tab/header/progress具体绘制。删除native_presentation两文件及产品三个固定renderer槽位，保留真实原生绘制失败且附着存活时的WM_PRINTCLIENT回退和原有正常返回释放合同。不新增native异常捕获/展开承诺，也不把这些测试当异常证明。登记build/t26-package/s13-native-renderer-01用于本批双宽增量、每例35秒有界当前桌面测试与原始日志，结束后归档清理。

第四批progress实际原生/套样式矩阵需要启动器同目录包；development-package按前两批方式只用于增量自有consumer：S12六伙伴加当前两宽ntw16s，不发布、不冒称同快照正式包；本批归档后立即删除。补充同一增量树progress-v5/v6夹具target，命令/退出码/日志同属已登记scratch。

第四批双宽增量18.30/22.41秒、公共native两项CTest通过；24次原生实际调用首轮23过/1未过。x86 header-v5在原生回退像素比较出现24像素差异；S10同一断言已有间歇记录，单次S12正式包/当前候选对照均过，不能据此称复现旧版失败、找到原因或修复。首轮失败及两次对照原文保留。两宽独立native字体借还/重入换font/退役/无font与真实tab/header v5/v6普通/aware和progress四组合矩阵均有实际调用证据。公共control归档反向依赖门禁通过；无新异常展开合同。

已核验无在用进程和reparse，第四批scratch/临时开发八文件包5,663,205字节归档后回收；本S四批累计16,963,110字节回收，增量缓存和正式S12包保持。当前有限输入183项（171初始+12显式闭包）、3个真正新输出；剩余frame_view、普通/现代renderer根和window_titles/global文字注册仍由S13处理，S13/S14未收口。当前生产diff连同未跟踪新C/H计净+113行，是部分S13而非最终统计。

效率复核：本轮合并呈现/打印与实际控件消费者、原生renderer字体租约/回退与注册两组，不逐文件完整交付。直接复用已有style_instance和style_control_state/fonts，没有新建状态协调器或font ops副本；移除产品转发和两个私有文件。一次间歇失败保留并只做一对基线对照，不重复盲测或扩展到旧S12宿主/UAC问题。下一组仍是frame_view完整呈现/普通与现代共同根，不把改名字或字段回调算迁移。

第五批完整外框呈现设计：不强行合并native_frame和modern host的不同生命周期，也不复制active/menu/button状态。年代renderer操作接收完整style_instance、完整font_pair资源根与现有完整renderer上下文；用一次完整呈现状态读取代替十个字段地址。布局/配色/滚动条仍为实际年代操作。菜单测量返回后、标题文本读取返回后分别重读完整呈现状态，保留原同步重入对字体/选择/active的可观察顺序；只快照值，不新建字体或生命周期所有者。公共graphics拥有通用有界buffer/commit/fallback事务；未增加native异常处理承诺。button binding改为借用完整instance和caption-button，而非detached字段地址。普通/modern真实消费者和现有故障/像素测试合并增量验证后再进入整S交付。

登记build/t26-package/s13-frame-renderer-01，所有者S13；双宽增量每宽180秒、原生每例35秒当前桌面job有界；日志/工具链/JUnit/来源/实际结果归档后清理，仅保留稳定Ninja/ccache，不逐文件完整发布。

第五批frame_view已用git mv归为frame_renderer；原字段地址表及paint_view工厂删除，普通和modern实际使用完整renderer操作。原现代no_selection/no_keyboard两个占位字段删除，按其既有固定呈现值表达；没有复制native_frame/host状态。菜单回调返回和标题WM_GETTEXT返回后重读整个呈现状态，font_pair仍为同一实际公共资源根，状态快照不拥有font或独立生命周期。button binding只借用完整instance/caption_button，未谎称更换命中/命令年代策略为新公共协议。

双宽增量构建16.33/20.57秒、4项公共CTest和18次原生调用通过；补充标题回调测试仅增量重建两个测试exe。两宽真实WM_GETTEXT修改font/active后新font与active text color断言均通过，普通frames、menu-dispatch、公共modern provider及Win3x现代frame/layout已有实际调用。每宽公共present归档反向依赖通过，原资源保留。103,698字节scratch已核验无占用/reparse并归档回收，五批累计17,066,808字节，正式assets/O:/winnt仍S12，不是本S完整交付。

本批执行为progress：完成实际renderer接口替换和公共buffer事务，取得改变下一动作的真实重入证据。没有按文件全量交付。单人独立复核已读实际renderer/caption/普通和modern装配，确认快照读取不调用app代码且保留原回调后字体/active更新时点；Win2000按钮缓冲需直接访问像素以重映射年代色板，已使用公共style_bitmap，不能强改为无像素访问的present事务。下一组是剩余window_titles/global文字的实际资源/注册和普通menu-only异常门面，随后完成183全表，不重复既有S12接入/UAC调查。

第六批文字资源/注册：公共graphics/font_scope统一已有临时字体选择、SaveDC/原字体恢复与lease归还；正常文字路径共同使用完整thread gate而非两份depth/SelectObject清理。status/month既有native unwind直接调用公共scope end，仍先恢复DC后归还实际provider lease。公共adapters/win32/runtime_text_imports承载现有四ANSI文字API绑定和import事务，provider只给字体选择；product只保留原ThunderRT6FormDC/MSVBVM60/线程模块资格。接口为每个链接模块内既有唯一绑定，一生使用同一字体provider；不新增目标、IAT能力、物理卸载或异常恢复。常规ANSI/ExtTextOutW仍是原正常返回合同，不能宣称新增异常展开。

登记build/t26-package/s13-text-scopes-01，所有者S13；双宽增量每宽180秒，原生自有每例35秒job有界，公共字体scope独立native异常/嵌套/失败测试及真实VB/status/month消费者合批；归档后立即清理scratch/开发包，保留Ninja/ccache，不做逐文件正式发布。

第六批真实consumer补充月份和静态文字的已有native exception夹具，以及status/simple/stock和VB scoped文字。开发包仍仅为六个未改动S12伙伴加当前两宽ntw16s，用于增量自有验证，不是正式同快照八文件发布；证据归档后立即清理。菜单only门面改为既有style_dispatch_message，删除产品returned/unwind记录；实际产品夹具已增加异常后instance.calls=0、后续native结果/LastError及移除断言。

第六批首轮22原生调用20过/2未过；两宽static exception以0xC000041D退出，完整S12正式包同条件各一次对照均同码失败，保留原文，不改系统异常策略或把旧失败改记成功。新增独立runtime_text_imports测试仅链接公共库，在自有EXE的四个原生API import上使用普通Courier字体provider，验证未启用scope/屏障透明、嵌套gate和字体恢复；无具体年代库/目标fixture依赖。其增量编译仍归同一scratch，不重复整包构建。

第六批双宽增量7.66/10.32秒、4项公共CTest通过；首轮22原生调用20过/2未过，另2次独立公共文字API测试通过。两宽static-exception的0xC000041D由完整S12同一夹具各一次对照复现，原失败不计通过；不扩大异常修复范围。真实month/status/static普通/VB四组合及菜单only实际异常后的引用、后续native结果/LastError/移除均有调用原文；公共font_scope普通和native展开、saved DC、释放前恢复及嵌套gate通过。独立API测试的两宽实际PE仅系统DLL，无任何产品/具体风格DLL；公共control归档依赖门禁通过。

第六批scope/root、四API once/import事务与正常返回深度/字体恢复已实际迁入公共层。产品global/vb_text仅保留原class/module/thread资格和Win3x font provider；window_titles的三类字体borrow私有结构/清理和private depth删除，保留WINAPI参数/原trampoline/已有调用位置及支持条件。ExtTextOutW仍先恢复font，再释放临时字形文本，最后退出gate，未改变其原正常返回异常边界。普通menu-only自制returned/unwind记录删除，消费现有公共dispatch；保留必要的ABI/完整操作装配。

所有第六批结果、实际来源/PE依赖及失败对照已归档后核验清理5,629,738字节；六批合计22,696,546字节，缓存和正式S12包保持。输入192项（171初始+21显式闭包），8个新增输出；S13尚有theme_unwind与popup_owner_scope临时生命周期、其余冻结清单逐项复核及整组交付，不能称全S完成。效率决定：后续先结束这两项现有公共tracker/owner的临时scope及全表，不继续增加新的字体/错误模型；同一问题已用匹配S12证据定界，不再重复static异常盲测。

第七批剩余临时注册生命周期：将theme_unwind原有active/closing/handle与取消结算迁入既有lifecycle/theme_track；将popup_owner_scope的owned-identity、installed先清除、存活检查及按owner移除迁入既有menu/menu_owner。原API安装/移除与资格仍由产品提供，不新增重试、恢复、异常捕获或安装失败补救。public scope是实际临时所有者，owner地址仍为同一栈scope，不能换成不稳定副本。既有原生异常facade仍调用同一process-hook unwinder。

登记build/t26-package/s13-registration-scopes-01，所有者S13；双宽增量每宽180秒、每原生调用35秒job有界。公共取消/失败/重入/销毁/native unwind及真实vcl popup/菜单消费者合批验证；只用当前桌面，结果先归档再清理scratch/开发包，保留Ninja/ccache，非逐文件正式交付。

第七批公共4项CTest每宽通过，构建7.58/9.27秒。后续真实popup普通/aware及native对照使用仅验证的开发包：S12六伙伴二进制加当前ntw16s两宽，不冒称同快照正式交付。测试结束和来源/结果归档后立即清理此包与本批scratch。

第七批归档结果：双宽8项CTest、16次有界原生调用全部通过，包含公共theme open/close取消、popup identity清除前置/重入/销毁/native异常，以及真实VCL普通/aware/nested/wrong-owner/菜单数据恢复。构建约7.58/9.27秒，公共归档反向依赖检查通过。证据已保存在开发JSON，不把调用次数称为互不重复case或正式发布。

本批scratch与仅验证开发包5,607,440字节已核验归属、无占用及reparse后清理；七批共28,303,986字节，稳定缓存和正式S12包保留。当前199输入（171初始+28显式闭包）、8新增输出；137输入仍为frozen-to-review，其余含局部消费者状态，不能以已测试局部操作冒称全表完成。后续顺序复核当前摘要与历史证据，实际读取剩余呈现/注册实现及调用者，再一次完成S13整组交付。原static exception与header间歇失败记录保持，不继续盲测。

效率复核：三层架构、14S与按完整能力合批的规划保持；本轮不重复准入、不覆盖未提交成果、不启动重复构建。七批增量验证不替代S级同快照干净构建、先发布后测试及实际远端复核；剩余全表审计之前不提前发包。

本轮按冻结顺序完整读取并结算12项：Win2000 caption/frame/system_menu六文件、Win3x control_api/layout/palette五文件及attachment_guard。归属表记录实际函数、共同根与保留原因；frame的按钮像素色板和Win3x角命中分隔线是年代策略，不为迁移而增加通用回调。当前125项仍frozen-to-review；本轮是源码终审，不新增运行通过数，不触发逐文件构建或发布。

顺序续审9项基础/选择/组合绘制与资格头：basic_paint、choice_paint、choice_view、combo_paint各C/H及control_class.h。choice_view虽保留名称，但同步借用完整instance/state，没有字段地址视图；实际快照资源由公共button/combo_snapshot拥有，剩余几何/字形/父背景/字体和支持条件归年代。control_class.h含实际策略，不是可删除的include空壳。当前116项仍冻结待审，累计本轮21项源码终审；不因纯审计再构建或发包。

继续完整读取并结算control_guard/install/scope各C/H和control_state.c共7项，累计本轮28项源码终审，109项仍冻结待审。核实S10/S11历史receiver文字不代表当前未迁移：当前scope已无私有hook/list/count，control_install已无旧dialog pending指针事务；字体provider只选择具体配方/模式，共同根拥有状态和资源。现有list/other/scroll/special/text的S13资源接收项仍须逐项结算，不把局部字体scope已迁移当作整个绘制文件已完成。

本轮权威文档及映射门禁：治理13负例、来源124决策/294include/1076回执及LF/第三方负例、5原资源byte-exact和6替换负例、公共源码两根反向依赖检查与diff检查均通过。此轮未新增生产改动，无重复整包构建/发布或新scratch；当前正式包仍S12。

第八批有限图形操作：按剩余接收项审查list/other/scroll/special/text，移出other_paint的fit_push_text完整原生事务到既有graphics/text_metrics；调用者提供原4096x256源尺寸上限、字体和颜色，不改测量、分配、选择/释放、失败不发布和正常返回合同。公共层不选择Win3x资源或字体。既有public text_metrics测试增加真实缩放像素边界、DC状态/资源平衡与原生失败路径；真实按钮/group消费者合批验证。边框局部SaveDC与棋盘相位本身是绘制操作，不为此新增字段回调或泛型资源框架。

登记build/t26-package/s13-text-fit-01，S13拥有；双宽增量构建每宽180秒、每原生调用35秒有界，仅当前桌面，不创建私有桌面。原始结果与摘要先归档再清理scratch；稳定Ninja/ccache保留，本批不作完整交付。无活动gcc/cc1/ninja/cmake工作，沿用现有增量树和未提交修改。

第八批首轮x86编译发现测试枚举BITMAP与Win32类型同名，仅测试故障常量改为FIT_前缀；首次失败日志已存开发证据，不计通过。登记第二增量尝试build/t26-package/s13-text-fit-02，沿用同一稳定缓存；第一scratch归档后清理，不重新整包构建。

第八批双宽增量构建约3.97/11.64秒、2项公共CTest与8次原生调用通过，包含独立7阶段故障/边界像素/输出不发布/DC状态/GDI平衡、真正Win3x fitted-button、control-present和dc-failure。迁移前HEAD原函数与公共函数在仅重命名/上限参数化后逐字一致，两宽公共归档反向依赖通过；不增加native异常合同、不冒称正式包。两轮scratch共80,403字节已归档核验回收，八批累计28,384,389字节，正式S12与稳定缓存保持。

继续结算24项header/list/month/other/progress/scroll/special/tab源码与头，逐项说明实际年代绘制与公共根边界。局部SaveDC/brush相位不新增泛型包装，既有公共snapshot/present/bindings实际拥有共同事务；完整文本bitmap操作已真实移出而非仅转发。当前冻结待审83项、总输入199/新输出9，字体glyph解码的heap生命周期仍显式由本S接收，不冒称已全部完成。当前生产净+205行是部分S13统计。

第九批收尾原glyph接收项：公共graphics/font_cache增加完整临时glyph-text根，拥有已有decode/单字节编码两块heap缓冲及释放；Win3x保留count4096/RTL/CP1252和具体decoder选择。产品ABI叶子直接调用公共根并仅选择原生A/W路由；删除私有prepare分配实现，保持失败不发布text/options、无font时提前丢弃ANSI缓冲、调用后先还字体再释放decoded/encoded最后退gate的原顺序。只保留既有正常返回合同，不新增异常处理。

登记build/t26-package/s13-glyph-text-01，S13拥有；双宽增量每宽180秒、原生每例35秒当前桌面有界。独立资源根测试覆盖zero/decoder失败/编码拒绝/两次分配失败/早丢ANSI/最终释放，原实际VB文字和profile规则测试保留。结果归档后立即清理scratch；不新增产物/权限/框架或逐文件完整发布。

第九批双宽增量15.22/19.28秒、2项公共CTest与16次原生调用全部通过，含两次分配失败/解码编码拒绝/zero/早丢ANSI/终结次序/heap=0及实际VB bare/styled普通/aware glyph绘制。两宽font_cache归档反向依赖通过。产品已删除直接管理decoded/encoded双指针及heap释放，保持font→buffers→gate和A/W原路由；公共接口新增int原生编码长度可表达性检查，不改变调用者原4096上限或目标范围，不添加异常能力。开发包仅验证且未正式发布。scratch/开发包5,619,955字节已归档核验回收，九批总34,004,344字节，正式S12及稳定缓存保留。

本轮又结算38项toolbar/track/updown/VCL/menu、原资源及公共graphics契约，证据记录原语、完整资源所有者和实际年代策略；没有为局部SaveDC或颜色选择制造空包装。当前43项仍冻结待审，剩余普通frame/产品装配及测试映射接收清晰；199输入/9新增输出、S13整组交付仍未完成。部分S13生产净+230行，不称最终减码结果。

下一具体接收项已核对：frame_layout的measure_window仍以dpi/menu_font/detached三个字段地址借用，唯一实际调用者为产品win16_style.c。后续改为借用完整instance/font_pair，不能复制状态或新增字段访问回调；需以真实WM_MEASUREITEM回调改变DPI/字体和退役验证回调后读取时点。其余frame文件已读取但本轮未冒称终审结算，43冻结项状态保持。

第九批之后来源1077回执、124决策/294include及原资源/源码反向依赖、治理13负例和diff检查通过；old fit_push_text/vb_glyph_prepare的生产与测试旧入口搜索无命中。所有本轮构建/原生测试均终止，临时目录已回收；未提交改动和正式S12包保留。S13/T26未收口，不把开发验证当正式发布。

第十批完整外框测量借用：frame_layout改为同步借用既有完整style_instance与style_font_pair，不再接收dpi/menu_font/detached三个字段地址；删除产品仅转接这些字段的measure_window。原生owner-draw菜单测量可重入，保留原先第一次几何读取及回调后滚动条DPI/退役读取顺序，不复制根、不新增字段回调或资源模型。实际WM_MEASUREITEM夹具验证改变DPI后的计算及回调退役取消。

登记build/t26-package/s13-frame-measure-01，S13拥有；双宽增量每宽180秒、原生每例35秒job有界，仅当前桌面自有窗口。目标实际普通外框/DC/菜单及现代几何消费者合批验证；结果归档后清理scratch并保留稳定缓存，不逐文件完整发布。

第十批双宽增量约11.43/12.42秒、2项CTest与12次原生调用通过；真实WM_MEASUREITEM改变font/DPI后使用当前DPI，回调退役后取消测量。原frame_layout实现仅替换完整根访问后与HEAD逐字等价；产品measure_window转接删除。125,047字节scratch已归档核验回收，十批累计34,129,391字节，正式S12与稳定缓存保持。

顺序终审已结算199输入/9新增输出，原43冻结项及早期局部消费者状态有最终所有者、具体原因和实际验证边界。产品attachment/control/ABI/loader装配已完整读取；共同根拥有cache/lease/renderer/present/事务，保留具体资格、原trampoline与完整年代操作绑定。审计完成不表示本S发布完成。生产C/H当前+714/-484净+230，含staged移动与新文件，不隐瞒正净增量；原DWM/static异常/宿主限制不计通过。

登记build/t26-package/s13-delivery-01，S13拥有：单次完整冻结快照、干净双宽构建及八文件候选，每宽600秒总1200秒有界并后台监控。沿用ccache；构建后核验快照/ABI/来源/资源/依赖，先发布assets根与O:/winnt后运行实际包有限回归，提交推送及远端逐blob/八文件复核后清理scratch和发布备份。当前无其他构建/测试存活，S13尚不收口，S14随后准入。

登记build/t26-package/s13-publish-01用于同批八文件两处发布备份，S13拥有，实际包测试、远端复核后归档核验清理。正式自有原生测试脚本与结果保留在既有s13-delivery-01，单例35秒job、外部45秒观测有界，仅当前桌面；不使用私有桌面或computer-use。

S13 P1整组本地交付：同快照双宽干净构建约267.96/299.30秒，ccache直接命中887/975、每宽miss17；每宽4进程/23样式归档、1077来源回执、5原资源、ABI通过。1170当前生产/测试/工具输入与构建快照逐项一致。八文件已先发布assets根/O:/winnt，随后120次当前桌面自有原生调用119过/1未过/0skip；原始失败是x86 v5 header回退24像素。同一夹具配完整S12八文件的单次对照复现相同24像素，随后S13一次对照通过，第一失败仍保留，不冒称修复或120全过。

最终199输入/9新增输出已结算，生产C/H+714/-484净+230。公共根实际拥有字体cache/pool/glyph、原生renderer lease、完整present/print、text fit、frame measurement borrow、theme/popup scope；产品选择年代呈现/资格及保留兼容ABI。原static异常、DWM恢复、激活和真实宿主接入限制继续TODO/S14，不把本S有限迁移称全平台验收。见[整组交付](../etc/evidence/t26-s13-renderer-delivery.json)。下一步实际提交推送、读取远端逐blob及八文件复核，清理本S临时目录后按原规划S14，不新增T。

S13 P1 `1b1b33d34732002fe4bd1088ddfb482202a8a191`已推送并实际fetch，逐blob核对77现存变更、4删除路径与八文件摘要；S13按有限重组条件收口，原失败未转成功。后续自动S14全T终审，T26保持开放。S14仅按原规划使用本快照必要的公共测试/归档最小副本，登记build/t26-package/s14-final-audit-01；其余S13临时构建、源快照及发布备份在证据归档后立即核验清理，稳定缓存保留。

S13临时源树/干净构建/候选/自有测试/完整S12对照及两处发布备份已核验无占用及reparse后清理，共334,638,930字节。S14仅接收327个必要测试/归档/身份元数据文件29,683,976字节，不含源码树或对象；后续验证结束后同样清理。稳定.ninja/.ccache和正式八文件保留，S13不留活动worker。
