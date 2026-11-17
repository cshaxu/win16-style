# M0 T26 S10 — 扩展控件与运行时适配

## 准入与边界

S9 P1 f9de3404 已完成同快照双宽八文件发布后146项聚焦通过、两项旧DWM失败不变及实际远端复核，按批准的14个S顺序自动准入。M0固定、T26开放；本S不是新T。不重做已经公共化的基本控件状态、combo、scroll和生命周期。

目标是扩展控件的原生协议、快照、通知与实际运行时状态/资源归application/control或application/adapters/win32；Win3x只提供既有度量、字体、栅格、颜色及呈现资格。公共对象拥有身份和资源，完整操作回调代替字段指针view，实际产品调用后删除旧机制和无意义接线。不新增Win2000控件或扩大VB/VCL支持集合。

## 按文件顺序的有限审计准备

先列冻结清单和逐项接收者，再按文件名顺序审计全部本S相关文件与实际调用，不随机搬叶函数。S9明确交接：attachment_state及产品attachment的扩展候选/parent/witness/通知/TLS，choice_paint的checklist原生身份与重入，control_dispatch的tab/header/progress/trackbar/fileowner分支，special_install/notify/native_presentation的扩展协议，control_status/native_combo等既有public view所有权及对应测试。复用S9的公共instance、control_state、retire和资源对象，不能另建平行根。

创建scope、creation_begin/before/return/end、old-file-dialog pending和结果事务仍归S11；现代composed-frame与宿主归S12；最终renderer注册及通用图形fit/present资源归S13。冻结时列具体函数和真实消费位置，不以“风格/业务逻辑”包揽这些机制。S10的身份和绘制支持政策不得改变。

## 实施与验收

按完整能力合批：先扩展状态/通知生命周期，再原生项目/文本快照及编辑/呈现协议，最后VB/VCL适配与产品装配精简。批次可按冻结审计发现合并；不预称每批都需完整交付。开发使用稳定.ninja/.ccache的双宽增量和相关原生测试；同一公共协议可由独立无Win3x测试使用，实际Win3x消费者必须调用同一实现。

既有原生返回/LastError、应用自绘、文本/图片/数据/字体所有权、通知时序、换父/角色变化、重入解除/销毁和失败清理维持。使用自有当前桌面目标、有界等待和清理；禁止computer-use和私有桌面，不修改或依赖兄弟仓库。默认使用isolated_process夹具前检查显式--private-worker提前返回路径，不重复历史desktop违规。

全部有限清单落实后一次干净同快照双宽八文件打包，先发布assets根与O:/winnt，再实际包回归、提交推送及实际远端读取。保持原资源/来源/ABI，失败或环境限制如实记录；只停用不能称物理卸载。每批或约30分钟复核效率并立即清理已终态scratch，保留增量缓存。不逐文件发布。退出条件满足后自动S11，T26保持开放。

## 状态

S10 active。扩展状态、快照、通知、运行时观察/消息及公共attachment根已分批公共化并接入，最新实现与原始测试结果见下方连续记录及审计/开发证据。冻结309项输入的首次逐项复核已完成，23项过渡归属已按当前源码终整并明确后续S接收者；未发布失败清理已归公共publish事务，其他partial与整批交付仍待完成，不能称本S完成。正式包仍为S9，整S审计和统一交付尚未完成。

## 冻结清单与第一组实施

已冻结309项生产/头/测试及构建输入，见[有限清单](../etc/evidence/t26-s10-controls-audit.json)。包含完整appearance/control和已有public control及实际产品调用者，并按文件顺序审计，不把S9已有通过自动标为本S已review。根attachment_guard/state含可共享的扩展状态/原生见证/重入与退场，需与绘制/资格分离；其整体迁移仍待完成。

第一组在该顺序的choice_paint明确分离运行库协议：VCL checklist的DC/模块/原生项目区域校验和VB5/VB6的既有44x11来源位图、11x11 blit、选中/启用偏移、原生项位置及同步查询重入归application/adapters/win32；年代最大绘制范围与具体glyph留Win3x，提供完整paint操作。产品BitBlt观察直接调用公共适配器，删除私有协议入口，不加转发包装或扩大支持集合。随后继续根状态/通知与扩展快照合批，不为这个文件完整交付。

登记build/t26-package/s10-checklist为双宽all增量/公共原生checklist契约及已有基础控件回归，日志run.log/run.err/result.json，Worker每宽600秒；实际VB/VCL自有当前桌面回归在同组完成后执行并提炼清理。最终S10整体交付前不更新正式八文件。

首批common checklist与独立无风格runtime fixture双宽all增量及每宽9项通过（构建约14/18秒）；来源990回执与边界通过。登记build/t26-package/s10-checklist-runtime，同一增量八文件开发包，仅自有当前桌面VCL checklist与VB5/VB6普通及144-DPI夹具；所有isolated_process入口显式--private-worker，child直达路径已读，无desktop创建/切换。各case有界，结束提炼清理；144-DPI系统条件不满足时单列skip，不计通过。

下一连续能力为原生color-combo/date-picker见证状态：将candidate/attempted/installed、borrowed parent/module真正置于公共对象，公共实现执行首次paint查询/提交；身份与renderer改为借用整个instance和公共状态，删除产品字段view及纯查询转发。原生按钮区域绘制仍由Win3x提供，status和外层overlay事务继续在同S后续处理，不以局部完成声称整根已迁移。登记build/t26-package/s10-native-combo，双宽all增量及公共native-combo、checklist和根状态回归；首批实际checklist验证终态前不重建其使用中的.ninja产物。

Checklist首批实际VCL/VB5/VB6及144-DPI两宽10调用全部通过，无skip；开发累计28去重调用，独立公共测试没有加载Win3x。两个已终态scratch已核验无存活自有进程及reparse，提炼清理。继续native-combo公共状态事务，尚未使用其新二进制验证，不能把前批结果当作后批通过。首轮来源门禁因TEST_MAP在刷新后修改而拒绝旧receipt，已按全部改动完成后刷新纠正；没有修改门禁或原资源。

Native-combo第二组双宽all增量及每宽10项通过（约25/35秒），公共对象验证成功/失败/一次尝试/查询中detached和LastError。登记build/t26-package/s10-native-runtime为同批八文件开发包的color/date模块真实绘制回归，两宽4调用，显式当前桌面worker、每项60秒；验证既有像素/clip/DC/异常/反复资源/销毁，不使用computer-use或私有桌面。结束提炼清理，不作正式发布。

## 第二组复核与效率检查

双宽all增量及每宽10项聚焦通过，随后同批开发八文件的color/date真实模块两宽4调用全部通过：像素变化仅在原生按钮区域、clip/DC不变、原生内容/输入、异常展开、反复区域资源、回调中remove/destroy均保持。当前累计34个去重调用（20个CTest架构/用例对及14个实际产品调用），无skip。公共control/instance两宽归档边界通过，原资源/来源990回执未变。

源码复核确认：公共native state真实持有parent/module/attempt/install，非字段view；public settle执行旧的观察提交及LastError，不新增重试或修复分支。产品删除两处查询纯转发；原来color→status→date顺序保持。private native_combo_identity仍混有原生按钮快照，outer overlay/clip仍在attachment_guard，后续同S继续；不因本组通过声明整根完成。

本轮两个能力组各做双宽增量和对应实际消费者，未做任何正式发布/干净构建；所有4个已终态scratch均提炼清理，保留.ninja/.ccache及S9正式八文件。首轮来源receipt更新顺序错误已记录并纠正。下一能力批次为tab/header/progress共同消息、版本和按下状态及原生项目快照，再整合扩展通知/运行时根；按已冻结清单和S1逐文件设计推进，完整S10退出前只做一次必要整批交付。

## 原生扩展状态/消息/快照批次

按已冻结header/tab/progress及control_state/dispatch/native_presentation消费者实施一组能力：公共native状态拥有epoch/pressed，统一三族原生消息执行与native变更分类；支持资格与完整render操作由profile提供。保持progress支持查询时机、header命中查询和重入退场、WM_PRINT借用DC及native结果。header临时文字/项目与progress范围/状态/颜色原生快照公共化，profile保留格式支持掩码、布局和画法，删除旧change及消息实现。

登记build/t26-package/s10-native-controls为双宽all增量和独立公共native/header/progress快照及已有native-combo/checklist/root回归，Worker每宽600秒，run.log/run.err/result.json；完成后同批自有当前桌面实际控件组合验证，提炼并立即清理。只做增量，不作单文件完整交付。

原生批次首轮在header-owner夹具编译失败：它原来直接包含自足profile源码，迁移后需要公共header_snapshot/native状态的包含与链接依赖。已给该测试显式链接style-control-common，保留原16项分配失败/退场/DC断言；生产编译无此错误，不计测试通过。失败证据提炼并立即清理。登记build/t26-package/s10-native-contract继续相同增量，不重新做干净构建。

统一消息与header/progress快照双宽all增量及每宽10项通过。登记build/t26-package/s10-native-surface为同批八文件开发包的实际控制器/绘制回归：每宽header-owner、control-extensions、v5/v6 tab与header各default/aware、v5/v6 progress矩阵及control-present，共26个原生调用；均显式--private-worker，progress矩阵child分支和helper已确认无desktop设置。每项有界，结束提炼清理，消息/DPI模型不冒称物理键鼠验收。不正式发布。

实际native-surface已终态：26项调用25通过，x86 v5 header默认DPI在特殊格式回退的native_hash相等断言失败；同一二进制夹具/S9正式DLL对照通过，其余header版本/aware及两宽故障退场、progress/tab/present通过。失败保留，不算整组通过。新增仅在失败时输出style/format/两边字体及前8差异像素的测试诊断，不改断言或产品；登记build/t26-package/s10-header-diagnostic，增量仅构建header v5/v6测试，随后有限同条件基线/候选配对，不重复其余通过项。

失败专用诊断的像素计数语句被-Werror判定缩进歧义，已独立换行；只修测试格式，无断言/产品变化，原编译失败已提炼清理。登记build/t26-package/s10-header-contract继续目标化增量。native-surface的八文件最小集合暂留用于同摘要配对诊断，结束立即清理。

Header失败限定诊断的两宽测试目标编译/ABI通过。三对候选/S9同条件header-v5调用全部通过，无产品代码修复，因此首次差异仍是未解释的间歇观察，不能称修复或用重跑覆盖。只读CIM未发现global控制器。native-surface其余25个实际调用均通过，新公共native/header/progress两宽6个契约调用通过；本S累计65项已验证去重调用，首次header失败单列。所有已终态scratch/诊断包已提炼清理；不再重复盲测，S10最终兼容性审计仍须复核该项。

登记build/t26-package/s10-tab-snapshot为下一连续能力的双宽all增量：tab消息前段已有公共state/controller，现将page/child裁剪、原生几何/项文字/借用image-list、两遍选中项迭代与DC/文本释放移入公共tab_snapshot，profile只提供surface/typeface/item三种完整呈现操作。保持获取借用image-list后不再发送应用消息的时序，保留v5/v6/native回退。结束后提炼清理，再做相关tab实际路径，不逐文件完整发布。

Tab事务首轮仅新增独立测试的Unicode初始化for语句触发-Werror缩进警告，生产源码编译通过；拆行修正，无断言变化。原失败已提炼清理，登记build/t26-package/s10-tab-contract继续双宽增量和公共契约，不重做干净构建。

Tab公共事务与实际profile消费已完成双宽all增量（约9/15秒）及每宽7个契约/ABI通过。登记build/t26-package/s10-tab-runtime为本批八文件开发副本和v5/v6/default/aware实际tab调用，共8项；入口已核实显式--private-worker在任何desktop操作前返回，原生返回/64字节item-extra/图片/页区域/通知取消/字体/销毁断言保持。每项60秒，当前桌面自有窗口，完成提炼清理；不更新正式发布包。

Tab能力复核：公共层实际持有原生读取、page/child裁剪、两遍项目迭代、临时Unicode文字和DC恢复；三个完整呈现阶段保留旧查询顺序，借用image-list后没有新增应用消息。Win3x旧采集/遍历路径已删除，保留度量/颜色/字体和栅格。双宽all增量与14项公共/ABI通过，实际v5/v6/default/aware八调用全部通过，既有64字节item-extra/页内应用像素/图片/通知取消/晚期字体及查询销毁断言不变。新增独立tab两宽使本S去重调用67（既有八项重测不重复计数）；此前表头间歇差异仍单列。两处scratch均核验无存活占用并提炼清理。来源1002回执、原资源、公共源码边界及治理13负例通过；本批没有干净完整交付或正式发布。下一步继续同S的扩展根/通知/状态栏对象，S11及之后范围不提前称完成。

## 状态栏对象与快照批次

恢复核验无活动构建或scratch夹具；沿用全部S10在途改动。下一组删除status identity/snapshot字段指针view：公共状态持candidate/attempt/install、borrowed parent/module及epoch；公共快照拥有固定文字/分区数组，公共函数执行旧身份、首次paint提交和原生采集。profile接收完整instance/state/snapshot，不将私有attachment传给公共采集回调。现有别名资格、字体与黑框呈现保留年代策略。登记build/t26-package/s10-status-object，双宽all增量及公共status v5/v6、native与snapshot契约；Worker每宽600秒，结束提炼清理，再执行相关实际status组合。不改变正式发布包或支持范围。

Status对象双宽all增量及每宽7项公共/ABI通过（约14/22秒），无skip。登记build/t26-package/s10-status-runtime为同批八文件开发包的两宽原生status v5/v6/default/aware、VCL模块default/aware、simple、pane、pane-font异常/重入回归，共18调用；全部显式--private-worker提前绕过desktop代码，transitive异常/import-swap仅当前自有进程，等待每项60秒并清理自有子树。原生像素/通知/字体/借用数据/恢复断言保持，不作正式发布。

实际status驱动首轮在创建进程前拒绝Popen的capture_output参数（该参数仅subprocess.run支持），未启动任何夹具；改为stdout/stderr PIPE后沿用同一开发包，不改产品或测试断言。

状态栏驱动前六项x86通过，随后case目录名与同名exe冲突，未运行后续；case目录统一加.case，读取既有result跳过已完成项，从第七项继续，不重复六项测试、不改产品。

Status对象复核：公共status_state真实拥有候选/一次尝试/安装/父模块见证/epoch，status_snapshot拥有513字文字与64分区数组；删除两个公共字段view与采集回调、产品拼装字段地址的代码。profile只借完整instance/state/snapshot。公共首次paint提交仍color→status→date且保留LastError，不新增失败重试。双宽all增量与14项公共/ABI、18项实际status调用全部通过，核对成功标记，包含原生v5/v6/default/aware、VCL模块/simple/pane/pane-font、clip/owner-draw/查询修改销毁/移除与异常展开。累计89去重调用，旧header间歇差异仍单列；来源1002/原资源/公共源码边界通过。两个scratch核实无存活占用并提炼清理，仍未作S10正式发布。通知/TLS作用域与其余扩展根继续同S推进，不能将状态对象迁移称全部生命周期完成。

## 轨迹条/工具栏通知批次

按冻结notify_message/control_state/special_install及产品controls消费链，将父子绑定、private-stage协商、runtime-track pending/post匹配与原生派发作用域移入application/control/notify。公共对象拥有parent/window/internal，实例引用与native LastError原序保持；profile只提供supported/part/runtime-border绘制。删除旧profile通知文件，不保留空转发；native dispatch异常恢复继续用同一公共unwind。登记build/t26-package/s10-notify，双宽增量all及custom-draw/status/native契约，Worker每宽600秒；后续相关自有实际track/toolbar回归，结束提炼清理，不作叶文件完整发布。

Notify首轮编译暴露产品controls直接提供toolbar painter后缺少显式toolbar_paint.h（旧接口由notify_message.c内部包含），已补充真实消费者依赖，未改行为/断言。编译失败未启动原生测试，证据提炼且scratch清理；登记build/t26-package/s10-notify-contract继续同组增量。

独立notify测试局部名accept与Windows winsock声明冲突，改为paint_supported；生产编译通过，未改断言。失败提炼清理，登记build/t26-package/s10-notify-validation继续同组增量，不重建已经有效的对象。

Notify公共化双宽all增量及每宽6项契约/ABI通过，独立测试实际验证native异常展开恢复原scope。登记build/t26-package/s10-notify-runtime，同批八文件及自有track v5/v6四子矩阵、VCL track/toolbar模块，共8个顶层调用；显式--private-worker，control_matrix的四child只当前桌面原生创建和自有Job清理，无desktop操作。每调用60秒、结束提炼清理；真实输入/所有第三方控件不由消息模型推断。

Notify能力复核：公共notify_state拥有绑定窗口/父窗/internal，notify执行共同的引用、通知来源、应用返回和私有绘制阶段、runtime-track item/DC/rect/pending/post匹配；track_dispatch用唯一公共unwind恢复调用方提供的GUI线程scope链。special_install/retire与产品controls直接消费，旧notify_message.c/h删除，profile只提供资格和实际画法。双宽all增量及12项公共/ABI通过，独立native异常展开验证通过；同批八文件开发包的track v5/v6四child矩阵及VCL track/toolbar共8顶层调用全部通过成功标记（nested/apppost/appskip/clip/disabled/reentrant-remove/destroy/reparent/cap64/65）。本S累计101去重调用，旧header间歇失败仍开放。来源1003/原资源/两宽control-common+instance归档边界/治理13负例/diff通过。两轮compile失败与驱动修正均保留，终态scratch全部提炼清理；正式包仍S9。

效率复核：本轮状态栏和通知按完整能力合批，沿用同一.ninja/.ccache；编译失败只修显式依赖/测试命名，不推翻已验证对象，没有做任何干净包发布。下一批需处理native_combo_identity中的原生按钮采集/clip事务与扩展运行时根和drawing-scope；仍待实施的机制不标era-only。T26/S10继续开放，按S10全部退出条件统一交付。

## 原生按钮快照与overlay事务

恢复确认无活动构建或scratch夹具。沿冻结native_combo_identity/combo_paint/attachment_guard及产品attachment链，颜色/日期原生按钮bounds/pressed采集归公共native-combo，GetUpdateRect/借用print DC裁剪/临时GetDC/释放归公共native-overlay对象；profile保留DPI/箭头/边框/默认palette策略。删除私有身份文件、native_services及两个产品color/date转发。color→status→date首次提交顺序不变，异常路径清理同一overlay-owned区域。登记build/t26-package/s10-native-overlay：双宽all增量和native-combo/overlay/status/native/notify契约，Worker每宽600秒；随后实际color/date模块原断言，终态提炼清理。不作单文件完整发布。

Overlay生产编译通过，首轮独立测试在直接观察已释放GDI区域type时失败（无GdiFlush），同宽其余五契约通过。测试改为调用结束立即保存LastError，再GdiFlush观察精确region，与已有date精确资源观察顺序一致；保留失败，不改产品或放宽资源断言。终态scratch提炼清理，登记build/t26-package/s10-overlay-contract继续增量。

加入GdiFlush后精确region/DC/pressed/查询退场断言通过；下一不同失败是隐藏原生combo没有update region，不能用于验证paint DC前提。将自有父子窗口设WS_VISIBLE且置屏幕外，不激活、不切换桌面，保留GetUpdateRect断言，生产代码不变。终态证据提炼清理，登记build/t26-package/s10-overlay-validation继续增量。

Overlay独立契约在修正夹具条件后双宽all增量与每宽6项全部通过，精确region/DC状态/借用print/临时paint/LastError/查询退场保持。登记build/t26-package/s10-overlay-runtime为同批八文件开发包的两宽color、drive-combo与date模块，共6个实际调用；显式--private-worker，vcl_capture/import_swap/exception_probe无desktop操作。每项60秒、当前桌面自有实例，原像素/clip/DC/异常/精确region泄漏负例与反复GDI断言保留。终态提炼清理，不正式发布。

Overlay批次复核：双宽all增量与12项公共/ABI、两宽color/drive/date六个实际调用全部通过成功标记，含像素只限按钮、原生内容/输入/借用字体、clip/DC、异常展开、精确region泄漏负例、重复GDI以及查询remove/destroy。公共native-button read拥有原生几何/pressed；native-overlay拥有update/clip、borrowed print DC及临时paint DC和结束释放。旧native_combo_identity.c/h、native_services与产品color/date转发删除；Win3x保留原DPI/箭头/边框绘制。累计105去重调用（旧color/date四项重测不重复计数），两项夹具条件失败如实保留，没有修改产品掩盖失败。两个终态scratch已核实无存活占用且提炼清理，正式包仍S9。下一步扩展运行时根的protocol安装事务与实际state对象，不把overlay局部完成称整S完成。

## 运行时protocol安装事务批次

按冻结attachment_state/guard及产品attachment链合并group-button、scroll、tab、track、toolbar的candidate/attempted/installed、parent/module/witness和唯一protocol所有者到公共runtime对象；公共实现执行既有绑定、一次尝试、原生查询退场、风格install回调、失败remove/协议释放及提交重绘。产品仅按角色选择已有绘制器。保留scroll的前后parent检查、group成功后绑定parent、tab/toolbar/track原有检查差异，不新增统一重试或扩大支持。edit特殊字体/主题事务和report后续同S审计，不用当前耦合认定应保留。登记build/t26-package/s10-runtime-binding：双宽all增量及独立runtime契约、protocol/notify/native回归，每宽600秒，终态提炼清理。

Runtime首轮新static helper名bind与Windows winsock声明冲突，统一加runtime_前缀；本轮应直接避免系统常用函数名，不更改头包含范围来掩盖。原失败提炼清理，登记build/t26-package/s10-runtime-contract继续增量，无行为或断言变化。

Runtime公共对象/安装事务双宽all增量及每宽6契约/ABI通过，原生无风格测试含真实protocol、重入、bind中失效、apply失败清理、apply中失效后root保留/释放和LastError。登记build/t26-package/s10-runtime-modules为同批八文件开发包两宽group/radio/push/check、scroll default/aware、page/notebook/tab、track与toolbar模块，共22顶层调用；入口全部显式--private-worker，当前自有实例每项60秒，成功标记和原断言保留，终态提炼清理。

Runtime事务复核：五角色state和唯一protocol由公共runtime实际拥有；公共settle执行binder、attempt/parent顺序、风格install、失败remove/release和提交Invalidate，dispose负责protocol最终释放。产品runtime_control_install只选择已有renderer；旧post_toolbar与release_failed_protocol及四组重复事务删除。scroll前/后parent检查、group成功后绑定parent、tab/track/toolbar原查询差异和toolbar→其他post→group/scroll→edit/report→tab/track顺序保持。双宽all增量及12项公共/ABI通过；实际group/radio/push/check、scroll default/aware、page/notebook/tab、track/toolbar两宽22调用全部通过原标记与原断言。累计127去重调用（已测track/toolbar四项不重复计数），旧header间歇差异继续开放。源码1007回执、原资源、双宽control-common/instance归档边界通过。两处scratch已核验无存活占用且提炼清理，正式包仍S9。

本轮效率复核：overlay与runtime根按完整能力合批，开发一直使用稳定Ninja/ccache增量树，未生成任何单文件正式包；GDI观察顺序和可见控件前提的测试失败逐项保留并纠正，没有增加产品容错。后续须继续edit/report/grid/VB等实际state/协议与绘制关联scope，不能因已迁移五角色就结束S10或将剩余基础机制笼统归为业务策略。

## Edit/report运行时生命周期批次

恢复核验无活动构建与scratch夹具，沿用现有S10改动。将edit候选/首次尝试/安装/原生见证、link/combo/spinedit角色及原始style/type/parent移入同一公共runtime；沿用公共settle事务，保留combo前后角色验证与空SysLink必须重新见证的原重试语义。产品只提供既有字体/主题/滚动条应用与回退操作。report/list/tree protocol绑定、父通知子类、module/parent身份、pending/commit和失败见证重置归公共report，具体tree/frame选择由产品提供。删除profile post_verified、verified_services及report_bind/unbind重复路径。登记build/t26-package/s10-edit-report，双宽all增量/独立runtime和report契约及相关现有测试，每宽600秒，随后实际模块原回归；终态提炼清理，不作叶文件完整发布。

Edit/report组合双宽all增量与每宽7项契约/ABI通过（约13/17秒），独立report无风格native父子协议与失效/失败清理、runtime普通edit和fresh-link-witness均通过。登记build/t26-package/s10-edit-report-modules为同批八文件开发包的两宽mask/memo/link/spinedit/number/labeled/edit/report/dbcombo/combo/simple/list模块，共24顶层调用；显式--private-worker，相关入口已读且transitive native helpers无desktop操作，每项60秒。保持真实旧返回/字体/选择/只读/通知/布局/ANSI和重入恢复断言，终态提炼清理，不正式发布。

Edit/report复核：公共runtime现持edit/link/combo/spin角色状态、原style/type/parent与协议安装/失败生命周期，保留combo前后核验和SysLink fresh-witness条件；产品仅提供具体字体/主题/滚动条组合及回退。公共report持候选/见证/原生tree/list身份、protocol与parent notify绑定/提交/失败重置；旧post_verified/verified_services及profile report三函数删除。双宽all增量与14项公共/ABI、实际两宽24模块调用全部通过，含ANSI DBEdit/DBCombo、memo滚动/裁剪、link延迟fresh-witness、组合三模式/角色变更/查询换父，以及report/tree owner-draw/owner-data和换父。累计153去重调用，已有runtime扩展不重复计数。两个终态scratch核验无存活占用并提炼清理，正式包仍S9；旧header间歇观察与剩余root/scope仍开放。

## Grid/VB边缘延迟绑定批次

按冻结post_controls/root状态与实际消费者，公共edge对象持candidate/attempted/installed、parent/owner；公共事务拥有VB首次合格paint后绑定/发布状态、data绘后资格复核、grid原生scroll协议读取、失败remove与重绘。VB呈现支持资格（含Data既有DPI边界）由完整eligible操作提供，不增加控件/DPI支持；保留data/user是否绘后复核、grid先attempt后资格的原差异。移除旧post_controls，只保留尚待审计rich操作。登记build/t26-package/s10-edge：双宽all增量及独立edge/runtime/report/native契约，每宽600秒；随后实际grid/VB原回归，终态提炼清理。

Edge双宽all增量及每宽7项公共/ABI通过（约14/20秒），独立契约验证data/user原发布/重查差异、失败/失效重入、native grid scroll/query退场和LastError。登记build/t26-package/s10-edge-modules为同批八文件开发包两宽grid、VB5/VB6 user-edge、Data border/arrows及144-DPI arrows，共12顶层调用。VB驱动显式--private-worker，child直达分支在desktop前返回，自有Job/映射有界清理；DPI144确切不满足时按77+VB_DATA_DPI_SKIP记skip不计通过。每项60秒，结束提炼清理。

Edge批次复核：公共edge_state实际拥有延迟父窗/attempt/edge状态，edge_settle执行原生grid scroll检查、data/user原发布顺序和绘后重查差异、失败/失效退场及刷新。Win3x完整eligible维持既有Data DPI与支持资格；安装/移除调用既有renderer，不增加能力。profile post_controls已删除并精简为尚待拆rich路径，旧root grid/VB字段删除，真实消费者使用公共对象。双宽all增量与14项公共/ABI、12项实际grid/VB模型全部通过（含144DPI，无skip），累计167去重调用。原生模型不扩张为所有真实VB版本认证；旧header间歇失败仍开放。来源1013/原资源/双宽control-common+instance归档边界/diff通过；两处scratch核验无存活占用并提炼清理。

本轮第二角色复核实际源diff与操作顺序：edit组合角色在bind前/后及apply后均按旧位置核验，multiline主题/滚动条仅产品选择，失败先移除控件再退主题最后释放protocol；空link失败必须fresh witness。report在父notify子类成功后才发布protocol，apply中失效保持root所有权，失败见证按旧条件复位。edge数据/用户先发布edge再apply、grid验证后发布，data/user不同复核语义保持。24项edit/report及12项edge实际模块全过，无断言削弱或新增容错。开发继续按能力合批增量，所有scratch已终态清理，无完整正式交付；下一组继续rich/frame身份与绘制关联scope及VB/VCL remaining mechanisms，不提前关闭S10。

## Rich/frame身份与状态批次

恢复核验无活动构建/临时夹具。冻结rich/controlbar字段view、box身份和rich/box后置安装、group/panel/controlbar退役链，迁入application/adapters/win32/runtime_identity。公共对象拥有rich角色/parent/module/witness与frame candidate/verified/retired关系；完整身份与安装/复核/失败退场公共化。Win3x只提供既有grid renderer和month拒绝样式掩码，保留当前MCS_MULTISELECT边界。删除两个字段view、profile身份与post服务/退役转发。登记build/t26-package/s10-runtime-identity，双宽all增量和独立native身份/事务测试，每宽600秒；随后已有rich/hotkey/month/popup/box/controlbar及group/panel回归，终态提炼清理，不做叶文件完整发布。

Rich/frame公共对象与原生身份/安装事务双宽all增量及每宽7契约/ABI通过；独立测试不加载风格，覆盖module/parent/RTL、换父退役、发布顺序、重入、失败和detach以及month caller mask。复核发现一次范围替换误删邻接接线，构建前已恢复并按新root状态对齐，保留此事实。登记build/t26-package/s10-runtime-identity-modules，用同批八文件开发包两宽rich ANSI/Unicode、hotkey/month/popup、box/controlbar及group/radio/panel default/aware模型，共22调用，显式--private-worker；每项60秒、自有目标当前桌面，无desktop切换；终态提炼清理，不正式发布。

Rich/frame复核：双宽all增量构建15.5/21.3秒、14公共/ABI通过；22实际产品模型首次21通过、x64 month连续绘制像素memcmp一次失败。失败发生于移除前稳定性检查；一次S9/当前候选配对均通过且没有产品或断言变化，仍为未解释间歇项，不计通过/修复。累计190去重通过调用；旧header间歇项同时开放。公共archive两宽control-common/instance、1016来源回执、原资源和diff通过。已保存同批开发八文件摘要、原失败和配对完整输出；正式包仍S9。下一批继续绘制scope和剩余runtime角色，不把identity局部完成称整S完成。

## Group/panel绘制关联批次

沿冻结vcl_frame_policy及产品static_paint实际消费，完整scope、同模块/父窗/调用者/DC身份、group背景/文本配对及panel两笔关联归公共win32 frame_paint；公共state负责verified/parent提交，风格只提供现有完整绘图操作。移除私有字段view和三个产品提交回调，不改变原输入资格、调用顺序或失败恢复。登记build/t26-package/s10-frame-paint为双宽all增量和独立native不绘制风格的关联测试（每宽600秒）；随后现有group/radio/panel/default/aware实际回归，终态提炼清理，不作叶文件完整发布。

Group/panel公共关联双宽all增量12.6/15.6秒及16公共/ABI通过；在同一能力批次继续将ControlBar DrawEdge资格、DC/调用者核验和verified提交合入同一frame_paint，避免留下第三组字段view和产品身份转发。登记build/t26-package/s10-frame-observers为最终双宽增量/公共测试，随后同批八文件group/radio/panel/default/aware/controlbar实际模块，共10调用；终态提炼清理。

Frame observer实际模型驱动首次仅因build scratch已有x86日志目录、mkdir冲突退出，未启动任何目标。改用同一登记root内modules/x86和modules/x64独立夹具目录；不改产品或断言，继续尚未运行的10项。

Frame关联批次复核：公共frame_scope和frame_state持原生group背景/文本、panel两笔关联与成功提交；公共frame_paint执行原有module/parent/caller/DC/clip/disabled核验。ControlBar原生DrawEdge资格及提交共用runtime_identity，不保留字段view或产品身份垫片。Win3x只提供原有group/panel/black-frame完整画法，产品选择这些操作；私有vcl_frame_policy.c/h与三个提交/身份转发删除。双宽all增量12.6/17.3秒、16公共/ABI及10实际group/radio/panel/default/aware/controlbar全部通过，无skip；累计192去重通过。来源1017/23负例、第三方9负例、原资源6负例、双宽公共control-common+instance归档、治理13负例及diff通过。Diff检查曾发现私有bit余下源码末尾一空行，已仅去空行并更新receipt，不重复行为回归。

效率复核：本轮rich/frame与绘制关联按完整能力合批，持续复用Ninja/ccache；新增公共/原生验证均对应状态、原生通知或真实renderer消费，没有逐文件干净构建/发布。初始驱动mkdir失败发生于目标创建前并已纠正，不改生产断言。x64 month一次间歇像素稳定性失败及旧header失败仍开放，不作循环重跑或虚构修复。本S当前生产+1386/-1138净+248行（含新增公共文件），删除重复私有机制但不以净行数代替职责审计。开发包摘要/原输出已提炼；本轮临时包随测试结束核验清理，缓存及S9正式发布保留。下一组继续剩余cool/inplace/nav/bit/static/lookup/MDI角色及通知/绘制关联、month/spin/toolbar资源；创建/现代/最终图形分别按S11/S12/S13明确接收，不笼统推迟仍属S10的机制。整S10和T26未收口。

## 原生绘制见证批次

恢复确认无活动构建/临时夹具，保留既有S10全部改动。将observe_native、ANSI DB转发、rich和box见证识别集中到application/adapters/win32/runtime_observer，使用完整instance/runtime/observation对象；status/color/date和cool资格状态归现有runtime根，去除产品私有字段依赖。保留原逐角色查询顺序、ANSI/Unicode差异、fresh-witness/attempt门限及所有原生身份检查，不追加重试、支持范围或异常恢复。scope由调用者pin，native/module均借用，不物理卸载。登记build/t26-package/s10-native-observer，双宽all增量与独立原生无style见证测试，每宽600秒，之后相关实际控件模型合批验证；终态提炼清理，不做叶文件正式发布。

Native-observer首轮编译发现一个已被公用owned操作替代的未使用helper，以及control_link测试直接构造attachment时仍使用旧cool_candidate字段；删除旧helper并将实际测试调用者改用runtime.cool.candidate，不改断言。原失败已保存，未运行原生测试，终态scratch清理。登记build/t26-package/s10-observer-contract继续同组双宽增量，不重启已终态任务。

Observer公共测试首次因ICC_LINK_CLASS在未声明COMCTL v6时InitCommonControlsEx返回126，尚未创建窗口；同宽其余6契约通过。为该包含SysLink的测试显式复用control_extensions.rc及既有manifest链接选项，不改变产品或断言。失败证据保留，终态scratch清理；登记build/t26-package/s10-observer-native继续增量。源码对照确认提取前后observe_native的Windows API调用顺序完全一致。

Observer显式v6测试manifest后双宽all增量与每宽7契约/ABI通过，独立无style测试覆盖13原生角色、ANSI DB、rich/box/static/cool及wrong-proc/caller、attempt/detach/owned/scope拒绝。登记现有build/t26-package/s10-observer-native内modules/及package/，使用同批开发八文件对两宽color/date/status、group/scroll/tab/track/toolbar、mask/link/edit/report/dbcombo/combo、rich/box、cool/static模型合批回归，共36调用；入口显式--private-worker，当前桌面自有实例，每项60秒，终态提炼清理，不正式发布。

Actual observer模型驱动最初读取旧status证据时发现该历史schema没有marker字段，在首个目标创建前退出；对该status用例使用其已核验的VCL_STATUS_MODULE_OK明确标记，其余沿原case marker，复用空夹具目录继续。产品及原测试不变，此为驱动纠正。

Native observer批次第二角色复核：完整runtime持status/color/date/cool状态，公共observation持借用instance/runtime/owner/current/native和嵌套链，产品static/cool scope在原有unwind内发布/恢复；原生过程/module不获取新引用，不改变停用/物理卸载边界。原生观察Windows API顺序与原实现一致，ANSI DB wide拒绝、rich wide/类/module核验和box caller条件保留；旧observe_native、两个ANSI私有入口、box/rich见证和重复current helper已删除，真实产品消费公共操作。双宽all增量与14公共/ABI、36实际模块全部通过，无skip；control-link实际构造调用者的两宽原断言通过，保守累计198去重。该单独CTest最初缺capture环境，被driver在目标启动前拒绝，已记录并补齐自有capture，不改产品或断言。来源1020/23负例、第三方9负例、原资源6负例、双宽control-common/instance归档、治理13负例及diff通过。

效率复核：本轮一次迁出整个原生观察分发组及关联完整状态，沿用稳定增量树；构建失败仅修旧调用者和多余helper，SysLink测试显式v6前提；实际驱动元数据和capture前提差异均在创建目标前纠正，避免重复运行已通过案例。没有干净构建/正式发布或新增能力。下一能力批冻结为剩余runtime角色与关联实例所有权/父通知：attachment root及guard、cool_grip_identity、vcl_extra_frame和bit/static/inplace/nav/lookup/MDI实际调用链；公共操作接管完整身份/lease/绑定/退场，按原调用顺序提供renderer支持资格，不能以当前私有状态为保留理由。原月历和表头间歇失败不扩成无限排障，最终兼容性审计保留原事实。T26/S10保持开放，正式包仍S9。

## 运行时角色与关联引用批次

恢复核验无活动构建/临时夹具。冻结attachment_state/guard和产品attachment中的static/bit/inplace/nav、MDI与lookup source引用；public runtime_roles持完整角色及关联instance引用，执行原生身份/父链失效退场和同序引用归还。移除样式只处理既有protocol，不提前释放最后引用；正常析构保持先清理字体/释放根存储、再popup和MDI引用的原顺序，attach失败保持先引用后根存储。产品保留原VB资格及所持subclass协议的host-attached谓词，公共库不依赖产品或风格类型。登记build/t26-package/s10-runtime-roles，双宽all增量与独立纯C引用/原生角色回归，每宽600秒；随后相关实际MDI/lookup/nav/inplace/bit/static产品模型，终态提炼清理，不正式发布。

Runtime roles首轮双宽all增量及14公共/ABI通过（15.8/21.2秒）。第二角色复核将独立测试的关联引用实际转入runtime根后执行protocol dispose，并用同HWND真实detach/attach新实例替代直接改expected指针的负例，强化资源存活和实例身份替换证明，不改生产实现。原结果保存且scratch清理；登记build/t26-package/s10-role-lease-contract，仅双宽增量core-style-runtime-roles与对应原生运行，随后同S相关产品模型合批，不重做产品全量构建。

强化后的runtime引用契约双宽增量/原生运行通过；真实runtime根在protocol dispose后仍持关联引用，最终copy-transfer按popup/MDI顺序归还；同HWND重新附着新instance被旧关联拒绝。登记build/t26-package/s10-role-lease-contract内package与modules子目录，同批八文件开发包两宽MDI、MDI强制attach失败引用平衡、lookup-popup、inplace、navigator/media、bit、static、native popup及cool模型，共18调用。全入口显式--private-worker，MDI-ref直接使用同批DLL的私有proc RVA，不执行旧ps1中的默认desktop入口；每项60秒，自有当前桌面，无desktop切换，终态提炼清理。

Runtime根引用强化双宽通过，18项实际模型全部通过（含两宽MDI强制附着失败后calls计数不变、lookup source销毁/异常、换父、native popup与导航/BitBtn）。同一能力批继续将CoolBar父通知绑定/解绑实际复用已有notify_state/bind/unbind，去除借用report.window和产品cool_unbind；三个私有绘制入口改用完整公共instance/角色/report对象，消除字段指针view及纯公开身份函数回调接线，原Win3x绘制/支持资格及查询顺序不变。登记build/t26-package/s10-role-notify-views，复用稳定Ninja/ccache双宽all增量及完整角色相关公共测试，随后同18项实际模型合批回归；终态提炼清理，不做叶文件正式发布。

角色/引用/通知组第二角色复核：public runtime_roles拥有text/bit/inplace/nav完整状态及MDI/lookup关联instance引用；reference_matches通过原同一subclass协议查真实instance再检原生关系，guard保留VB资格之后inplace→popup→MDI→nav及终态外scroll顺序。正常根析构仍先归还字体/释放根再popup和MDI，attach失败仍先关联引用再释放根；protocol dispose不提早清引用。独立契约实际将引用置runtime根并验证同HWND替换新实例被拒，原生MDI-ref强制附着失败两宽calls计数不变。CoolBar已复用notify_state/bind/unbind、删除产品cool_unbind，不借report.window；绘制入口改完整public对象，删除三个纯身份操作回调。

最终本组双宽all增量19.9/24.0秒及16公共/ABI、18实际模型全部通过，无skip，原断言未削弱。累计212去重（同组重测不累加），来源1023/23负例、第三方9负例、原资源6负例、双宽control-common/instance归档、治理13负例和diff通过。两轮同18实际模型与包摘要、MDI私有proc RVA均已保存，不作为真实键鼠或全框架覆盖；旧月历/header间歇及DWM事实不变。

效率复核：本轮按整组角色/lease/通知做两阶段增量，没有单文件干净构建或正式发布。测试复核主动改成实际runtime根和同HWND真实实例替换，第二阶段继续复用既有notify而非自造生命周期。当前309项有限输入仍有大量frozen-to-review；必须在S10收口前逐项完成静态复核，不能用212调用替代文件覆盖。后续集中处理剩余owner/custom-draw关联scope、month/spin/DC与toolbar快照；既有公共输入和纯呈现文件按顺序完成有限复核、具体S13图形接收只列原生preflight函数，不能笼统外推。正式包仍S9；本轮临时产物终态提炼清理，M0/T26/S10保持开放。

## 顺序静态复核：basic/choice/header/progress

逐一完整读取basic_paint、choice_view、header_paint、progress_paint的C/H八项并核对实际消费者，台账逐函数记录，不由文件名或通过次数推断。basic保留具体7x9/17x17栅格、palette和pressed/corner画法，缩放/solid frame已用公共graphics。choice已有公共button_snapshot采集/文本所有权，剩余role/font选择、13/14px glyph、4px间距、disabled/focus及group背景是Win3x绘制；保留背景原生调用后的本地失效核验，不为单个GDI调用增加空包装。header快照和epoch/pressed已公共，profile只保留格式能力及border/inset/text/RTL和本地DC/snapshot cleanup；既有header间歇失败仍开放。progress原生range/pos/state/color已公共，profile只保留支持mask、配色、边框与chunk/smooth布局。八项标明保留原因、公共接收者、具体消费者及已有验证；本次没有修改源码或新增测试结论，不触发重复构建/交付。其余frozen输入及部分拆分项仍须继续审计，不能称全S完成。

## Spin缓冲/DC关联批次

冻结spin_scope.c/h、special_paint前置snapshot/compose及产品GDI观察消费；完整state/scope、克隆DC/位图关联、半区失效与采样、native shape/父protocol资格、差分保留应用glyph及结束释放归application/control/spin。root持modern/verified，scope借完整instance/state；产品只提供既有按钮面/边框绘图操作。保留64x64和16 DC资源边界、主题开始先失效、原生失败/未知主题回退及原异常unwind，不追加支持或重试。删除私有spin_scope与两个像素资源helper导出。登记build/t26-package/s10-spin，双宽all增量与独立native无style DC/bitmap/buffer契约，每宽600秒，随后既有modern/legacy/spinedit实际模型，终态提炼清理，不正式发布。

Spin首轮生产编译通过，control_link直接构造attachment的dispatch仍有旧modern_spin字段，改用runtime.spin.modern；全src/tests复扫旧字段，不改断言。失败证据保存且终态scratch清理，登记build/t26-package/s10-spin-contract继续同组增量。

Spin独立链接暴露原special_paint通过win16-common间接获得的style-bitmap依赖；新公共spin直接使用bitmap资源，已将style-bitmap明确声明为style-control-common PUBLIC依赖，不能把Win3x传递链接作为公共独立性证明。生产算法/断言不变，失败证据保存且scratch清理；登记build/t26-package/s10-spin-native继续同组增量。

Spin显式bitmap公共依赖后双宽all增量及12公共/ABI通过。相邻toolbar的bounded text/item/rect读取与遍历仍混在profile，本批合入公共toolbar_text对象和读取/支持/查找操作，policy只接受现有text-only Win3x能力；不新增image/separator支持。保留512文字缓冲、1024/64配额、重复命令与查询顺序，原生GetButton后先policy拒绝再读text。登记build/t26-package/s10-toolbar-spin，双宽all增量及独立native toolbar1024/1025边界/长文字/拒绝后不查询和spin资源测试，每宽600秒；随后同批八文件spin/modern/spinedit/toolbar共10实际调用，终态提炼清理，不正式发布。

Toolbar编译复核发现旧700/510/511 guard测试直接include私有toolbar_paint.c并调用static button；已随真实生产reader迁到公共control测试目录，链接实际style-control-common、去除fake protocol/font替身，canary移至完整snapshot之后，所有原长度/回退/guard断言保留。两个CTest入口显式--private-worker，禁止旧默认desktop入口。失败保存且scratch清理；登记build/t26-package/s10-toolbar-contract继续整组增量，并将两宽v5/v6原bounds测试纳入本轮，不以新简单测试替代旧负例。

Spin/toolbar整组双宽all增量及每宽9项公共/ABI通过，包含原v5/v6的700字符guard、510接受和511回退。登记当前build/t26-package/s10-toolbar-contract内package与modules目录，复制同批八文件，实际modern-spin、legacy spin default/aware、spinedit与VCL toolbar两宽共10调用；入口显式--private-worker，每项60秒，原glyph/未知主题/失败/clip/unwind/64/65/重复ID断言保留，终态提炼清理。

本组门禁发现唯一摘要差异是reader测试迁移后追加TEST_MAP说明晚于receipt刷新，已核对仅该文档并更新摘要，不改源码/二进制。其余原资源/公共归档/治理通过；源码门禁再次验证后保存结果。后续源码提取应先反查直接include该C文件及static reader的测试，避免只在编译时发现旧测试耦合；这是本S审计输入补充，不新增产品机制或系统配置。

Spin/toolbar第二角色复核：公共spin持完整instance/state/scope、仅拥有Heap buffers，DC/bitmap全借用；前置theme失效、选中/删除位图和嵌套DC关联、原生半区采样/差分保留glyph及失败释放均迁出，profile只给face/frame。私有spin_scope.c/h和special_paint两个像素helper已删除，native形状/容量不变。公共toolbar_text持TBBUTTON/512文字/RECT完整值，先metadata policy拒绝再query text，原有1024/64、重复命令和查找顺序保持；Win3x保留支持policy/palette/字体/实际画法，无第二套reader。

本组最终双宽all增量9.8/15.8秒及18公共/ABI通过，原v5/v6的700字符canary与510/511边界保留且实际链接公共组件；10个实际模型全部通过完整标记，含现代spin的9个彩色glyph原样保留、未知/失败主题、source clip/hole、异常返回及旧spin default/aware、reserved spinedit layout、toolbar64/65/重复ID/应用skip。累计226去重调用，重测不累加；无skip，不声称真实键鼠或所有框架。来源1027/23负例、第三方9负例、原资源6负例、双宽control-common/instance归档、治理13负例及diff通过，源和原始失败/驱动纠正如实保留。正式包仍S9。

效率复核：本轮把相邻spin资源和toolbar原生读取合并验证，复用稳定Ninja/ccache，没有干净构建/完整发布。独立链接暴露真实bitmap传递依赖并显式修复，旧bounds测试随reader迁移而非删除/放宽。编译失败提醒提取前须同时查直接include .c/static reader的测试，下一批必须把调用图补完整再改root字段；不通过重复测试替代设计。剩余owner/custom-draw关联、month原生查询/selection状态、spin/status根候选及绘制分派仍在S10；局部资源完成不称全部控件完成，明确renderer预检S13和创建/现代S11/S12接收。终态scratch提炼后清理，M0/T26/S10保持开放。

## Month原生几何/配对与查询生命周期批次

恢复核验无活动构建/临时夹具。冻结month_paint/month_text、产品month query guard及全部实际调用者；完整month状态/scope、native主题/调用者身份、单calendar几何与分阶段提交、重入/异常guard及背景/文字配对归application/control/month。Win3x保留ASCII/U+200E/128范围、96DPI字体租约和black text覆盖/箭头画法；共用title-fit几何，删除字段bool view和产品query unwind/identity垫片。保留header先提交、prev/next失败不回滚既有header的原顺序。原spin candidate/msg分派同时归其公共state/操作，避免下一次再改root。登记build/t26-package/s10-month，双宽all增量及真实v6 COMCTL别名native查询/失效/重入/异常契约、原month/font/spin相关验证，每宽600秒；随后实际模型合批回归，终态提炼清理，不正式发布。

Month首轮x86 all增量在既有模型编译时捕获_WIN32_WINNT定义晚于新公共Windows头，已调整头顺序，未改变功能。原失败日志保存于development证据。登记build/t26-package/s10-month-contract用于同批双宽all增量与公共契约验证，复用缓存，不重复干净交付。

Month组双宽all增量与16公共/ABI通过。登记s10-month-contract的package及modules子目录：同批八文件开发包、双宽month/status默认及aware/modern与legacy spin共10项实际模型，显式--private-worker且WIN16STYLE_CAPTURE_DIR指向自有case，每项60秒、终态保存清理；不正式发布，正式包仍S9。

Month组第二角色核对原逻辑：header先提交、导航失败保留header；scope guard正常/原生异常归还；selection先消耗再判断及原黑字策略保留。Spin candidate/message、status present归完整公共state，私有分派删除。双宽16公共/ABI和10实际模型均通过，既有month间歇不因此宣布修复；不计重复模型为新增覆盖。剩余绘制/TLS关联scope仍S10，正式包S9未改。文档更新脚本曾遗漏显式UTF8而中断，补齐编码后继续，未覆盖已有改动。

本批源码1030回执、23来源负例、9第三方负例、原五头/声明及6资源负例通过；control-common/instance双宽真实归档边界通过，治理13负例通过。失败与成功scratch均已保存必要结果并核验无存活进程/reparse后清理，成功组回收6,702,300字节；稳定Ninja/ccache不删。本批双宽all为5.9/33.5秒，未做单文件完整交付。去重228仅增加两个新公共month契约，不将重测当进度。剩余状态栏scope/原生final-pane查询及owner/custom draw需完整操作下沉，已检查status_frame/text/font真实入口，不能仅更名view后标完成。

## Status绘制关联与原生查询批次

冻结status_frame/text四文件、status_font与attachment/status_theme/window_titles实际消费者。公共status_paint对象持快照、借用instance/state、DC/窗口及复制的主题text关联；原生final-pane查询、caller/DC/epoch匹配与背景文字关联整体下沉，Win3x提供受限文字资格及真实字体/黑框。删除私有字段view和纯preflight文件，保留原条件顺序、查询上限、绘制时点与normal/native-unwind外层引用。登记build/t26-package/s10-status-paint（含package/modules）用于双宽增量公共v5/v6原生scope契约及相关实际status模型，每宽600秒、模型60秒、自有当前桌面，不隔离；终态提炼清理，不做叶文件发布。

Status scope首轮x86编译捕获新增测试LoadIconW与通用ANSI资源宏类型不符，已明确MAKEINTRESOURCEW；生产源码编译成功，未改行为。保存原失败；登记build/t26-package/s10-status-scope用于双宽all增量/公共v5/v6契约及同组实际模型，含package/modules，原预算和清理要求不变。

Status组双宽all增量10.2/22.8秒及16公共/ABI通过。真实v5/v6公共测试覆盖frame/native final-pane text+icon、错误调用者/观察目标、epoch/detached、主题复制文字/512边界/独立支持策略/同HWND不同scope及嵌套无匹配barrier。开发包22项实际模型全exit0，无skip；初始驱动只在stdout找marker误报simple/alias双宽4项，逐源核对确认标记在stderr，保留原始oracle及两路输出，以combined-stream核验22全通过，不重跑、不放宽模型断言。pane-font原生异常及simple重入/字体切换/退场计数原断言均通过。

第二角色终审：status_paint按完整原生快照/scope持有值，主题manager管理嵌套barrier及头部配对；公共代码不持有font或安装hook，也不依赖年代头。ASCII支持资格从原私有循环变为Win3x实际policy，独立公共测试使用Unicode正例证明未硬编码。原normal/native-unwind的模块入口保持status_theme_end配对，pane-font原异常后再画/借用回收通过。状态栏私有frame/text四文件与两个字段view确已删除，产品仅组合公共native资格和真实font/frame画法。

效率检查：本组以完整status scope/原生匹配/嵌套主题能力合并修改，稳定Ninja增量10.2/22.8秒，无单文件发布；编译一次资源宏问题与测试驱动stdout-only误判分别保存，前者修类型，后者据原始双路输出修oracle而不重跑产品。最终来源1028回执、资源五头/声明、23+9+6负例、双宽control-common/instance归档和治理13负例通过；成功scratch回收7,057,246字节，失败scratch也已清理。剩余attachment根的static/cool/box/lookup/spin/color/status七组原生调用与unwind结构已定位，反复的completed/nested/LastError及引用/TLS退场应按一组完整dispatch所有权处理；不同资源的清理顺序必须保留，不能只移一个回调或扩张成逐字段view。status_font borrow/source/metrics缓存按已指定S13图形资源契约继续，不称era-only。M0/T26/S10仍开放，正式包S9未动。

## 绘制分派调用事务批次

七组原路径均在同线程依次完成scope建立→native/嵌套调用→保存LastError→completed提交→end→恢复LastError；异常时end观察未完成并按原资源顺序退场。先将这一真实共享事务收敛到application/lifecycle/dispatch，公共栈对象持completed及唯一native-unwind注册；调用和finish均为完整操作/上下文，产品保留不同观察head和既有font/provider资源结束顺序。删除七份completed字段、unwind注册和LastError骨架，不新增错误补救、权限、支持面。原scope具体身份与头状态仍继续S10，不把这个事务提取称全根已公共。登记build/t26-package/s10-paint-dispatch（含package/modules），双宽all增量与独立真实窗口/子类默认转发、嵌套、原生异常及后续重用/LastError契约，随后相关实际模型；每宽600秒、模型60秒，终态提炼清理，不正式发布。

相似路径扫描同时命中bit owner-draw相同事务，纳入同组第八路径；删除其私有completed/unwind骨架，bit_current恢复和异常release不变。保留Box witness回写在共同调用正常返回后，normal end只恢复TLS无回调/资源操作，native异常仍不提交witness。

首轮configure发现已有access/runtime的core-style-dispatch测试同名，新建lifecycle测试改为core-style-paint-dispatch，保留原目标不变；未运行任何测试进程。原失败保存，登记build/t26-package/s10-paint-call（含package/modules）继续同组增量/契约及模型，原预算不变。后续加测试前同时核对全局CMake target和test命名，不能只核对源码路径。

第二轮x86 all增量编译通过，新增独立测试在InitCommonControlsEx(ICC_STANDARD_CLASSES)返回126，尚未进入dispatch；其他8公共/ABI通过。测试改用既有v5测试已验证的ICC_BAR_CLASSES，不改变生产。原输出已存。登记build/t26-package/s10-paint-contract（含package/modules）继续同组双宽增量与实际模型；不得重复运行原失败输入。

调用事务双宽all增量4.5/15.1秒、18公共/ABI与32实际模型全通过，无skip。覆盖cool嵌套/异常、lookup源销毁/异常、box/native witness、spin glyph/异常后绘制、color/date DC/region与资源、month、static、bit ownerdraw、status/font/native异常、rich及group/panel重入；原断言保留。第二角色要求独立测试进一步验证内层窗口异常时内外两层finish都且仅一次，先保存本组结果并清理scratch，再登记build/t26-package/s10-dispatch-unwind仅增量core-style-paint-dispatch测试目标，双宽有界15秒，不重新构建产品或重跑32模型。

强化独立测试双宽通过，native默认转发及自定义invoke下，子窗原生异常按内→外各finish一次、active归零、后续调用和LastError保持；仅增量测试目标3.5/4.7秒，未改生产或重跑32模型。原生LastTest输出已保存。顺序静态复核补齐control_class六个支持predicate、control_guard、control_lifetime、control_scope七文件：支持集合属于profile；guard原生退役与updown lease仍S10，创建filter/count已明确S11，font/scroll临时renderer组合精确交S13，不能笼统叫业务留置。

本组治理13负例、来源1031回执及23/9负例、原资源五头/说明与6负例、双宽control-common/instance边界和diff通过；公共新事务无风格反向依赖。效率不足是未提前核对全局target命名、未采用已验证v5初始化类别，分别保留失败后修正，后续直接复用既有测试初始化。终态开发包回收9,683,700字节，测试-only scratch提炼后清理，稳定Ninja/ccache保留。新增两项独立架构case计入230，重测不累加。S10仍未整S交付，T26开放、正式包S9不变。

## Guard退役与owned-instance租约批次

完整审计control_guard和control_lifetime及controls/special_install实际调用者。公共control/guard持有原生消息先转发、三种退役路径、font source提交及失效刷新顺序，接收完整instance/state和有效角色/支持资格/移除操作；Win3x仅给支持策略，17项细粒度服务表收敛为完整资格操作。已有instance提供按自身window/procedure核对当前subclass身份后借用/归还，updown安装直接绑定该公共instance，删除私有三层acquire/release接线。原updown测试自有owner回调保留，不强迫测试变成产品私有结构。登记build/t26-package/s10-control-guard（含package/modules）双宽all增量、真实BUTTON守卫/字体/forward/退役/换身份租约和实际变角色/重入/窗口销毁/Spin模型；每宽600秒、模型60秒，当前桌面、不隔离，终态提炼清理，不单文件发布。

本组首轮x86在新增guard测试cleanup同一行if/release触发misleading-indentation，已明确分行，未改生产。构建终态失败保存。将同组finish_message的native spin-after/后继state查询/font结算/最后引用归还一起移入公共control_state；只剩renderer palette组合的scrollbars_paint并入既有scroll_native，删除control_lifetime.c/h，不留下仅接线文件。登记build/t26-package/s10-control-lifetime（含package/modules）继续整组合并增量/契约和实际模型；旧失败scratch提炼清理，原预算不变。

第二轮x86 all编译通过、instance与message_finish等8公共/ABI通过；新增guard末个destroy用例假设整段角色仍无效却不会在WM_DESTROY退役，断言不符合原代码。原路径仅跳过WM_NCDESTROY，WM_DESTROY返回时IsWindow仍可为真；补齐两个明确用例：无效角色保持到destroy时先退役一次，以及转发中角色恢复有效时仅terminal解除。保存原失败与原断言，不修改生产以迎合测试，新增首种结果trace。登记build/t26-package/s10-guard-contract（含package/modules）继续同组双宽all增量，预算和清理契约不变。

Guard/owned lease/message_finish整组第二角色复核：原3条native-forward/退役分支与8项有效角色短路次序不变，font source由既有public state provider提供；native BUTTON外层remove失败才fallback，普通失效角色失败不擅自fallback。instance的window/procedure就是install_frame查获的同一control_proc身份，完整对象first-member地址与旧owner一致；adapter自有owner测试仍通过。后继字体使用实际当前subclass的公开state lookup，未新增私有布局假设；font/paint结束后才最后release。删除control_lifetime两文件及acquire三层接线，原palette滚动组合并回scroll_native；special_install余下事务已完整读取并明确留S10继续。

双宽all增量7.0/124.9秒及18公共/ABI、32实际模型全通过，无skip。x64公共instance/control_state头触发287条相关编译链接依赖，沿用Ninja/ccache而非clean；效率改进：下一组开发构建只选受影响五产品、公共契约及实际模型/fixture目标，避免不相关测试重链接；整S退出才干净all交付。本次两处新增测试问题分别保留编译失败和不符WM_DESTROY语义的断言，纠正测试而未改生产以迎合。独立新guard架构两case计入232，旧模型重复不累加。来源1032回执、23/9来源负例、原资源及6负例、治理13负例和diff已通过，归档门禁完成后终态scratch提炼清理；正式包仍S9，T26/S10开放。

本组control-common/instance双宽真实归档及源码反向依赖门禁均通过；成功组临时包已核验无存活进程/重解析点后删除，实际回收见development cleanup；所有本S终态scratch均已清理，保留.ninja/.ccache和正式S9包。

## 原生STATIC观察与嵌套调用作用域

冻结产品static_text类型、attachment的begin/suspend/end与font资格、window_titles的CallWindowProcW路径及现有runtime_observer实际native见证操作。公共adapters/win32/runtime_native拥有完整call与每GUI线程manager，合并原begin-or-suspend：所有调用仍先执行现有native观察见证，再提交有效scope或嵌套barrier；完整instance借用替代cancelled字段指针。common end配对并用于原生unwind，current查询保留原window/detached资格，不擅自新增身份/语言支持。产品仅组合当前观察与Win3x文字provider，删除私有scope/suspend/end。登记build/t26-package/s10-native-calls（含package/modules）；双宽仅构建受影响三DLL、相关公共测试及本组模型/fixture，CLI/demo输入未变时用Ninja依赖检查/摘要证明复用；独立真实STATIC super-class调用/嵌套barrier/有效scope与原生异常退场，后续实际模型有界验证。每宽600秒、模型60秒，当前桌面，不私有桌面，终态提炼清理，不叶文件发布。

原生scope首轮仅选择12条Ninja构建，新增测试的相对exception_probe头少一层目录，已修正4层父目录并在构建前核对实际文件；生产编译成功，未运行测试。原失败保存。同批将特殊track/toolbar/grid的finish_install状态/失败移除/引用归还放入现有lifecycle/install，而非新建平行安装器；install_frame直接复用公共instance_lookup，旧私有finish删除。完整额外setup资格及画法仍由profile选择，不改变旧LastError回调副作用。登记build/t26-package/s10-native-setup（含package/modules）继续相同定向构建并新增原生setup结算矩阵、实际track/toolbar/grid相关模型，每宽600秒、模型60秒，不单文件发布。

本组14公共/ABI双宽通过，定向构建8.2/12.3秒。第二角色将只剩两条产品桥接声明的static_text.h并入既有attachment.h，使用前向声明避免污染其他消费者，实际hook/attachment各自包含公共native-call定义，删除多余私有头。登记build/t26-package/s10-native-integrated（含package/modules）做最终同组定向构建/契约和实际模型；前轮未运行模型不重复计数，终态日志先保存再清理。

本组第二角色复核：native begin即使不产生STATIC有效scope仍调用原runtime_observer保留13-role见证副作用；有效scope保留完整instance，不匹配嵌套call产生barrier，end正常/native-unwind同序恢复，current维持原window/detached资格而不新增identity条件。public测试真实STATIC superclass实际CallWindowProc，异常后head为空且后续可用；实际26模型涵盖STATIC/font、status/aware、color/date、rich/box/month/group/panel/toolbar/track及popup/grid。额外setup失败时原先callback可改变LastError的语义保留，独立矩阵确认销毁/失败移除后的所有权及最后release；不称新增异常恢复或物理卸载。

最终本组定向构建6.6/8.1秒、14公共/ABI及26实际模型全通过无skip。CLI dry-run发现仍需一条link，已按实际依赖仅relink run16s，再查run16s/demo no-work后才复制八文件开发包，保存初始失败reuse断言及最终证据；cache类型UNINITIALIZED读取问题已按实际字段解析，未调用build或改变目标。去重234仅新增native scope双宽case；原header/month间歇不宣称修复。完整读取control_dispatch：原生track消息分支仍具体接收S10公共notify，不把单行API另造空wrapper；其余公共消息组合与render registry尾项S13。后续按此完整消息能力处理，继续定向构建，不为叶文件发布。

终审来源门禁发现attachment.h原列retain，而本组有意并入私有声明；核验只有这一retain源码变化，已将source-manifest及receipt准确记录adapt及具体delta。原始blob/size/SHA留作来源，原导出ABI段与图形资源不变，不改验证器或规则以绕过；初次失败和修正原因保存。元数据变更不改已测源码/二进制。

最终来源1034回执和23/9负例通过，原五头/声明及6替换负例、双宽control-common/instance归档（含native manager和extra setup）、治理13负例和diff通过。本组成功/失败/中间公共验证scratch均已保存必要证据并核验无存活进程/reparse后清理，实际回收见development cleanup；保留稳定Ninja/ccache与正式S9包。T26/S10保持开放，后续track完整原生消息操作及其余观察头、最终renderer契约和整S交付不冒称完成。

## Trackbar完整消息与通知退役批次

冻结control_dispatch中完整track分支、产品track_paint_dispatch/track_notify和全部公共notify调用者。现有notify拥有track manager（替代head字段指针）、原生调用scope和完整消息操作：先native/runtime forwarding，再WM_STYLECHANGED缓存刷新，再换父owned unbind与刷新，保留逐步detached检查及原callback owner契约。产品删除track转发和dispatcher私有消息段，风格侧仅选择实际track路径/notify呈现。登记build/t26-package/s10-track-message（含package/modules），定向双宽三DLL、public manager/native track v5/v6与原通知unwind契约、相关实际track及toolbar/role模型；CLI/demo按实际Ninja依赖补齐；每宽600秒、模型60秒，当前桌面、不隔离，终态提炼清理，不单文件发布。

Track整组首轮定向构建20.0/27.0秒及14公共/ABI全通过，无失败。第二角色全仓查仅一个私有dispatcher服务构造，class_is/link/scroll/combo/list/edit六callback都恒等于公共函数，已改为直接调用并删表项，不把固定能力伪装可配置策略；其余renderer/input操作保留。登记build/t26-package/s10-track-integrated（含package/modules）最后同组定向构建和实际原生/runtime/通用控件消费者，原预算/清理契约不变。开发过程中源审与第二角色清理可更早合并到首轮构建前，下一批先查完具体消费者和固定转发再编译，减少中间配置。

Track整组终审：native/runtime调用→style-cache→parent changed unbind/cache顺序与逐步detached检查保留，显式notify owner使公共操作不假设instance与owner同地址；parent changed仅撤销本owner原binding，不重绑、不扩大目标。公共track manager持整个head，不借字段指针view；既有通知结果/LastError/native-unwind断言保留。6分类callback全仓只有实际公共函数赋值，删表项后直接调用原函数，支持集合/查询顺序不变。双宽定向5.6/7.1秒及14公共/ABI、28实际模型全通过无skip；CLI按依赖只link1次后no-work才打八文件开发包。

逐源读取邻接cool_grip_identity与fileowner_message四文件：前者native rebar probe/每次失效检查与geometry分阶段，后者native-before-overlay/DC与打印时序仍须S10公共化，各自具体边界已记录，不把剩余私有算法称业务。新增4个native v5/v6架构case计入238；重复模型不累加，旧间歇/DWM未解。来源1035回执、23/9负例、原图形五头/说明与6负例、双宽归档及治理13负例通过。效率：首轮前应完成固定转调扫描，避免二次配置；后续继续完整能力合批、定向增量、整S退出再全量发布。临时包证据保存后按约定清理，正式包仍S9，M0/T26/S10开放。

Track本组全部终态scratch已提炼并核验无活进程/重解析点后清理，成功组回收8,869,033字节，稳定Ninja/ccache保留。源码/ABI及公共依赖门禁不等于整T完成，当前仍按S10有限清单继续。

## Rebar原生probe与owner-draw覆盖消息批次

完整原生probe进公共control/rebar，按原次序valid→DC/flags→ID/index/rect/band/borders/colors，每次有效性检查不变；分开post-policy geometry，防止提前查询改变顺序。probe持复制值与借用handles，不把REBARBANDINFO.lpText指针带出。Win3x保留原颜色/横向/style/空文字图片/12DPI容量条件并和实际cool grip画法合并，删除私有identity C/H。owner-draw统一进既有basic完整input/native/select/render/DC事务，风格只给既有eligibility与绘制；删除fileowner_message C/H并入现有profile组合，保持print借用、border→scrollbars及0处理输入返回。登记build/t26-package/s10-rebar-owner（含package/modules），定向双宽产品/公共原生v5/v6 probe与owner覆盖契约/相关实际CoolBar、VB file/checklist、combo/owner模型。每宽600秒、模型60秒，当前桌面不隔离；保留原绘图资源/声明，不扩大支持或新异常恢复，终态提炼清理，不单文件发布。

本组首轮生产编译发现已删fileowner私有头仍有两处include及dispatcher缺直接control_class声明；已保存失败并逐库/产品/测试扫清。登记build/t26-package/s10-rebar-owner-fixed（含package/modules）沿用有效对象增量续作，不覆盖首轮结果。不改变行为或门禁。

新增原生test漏掉既有_WIN32_WINNT定义，生产已编译而test失败，已并入既有native测试循环统一定义。登记build/t26-package/s10-rebar-owner-native（含package/modules）继续定向增量；两次编译配置失败保留，不算通过。

本组最终双宽定向增量3.6/27.5秒，14公共/ABI及20实际产品模型全部通过，无skip：REBAR v5/v6原生query顺序/失效/DC/几何、owner native-first/owned或borrowed DC与detach/destroy重入，以及CoolBar异常/换父/退役、VB5/6 file/checklist、三种combo/popup/owner转换。失败含测试helper命名及销毁期间非目标消息被观测计数，已保留并修正测试，未改生产政策。八文件仅开发同摘要包，CLI已实际link及no-work复核后复制；累计244去重，模型重复不累加。两个原生query/overlay旧私有C/H共4删除，Win3x实际画法合并现有special_paint/control_dispatch，完整对照旧源调用/查询顺序；既有异常恢复范围不扩展。后续updown仍有native registry/DC clone生命周期需公共化，不将其当画法。

三个本组终态scratch已提炼并用原生CIM确认无自有进程、核验精确目录及上下级无reparse后清理，合计7,839,476字节。保留.ninja/.ccache及S9正式包。效率复核：实际受影响增量而非叶文件全量交付；编译配置/测试观测失误已归档，后续在首轮前核对删除头全部消费者与新test既有编译定义，不重复发布或新分配S。

## Updown 原生登记与 DC 关联批次

审完 updown_adapter C/H、updown_paint C/H及全仓生产/测试调用：TLS窗口登记、32个借用clone DC槽、来源module、scope barrier、borrow/release和退场清槽是公共机制；允许style/ex、glyph/palette和绘图属于Win3x。公共control/updown持完整原生registry并接受完整render/支持操作；resolver由调用者提供，保留FALSE但非NULL window的parent-paint barrier，不以theme handle标识窗口。API可用flag仍只在原hook安装完成后置位，行为不扩展。profile配置并入已有updown_paint，旧adapter C/H删除；所有生产hook直接消费公共API。独立公共test不链接Win3x，原updown_owner glyph/SaveDC/ref测试保持实际profile。登记build/t26-package/s10-updown-native（含package/modules）双宽定向增量、独立协议与实际updown/spin/track/toolbar消费者；每宽600秒、模型60秒，当前桌面、不隔离、不操作用户窗口，终态提炼清理，不单文件发布。

Updown本组首轮双宽定向14.8/19.1秒、14公共/ABI与12实际消费者均通过，无skip。native public测试无Win3x/私有头；保留原updown_owner实际原始glyph绘图、165DC回收、130循环/32槽/溢出、barrier、同HWND新实例拒绝旧clone和最后release退役。生产hook直接调用public scope/module/lease算法，profile只保存palette/glyph及supported/render，旧adapter C/H删除，API-ready置位及LastError包装点未变。公共target先acquire再采style/ex再查module，再调用supported；metadata不调用drawing支持策略，拒绝/成功都归还一次，退役只清元数据不删除应用DC；普通renderer返回的原范围不伪称新增异常保护。来源1036回执及原资源/双宽common+instance边界通过。八文件仅开发包，CLI actual link/no-work后复制；246去重，重复模型不累计。下一批审vb_text native/report scope与字体租约边界；完整S10交付尚未完成。

## Native/report 文本观察作用域与 theme 分类批次

完整读取vb_text C/H及attachment/guard/hook/global文字和独立font测试调用者：两条嵌套scope链、native HWND/DC metadata、report module/DC/cancel及active/direct资格是runtime观察机制，迁application/adapters/win32/runtime_text；字体租约及CP1252 glyphmap实际资源仍由Win3x fonts持有，具体S13最终资源契约接收，不把它们藏入void context供公共层解读。公共scope仅保留借用调用者context，profile给两个阶段完整资格操作，保持native链先发布→edit查询→文字链发布→角色分类原顺序。report启用后才调用原graphical-label分类，end直接公共化并在既有dispatch正常/native-unwind路径实际消费。theme_kind有界native class reader并入既有lifecycle/theme_track，窗口hook和原6断言直接接入公共入口、删旧两个私有文件。登记build/t26-package/s10-runtime-text（含package/modules）；两宽定向原生scope/嵌套barrier/cancel/report/native-unwind与实际font/VB/VCL/report模型，每宽600秒、模型60秒，当前桌面不隔离，终态提炼清理，不叶文件交付。

Runtime text本组双宽定向增量4.2/16.9秒，14公共/ABI及22实际模型全部通过，无skip。独立公共test以真实原生窗口/子类/线程验证native先发布、text发布后role、仅native erase屏障、report/DC/module/cancel/context、普通及嵌套native unwind后再次复用；actual VB font的32 lease/32 map/33 source/两次OOM及report精确DC/原stroke保留；native/aware spin、modern-spin、CoolBar、VB5/6文件/checklist与VCL report仍过。既有font获取/字符支持/GDI对象资源由Win3x保留且S13接收，不把void context交给公共层解读；scope只拥有自己的metadata和链。theme类原6断言实际迁到public测试，新注册原有normal/cancel公共CTest，删两私有文件。累计248去重，移动/重复测试不累加。

第二角色逐项核对：role策略的GetWindowLongPtr/GetClassName及module查询不派发窗口消息；begin仍先native、再edit资格、再text scope发布、再具体class资格，report字段先提交再graphical-label资格，cancel后的report_active/direct与decode原差异保留。现有八种native dispatch的end直接消费公共作用域，没有第二套current链；profile字体缓存借用自己的root context，公共层无字体私有头或字段。来源1037回执/23与9负例、原五头/说明和双宽common/instance/theme归档通过。驱动曾拼错report fixture目标、standalone font遗漏新增策略参数，失败完整保留后修正，未弱化断言或改变生产策略。全部gate不等于S10或T26已完成。

相邻11文件完整源码复核已登记：固定thumb/hit几何属于Win3x；list/native呈现仍有S13具体图形/renderer资源交界；control_install/remove的公共入口已消费，但额外reentry/setup与bindings清理顺序仍须S10完成，不能把这些机制统称产品策略。下一批按这些明确剩余推进；source审查和测试配置须合并到首轮前核对，继续稳定缓存增量、整S才全量交付。

本轮两个终态scratch均已保存原始测试、失败、包摘要和边界证据，原生CIM/精确路径及reparse复核后分别清理6,802,402与8,359,514字节；稳定Ninja/ccache保留，正式包仍S9。Runtime text另保存实际双宽工具链摘要与cache前后统计。生产当前全S10累计+2742/-2241、净+501行（含48个新C/H，排除tests/docs），不把新增公共契约当代码删除。下一轮先完成control setup/reentry与binding teardown完整操作的设计和全部实际消费者扫描，再合批编译；中途两次驱动/consumer修正暴露目标/接口核对缺口，保留失败以改进首轮覆盖，不扩大测试或反复完整发布。248去重证据不替代220个frozen输入和其他partial条目的后续逐项审计。

## 控件 bindings 完整状态与安装/退场批次

审完control_install/remove/state、instance/install/retire及16个实际control_style调用文件。公共control/bindings拥有combo/scroll/native tab-header-progress/notify track-toolbar/updown及theme ownership实际状态，不接收指向产品字段的view；profile嵌入完整对象。公共setup按旧顺序执行native combo children→font/source设置→separator framechanged→redraw→detach结算；额外font读取的cancel/重复安装/source规范化进已有control_state；GUI thread/重复预检进instance，保留零数据外来subclass也视为已存在。bindings退场保持updown→track→toolbar→terminal或ordinary combo/theme/font/caret/redraw/timer/capture顺序，theme/provider子窗口动作仍由原完整操作提供，移除只是元数据/窗口附着不卸载DLL。updown glyph/palette描述与binding本体分开，原165DC/profile测试不弱化。profile control_remove合并既有control_state，删除私有C/H；现有dialog_combo pending事务仍按S11接收。登记build/t26-package/s10-control-bindings（含package/modules），双宽定向public状态/setup/preflight/font重入/退场及实际combo、controls、track/spin/header等消费者，每宽600秒、模型60秒；当前桌面不隔离，终态提炼清理，整S才完整交付。

Bindings本组双宽定向增量20.7/75.1秒（受公共root header布局及实际painter夹具影响，未全量clean），22公共/ABI和24实际消费者全过，无skip；新独立测试以真实HWND/GETFONT重入/零data subclass和foreign thread验证预检与cancel优先级，ordinary按child→theme→font→redraw退场、terminal按theme→child，验证theme失败不替代child结果、timer归还及owned notify/updown。profile old165DC、核心lifetime重入安装/销毁、真实screen/scroll呈现及三种combo/track/toolbar/spin仍过。构建初轮仅有temporary scroll view的机械重复限定符，已保存失败、移除并全仓扫清，未改算法/断言；其余修改合批后才构建，不逐文件交付。

第二角色逐操作对照原序：所有native对象实际归bindings，不是产品字段指针view；字体读取仍先WM_GETFONT→cancel→已存在subclass→source，source provider失败/取消对象仍由原caller释放并保留错误；preflight保留原thread/duplicate检查且零data也存在；combo setup后native system lease使用现有fonts.acquire(...,TRUE)，当前profile在native=TRUE时无额外查询并直接返回原System acquire。combo_passthrough在全仓仅初装写入，因此按值传入setup/release不改变重入语义。release保留Updown→track→toolbar与terminal/ordinary差异及最后pin，动态theme/child由原完整provider提供。控件root instance仍首成员，16个typed source/test消费整体bindings；原updown profile只留palette/glyph描述，注册指向公共binding，last-owner退役清元数据不删应用DC。control_remove C/H已并入已有profile state，两个固定unbind wrapper删掉。dialog_combo pending/子项provider组装仍是S11明确接收者，不称已公共化。

来源1038回执、23/9负例、原五头/声明与6替换负例、双宽common/instance归档及治理13负例通过；八文件仅开发候选（先实际CLI依赖link再no-work后复制），standalone fixture PATH限定该包以免载入O:/winnt旧DLL。累计250去重只加新公共bindings架构case，重复模型不加。当前全S10生产+2908/-2356净+552（含50新C/H，排除tests/docs）；218 frozen输入及其他partial仍需后续review，不能凭本组通过收口S10。下一组继续attachment观察装配及剩余runtime/painter资格边界；整S满足退出再干净构建发布提交推送。

本组scratch已核验无存活自有进程/重解析点并清理9,443,971字节，保留原生测试/失败日志、八文件摘要、实际工具链/cache统计及稳定.ninja/.ccache；正式包仍S9。效率复核：根公共对象导致painter真实消费者重编是必要依赖，仍按22公共/24实际能力组合做一次末轮验证；机械替换须先查重复限定符和完整16调用者，不为此新分配P或重复全量发布。

## 运行时绘制管理器与八类分派批次

逐项读取attachment static/cool/box/lookup/spin/color/status/bit八类scope与end，native observer/frame/month/status/spin公共对象及main attachment前后调用。公共adapters/win32/paint_dispatch拥有完整GUI-thread manager：static/cool/box/bit观察链、lookup source、status/spin current与stack、native-call/theme/month关联。选择资格只读完整公共runtime/instance，native/default无新保护；scope分派继续复用既有lifecycle/dispatch native-unwind。原普通返回只pop观察链，main负责常规资源归还；exception按各scope原顺序归还text/status/spin/overlay及恰好一个既有root借用，不新增catch/retry/卸载。nested native forwarding按原规则进入一次static观察，outer分派不能双release。产品只组合observation/runtime和实际Win3x month/frame等绘制，不向公共库提供私有字段view。登记build/t26-package/s10-paint-manager（含package/modules），双宽定向纯C独立八类/嵌套/native-unwind/最后引用及所有实际消费者模型；每宽600秒，模型60秒，当前桌面不隔离，终态保存清理，不逐文件发布。checklist ownerdraw scope及主消息前后font/resource组合仍继续审计，不预称全部完成。

本组16公共/ABI双宽已通过。接入复核确认vcl_extra_frame仍私有持完整native BitBtn caller/DC/parent/rect资格及verified提交，属于本组BIT scope的同一protocol，合并进既有公共frame_paint，原profile仅Win3x face函数；删除两个私有C/H并补既有独立frame test的真实TBitBtn正例、renderer失败/退役提交与caller/part/DC负例。沿用s10-paint-manager登记root，所有实际模型待合并后统一跑，不叶文件发布。


本组合并完成：双宽16公共/ABI和32实际模型全部通过，无skip。独立manager测试覆盖八种正常/原生异常、嵌套转发、最后引用退役、真实HRGN与spin堆资源归还；BitBtn公共frame测试覆盖正确caller/DC/part、renderer失败和重入退役。GDI原生对照证明删除后GetObjectType在GdiFlush前可保留旧类型，故仅修正测试同步，生产清理/断言未放宽。错误机械成员替换、测试缩进及误判foreach生成target的预检失败均归档；后续以实际Ninja图确认目标。生产累计+3012/-2459净+553（含52新C/H），252去重通过调用不替代218个frozen和其他partial审计。来源1039回执、原资源及负例、双宽common/instance归档和治理13负例通过。

第二角色逐项对照保存的八个旧end源码：normal只pop、main保留原常规清理；native/default不增加异常恢复；nested仅STATIC/BIT拥有自己的最后release，其余转发不双release；scope按旧顺序恢复text/status/spin与overlay。BitBtn仅原生预检和成功提交公共化，原Win3x face函数继续作为完整操作。实际产品消费者使用同一manager，无第二套TLS观察链。八文件仅开发候选，CLI实际link且no-work复核后复制；正式包仍S9，S10/T26保持开放。效率决定：相邻BitBtn资格合批后一次实际模型验证，不叶文件全量交付；下一批继续checklist ownerdraw关联和main消息前后资源组合。

本组合并scratch已按原生CIM、精确绝对路径与上下级reparse检查清理9,766,101字节；八类旧清理函数及原始验证日志已归档，保留稳定Ninja/ccache。

## Checklist owner-draw 关联批次

按attachment_guard/state、report_notify和全部DrawFrameControl消费者复核：嵌套owner-draw关联持完整public instance/runtime/window/DC/item，是通用runtime观察；Win3x只提供checklist绘制资格和最大glyph逻辑边长。并入既有adapters/win32/checklist，manager归既有paint manager，删除私有scope及target实现。begin保持原WM_DRAWITEM/parent/identity/item/DC顺序，end只正常恢复，不虚构原有notify路径不具备的异常清理。独立真实窗口测试覆盖嵌套/barrier、cancel、错误消息/item/DC和动态parent。登记build/t26-package/s10-checklist-owner（含package/modules）作双宽增量公共契约与实际VB/VCL消费者；每宽600秒、模型60秒，当前桌面不隔离，终态提炼清理，相关修改合批验证不独立发布。

首轮错误将fixture输出目录当target，Ninja未编译即拒绝；证据已保留。实际core-vb-checklist依赖两个runtime target，无需重复列出。登记build/t26-package/s10-checklist-owner-native（含package/modules）继续同组验证，不能把启动失败计作通过。

Checklist本组双宽10公共/ABI与6实际模型全部通过，无skip。包含原VCL bare/styled与aware四种child、自有job退场，VB5/6 bare/styled bitmap/禁用/重入/换父/应用自绘断言。新增公共scope断言使用真实HWND/DC验证嵌套、cancel、无效item/DC、动态parent及end恢复。manager拥有scope，不再传产品TLS字段指针；旧profile scope/target删除，产品仅组合checklist资格和16逻辑像素策略。原notify只正常end，没有新增catch或宣称异常可清理。原资源/1039来源/双宽common+instance边界通过。CLI实际依赖link并no-work后复制八文件开发包；正式包仍S9，不叶文件发布。

继续顺序审读12个剩余profile/测试输入，frozen-to-review由218减至206，但text_policy模块caller仍S10待迁、scroll/other临时绘图和font/glyph资源有具体S13函数接收，scope创建测试有S11接收，不冒称这些partial已实施。效率复核：实际模型只选发生关联变化的VB/VCL，不重复32项完整组；新增case是扩展既有公共测试，不增加252去重统计。错误fixture目标再次暴露命名核对问题，后续命令直接使用已查明的测试target依赖，不手写推测fixture名。

本批两终态scratch按原生进程/精确路径与reparse检查清理5,984,281字节，原始失败、native输出、八文件摘要、工具链/cache和边界保留。全S10生产当前+3056/-2484净+572（52新C/H），不把共用契约新增行数称为代码减少。

## Runtime caller与既有公共协议复核批次

text_policy的两个native caller函数只识别report/VB/system模块，不涉及Win3x字体，迁既有runtime_text并直接消费公共report/direct scope，删除两个产品固定转调及profile声明。四条原断言移到独立runtime test，补USER32、invalid/direct/label/report精确资格；CP1252/ASCII/RTL与font/glyph资源继续有具体S13接收。源码复核保留custom_draw_after与post两种现有协议：前者由notify.c真实生产路径使用，后者由CoolBar使用；active/painted分别由scope与persistent runtime持有，不为整体对象新增重复状态。一次受截断搜索输出误导的死代码删除尝试在完整搜索发现notify.c消费者后立即撤销，未编译、未交付，不把错误判断记为审计结论。后续消费者搜索只输出唯一文件名并读取完整结果，不能依据截断命中判断无调用。登记build/t26-package/s10-runtime-callers（含package/modules），本批改动合并后双宽公共契约与实际CoolBar/VB文字/Checklist消费者验证；每宽600秒、模型60秒，不私有桌面、不逐文件全量发布。

Caller批首轮新增测试错用wmain符号，公共测试实际是main，已修正测试地址，原生产编译无错误。原失败完整保留；登记build/t26-package/s10-runtime-callers-native（含package/modules）继续相同组增量，不改变断言或生产行为。

本轮两项测试失败已保留：import函数名地址是EXE thunk，改用GetProcAddress读取真实USER32代码地址；vb-font误入CTest默认私有桌面入口被error5拒绝，属于执行违规尝试，不算合规通过，移出CTest匹配，仅以已读--private-worker提前返回路径执行当前桌面模型。生产不变。登记build/t26-package/s10-runtime-callers-final（含package/modules）继续相同组；所有现有夹具入口已逐项确认，不再按测试名推测可用入口。

Runtime callers本组双宽12公共/ABI、14实际模型全通过，无skip。原caller模块顺序不变，report/direct读取直接消费公共scope，无字体/画法进入runtime。VB5/6 files/checklist、VCL report、CoolBar重入/退场和font32槽/33source/OOM/精确DC实际消费者保留；两种custom_draw生产协议及断言完整恢复且通过。CLI依赖实际link后no-work再复制八文件开发包，不是正式发布。公共core与instance双宽归档边界、1039来源及资源门禁通过。新断言扩展旧test，252去重不递增。

效率复核：复核25项摘要完全等于S9最终已审源码并读取其原归属/当前consumer，复用有效证据而不重复整包构建；原frozen206减至181，不把部分图形/创建/主消息工作算完成。custom_draw active与persistent verified属于不同所有者，保留原两标量引用，避免新增聚合、getter和重复状态；检查输出截断/函数入口/CTest默认desktop错误已归档，后续先列完整unique调用文件并读实际入口，不能只看测试名。主消息前后资源与通知composition继续S10；创建S11，具体font/GDI事务S13。

本批三终态scratch按原生CIM/精确路径及上下级reparse检查清理7,366,330字节，保留稳定缓存；全S10当前生产+3074/-2505净+569（52新C/H）。原始失败和最终结果均已保存，不以复测通过掩盖错误入口尝试。

## 完整运行时消息事务批次

冻结当前attachment_proc的顺序：frame retire→overlay begin→root retain→风格guard/status-font失效→spin/status scopes→风格roles/terminal/text→report原生准备→witness reset→唯一dispatch→text/status/spin归还→box/toolbar/data/user/rich/grid/group/scroll/edit/report/tab/track/color/status/date settle→overlay finish→最后release及有条件LastError。公共adapters/win32/message持每调用完整资源对象与事务，产品只供两个完整guard/text操作和既有frame/control/edge/report/render操作，不以字段view或逐行callback搬家。native_unwind仍由唯一paint_dispatch处理，默认NATIVE和prepare/post原先无异常保护的范围不扩展。report原生准备归既有runtime_text，删profile prepare/reset及产品report固定转调。登记build/t26-package/s10-message-transaction（含package/modules），保留改前attachment/guard快照，所有相关更改和公共原生事务测试合批后双宽增量与实际role模型；每宽600秒、模型60秒，当前桌面，不私有桌面，不单文件发布。

完整message首轮双宽12公共/ABI通过；既有paint test实际走生产message入口验证guard/prepare与root borrow、status/spin prior和嵌套异常最后引用。下一步合并本S已核验的paint-manager/bindings/runtime-text模型，按参数去重，先对实际EXE/DLL输出目标做Ninja增量依赖检查再从同批八文件运行；s10-message-transaction同时登记captures子目录供原样本原生capture日志，环境WIN16STYLE_CAPTURE_DIR指向该处，不写工作区外。

Message本组双宽12公共/ABI及68条去重实际命令全通过，无skip。public native test真实调用生产message，验证guard/prepare/root借用、status/spin先前head、有条件原生error、普通/嵌套native-unwind与最后引用；actual涵盖原生spin/aware、modern spin、CoolBar、lookup/box/color/date/month/static/BitBtn/status/rich/group/panel、track/toolbar、三种combo、updown、lifetime/present/scroll以及VB5/6 files/checklist/report/font/global文本。组合之前按实际输出目标做Ninja依赖检查，CLI实际link/no-work后复制八文件开发包；无叶文件正式发布。原11个成功结果因stderr标记漏读被驱动误拒，按两路原始输出核实并从第12项继续，失败原记录保留，未改生产或重跑已完成项。

第二角色对照保存的两个改前完整源码与新message函数：retain位置及两个policy phase不变；每调用持完整overlay/status/spin/text对象，cleanup传给既有唯一dispatch；normal清理和post settle精确同序，report绑定和native查询从profile迁到runtime_text，产品只给真实notify/graphical-label资格。status/spin/color/date在入口捕获preserve条件，native返回后保存error、最后release后恢复，默认其他消息原错误副作用保留。prepare/post没有新unwind/catch，不能声称新异常能力。整条产品main路径实际调用新事务，不留旧副本；profile prepare/reset和两个固定report转调删除。

另25项S9新公共实现/测试与最终已远端复核提交逐字节相等，复用其所有者/交付证据并与当前实际消费者核对，frozen181减至156。不是把未修改文件当作本次新实现，也不计新运行；root主事务已完成，parent report_notify通用协议继续S10；创建S11、font/GDI呈现资源S13不缩小。来源1041、原五图形头/声明及双宽common+instance边界通过。

本批终态scratch经原生CIM及精确路径/reparse核对清理15,957,602字节，保留稳定缓存和S9正式包。全S10当前生产+3151/-2533净+618（54新C/H）。效率检查：主消息完整能力一次实现并合并68条现有命令，而非逐个scope/文件交付；复用已验证S9原源码与已完成模型，从stderr观测错误处继续，没有重复全量构建。下批父report通知关联协议继续按完整操作拆分，不将其作为主事务的附带空包装。

## 父 report 通知协议批次

完整读parent report_notify、file/graphical/checklist/bit实际调用与公共report/text/checklist manager：公共message承担每通知root借用、native owner-draw payload/DC/parent核验、两类临时scope、native转发及原结果/LastError、custom-draw report DC重新绑定和NCDESTROY解除。产品仅提供固定初装file/button/checklist支持标记、file/button绘制资格及完整字体准备操作；标记全仓只有initial写入，按值传入不改变重入行为。BIT直接复用唯一paint_dispatch，删除产品bit固定转调和profile通知服务表/实现。普通file/checklist路径原先只有normal-return end，没有新增异常catch/物理卸载。登记build/t26-package/s10-report-notify（含package/modules/captures）保存改前guard，合并纯C公共native通知测试及实际VB5/6 file/checklist/graphical、VCL checklist/BitBtn/report消费者验证；双宽每宽600秒、模型60秒，当前桌面，不私有桌面，终态提炼清理，不叶文件交付。

父report本组首轮双宽12公共/ABI、20实际命令全部通过，无skip。新纯C公共test真实subclass/native消息路径验证返回值/LastError、三阶段report DC清除/重绑、skip/post无重绑、file/button嵌套scope、无效CtlType/换父拒绝、同步退役/root最后释放及parent NCDESTROY元数据回收；不链接Win3x painter/不安装进程hook。实际模型保持VB5/6 files/checklist/graphical、VCL checklist/BitBtn/report及font32槽/OOM原断言。普通file/checklist scope仍只normal-return清理，BIT使用原唯一native-unwind分派，不冒称新异常或卸载支持。删除profile实现/服务表、产品bit固定转调；native协议不残留第二份。

第二角色逐项对照改前guard：bit资格和itemAction严格等于ODA_DRAWENTIRE；file/button class选择、drive combo目标DC检查、prepare→report-enable→bind顺序不变；file/checklist标记只有initial赋值，按值配置无重入差异；native前恢复entry、返回后捕获error，先scope退场再按stage/skip/cancel重绑，最后release后还原error。无额外payload探测、消息、状态改写或新回退。基础basic/combo/edit/list/scroll_message十源码/头逐函数复核公共状态与完整render/input操作，无私有反依赖；现有S9/S10真实组合证据复用，frozen156减至146。新公共架构case双宽累计254去重，重复模型不加。来源1042、原资源与双宽common/instance归档通过，CLI实际link/no-work后八文件仅开发包，正式仍S9。

效率检查：本组无编译/测试修正重跑，实际输出目标依赖检查在模型前完成；只选择受通知协议影响的20条，不重复前批68条；机械指针替换在编译前源复核修正。余下attached/remove/terminal混合协调及基础source/test清单继续S10，不以公共通知完成声称全S关闭。无改动的无用include留待相邻生命周期批次一并清理，避免为叶文件重链。

本批终态scratch已按原生CIM/精确路径及上下级reparse检查清理8,168,341字节；原始结果/改前源码/工具链/cache与八文件摘要已保存。当前全S10生产+3212/-2582净+630（54新C/H），不把公开契约新增等同代码减少。保留.ninja/.ccache与S9正式发布包。

## Runtime attachment 完整根与退场批次

全仓只有attachment.c、profile attachment_guard/state与control_link使用私有attachment布局。公共adapters/win32/attachment持完整instance/runtime/themed，profile首成员嵌入公共root，四个实际源码/测试消费者迁移，不增加字段getter/view。公共root按指定subclass查找，统一GUI线程、附着协议状态、normal detach/bindings回收及terminal first-error结算。cool/host/user-label支持、month mask和实际frame/control/theme操作由产品提供；controlbar/rich/native role验证直接公共实现。终态不重画，ordinary尝试恢复；普通detach失败保留所有权，terminal仍按原契约处理exact nodes，不称物理卸载。产品现代surface预处理保持S12边界，不改composed-class资格。登记build/t26-package/s10-runtime-attachment（含package/modules/captures）保存改前source，双宽公共root状态/正常/失败/跨线程/重入/terminal与实际lifetime/role模型合批验证，每宽600秒、模型60秒，当前桌面不隔离，终态提炼清理，整S才完整交付。

首轮头清理误删modern_spin_shape初始化仍使用的control_vcl_spin显式声明，编译已检出并恢复，无算法/测试变化；原失败保存。公共root和独立test已编译。登记build/t26-package/s10-runtime-attachment-native（含package/modules/captures），沿用有效对象继续增量。清理include不能仅搜索旧函数前缀，后续按实际完整符号消费者核对。

Runtime attachment本组双宽14公共/ABI及74去重实际命令全部通过，无skip。独立公共test用真实subclass发布公共root，覆盖16类role已装/未装状态、cool/user策略、GUI thread、普通detach失败所有权、移除后重入、terminal first-error与无root终态、最后release及native窗口回收；不安装进程hook、不调用Win3x renderer。实际组合复核完整消息/通知两组去重并集，现有VB/VCL/字体/scroll/present/reentry所有断言保留。CLI依赖实际link/no-work后复制八文件开发包，无叶文件正式发布；正式仍S9。

第二角色对照三个改前源码：profile first member现在是public root，root首成员instance，原subclass data/context与owner指针地址保持；公共lookup无需私有字段getter。attached短路次序不变，controlbar/rich原固定wrapper直接调用相同公共identity，cool/host/user-label与month拒绝mask保留调用方策略。ordinary依次detach→report/cool解绑→frame/control owned remove→themed/root theme→protocol dispose→release，失败detach不释放持有状态；terminal仍累计第一个错误并访问原exact节点，不重画或冒称卸载。清理后回调context保存至最后release，原references最后根释放仍在profile字体dispose之后，未擅自改变字体/关联引用顺序。旧profile生命周期服务表/实现及两个固定转调删除。

顺序补审control_state和scroll_track两公共C/H，确认完整字体/protocol/tab及输入actions所有权；attachment ABI头保留兼容声明、创建S11；controls.c完整读完并明确control_proc消息borrow/font/observe/guard/dispatch/finish组合仍需S10，而不是统称产品接线。dialog combo创建S11，present/scroll-caret图形边界S13有具体函数。frozen146减至140，其他partial不算已完成。新增公共架构case双宽累计256去重，重复模型不加。来源1045、原资源及双宽common/instance归档通过。

效率复核：源布局只影响四个实际typed消费者，先穷举再改，不扩大机械替换；一次首轮头清理漏modern_spin_shape声明已保存并修复，其余测试/模型无修正重跑。实际cache文件名为cache-before.txt/cache-after.txt，本轮按真实文件保留；早批cache_statistics为空时不能声称已留hit统计，只能引用其构建日志的ccache launcher证据，不补造历史统计。继续控制消息完整事务、owned predicate和余下逐项测试审计，不缩减S10收口条件。

本批两个终态scratch经原生CIM/精确路径和上下级reparse核对清理16,596,230字节；改前源码、原始失败/成功、工具链、真实cache-before/after与八文件摘要已保存。当前全S10生产+3347/-2681净+666（56新C/H）；稳定缓存和S9正式包保留。

## 基础 control 消息完整事务批次

冻结controls/control_state/dispatch与全部font消费者：公共application/control/message持每调用message/parameter/value及previous/outbound字体租约，统一root retain、VCL spin before、原生tab/header/progress observe、combo native-paint depth、pinned font retain、guard→风格font准备→绘制分派→既有message_finish。风格提供两个完整font/dispatch操作，不按字段getter拆分；当前fonts.retain就是原win16_style_system_retain，改为公共provider直接使用。普通完成按原顺序，既有异常边界不扩大。三个font消费者及header一起接入完整message对象。remove_owned改用既有style_instance_lookup及owner资格比较，避免重复读subclass/private cast，不新增空包装API。登记build/t26-package/s10-control-message（含package/modules/captures）保存改前controls/control_state，双宽纯C独立生命周期/字体/失败/嵌套/重入及既有消费者合批验证；每宽600秒、模型60秒，当前桌面不隔离，终态提炼清理，整S才正式交付。

Control message本组首轮双宽18公共/ABI及74实际命令全部通过，无skip，无编译/测试修正重跑。新增公共test真实native subclass验证正常结果/错误、guard短路、font准备失败后租约归还、嵌套消息、native参数改写、dispatch重入detach/释放最后owner与字体最终清理，不链接Win3x renderer/不装进程hook。原control_link字体模式/取消断言改用同一完整message对象，条件原样保留；实际VB/VCL/按钮/组合/scroll/header/status/present所有旧断言通过。新公共双宽case累计258去重，原74命令不重复累加。

第二角色对照两个改前完整source：入口先读取pinned再retain，spin-before在native分类之前；原生observe及combo-paint depth在font pin/guard之前；fonts.retain使用现有provider，其当前配置与原直接system_retain相同。guard→font→draw分支短路顺序保持；message/parameter/value的native重写传到既有finish，previous/outbound/pinned和后继font、最后instance引用都由原公共finish按序归还。没有新增native-unwind/catch；provider异常仍沿用原边界。产品只装配实际风格操作，prepare_font实际函数接受完整public对象，没有新增字段getter转调；remove_owned直接消费现有instance_lookup并保留owner资格，没有为一次比较造新API。

顺序读完9项integration测试辅助文件并记录job/handle/DC/map/hook所有权、native baseline和legacy worker边界，frozen140减至131；原版可选actor样本未运行，不计新通过，vb_intrinsic完整源码仍待后续审读。独立native观察器用于测试oracle，不迁入生产style库；不把读源码当运行或外部依赖。原生队列见证仍被当前实际模型使用，单独test若未运行明确未计通过。

来源1048、原五资源头/声明与双宽common/instance边界通过；CLI实际link/no-work后复制八文件开发包，正式仍S9。效率复核：完整控制消息与四个接口消费者先合批再构建；原74 recipes直接复用并先按实际输出目标检查Ninja依赖，stderr标记一起读取，无重跑已完成案例。剩余初装root引用/native identity采集与role完整性混合部分仍S10，创建API/first-show S11，图形/font S13，不能把全部attachment初始化直接当风格策略结案。

本批scratch按原生CIM/精确路径和上下级reparse检查清理16,557,353字节，保留原始source/test/toolchain/cache和八文件摘要及稳定缓存。当前全S10生产+3410/-2708净+702（58新C/H）。聚焦两宽实际build约11.2/15.1秒，未做clean全量或正式重复交付。

## 初装 native metadata 与关联引用批次

完整读取private initialize及六个candidate/seed实际源码消费者。runtime候选/extra是纯原生family选择数据，进入已有application/control/runtime；appearance继续决定哪些family受支持。公共attachment initialize持root借用、parent/module/native role派生、MDI/popup引用和report来源，按原语句顺序执行；中间只调用一次完整owner角色选择操作，保留原file/picture/button/checklist查询与私有标记初始化位置，不用getter/field view。不改创建API链或supported范围，旧private seed/initialize删除，产品直接组合public seed。登记build/t26-package/s10-runtime-seed（含package/modules/captures），保存改前guard/product，双宽public元数据/引用/失败回收及既有原生模型验证；每宽600秒、模型60秒，当前桌面不隔离，终态提炼清理，整S才正式交付。

初装public seed首轮双宽16公共/ABI通过。实际模型复用上一批74命令，再加入MDI普通/强制publication失败两宽4命令，覆盖借入引用失败平衡。mdi-ref需要目标模块内attachment_proc RVA，必须从当前DLL的nm和PE ImageBase计算，不能沿用历史硬编码地址；驱动将符号/VA/base/RVA与测试包摘要一起记录。所有入口已读，显式worker/current desktop，case60秒，先依赖输出检查和CLI no-work后八文件开发包。

本组已终态：16公共/ABI与76实际命令通过；x64 VB50/60 files在第一个未装样式的原生child安装测试witness时返回126，VB50同条件一次继续亦失败，没有通过或修复。全部原始结果保留；continue驱动的无条件PASS打印不能替代exit_code及markers，两项单列未解决。四个MDI普通/强制publication失败均通过，符号RVA来自当前DLL且摘要与开发包一致。来源1048、原资源及双宽common/instance归档通过；全S10生产+3532/-2751净+781，58新C/H，累计258去重不增加。终态scratch已核对原生CIM、精确路径及reparse后清理17,284,079字节，正式包仍S9。

## 安装后的主题及滚动条结算批次

登记build/t26-package/s10-install-settlement（含package/modules/captures、改前guard/product）用于本批双宽增量、独立公共原生事务及现有实际消费者验证。完整native install选择仍由呈现方提供，公共attachment负责主题目标选择、滚动条setup、取消/错误和失败remove；复用已有root，不另建事务状态机或逐字段getter，不改变native异常边界。每宽600秒、模型60秒，当前桌面不隔离；合批验证、归档后清理，整S才完整交付。未解决witness126另行调查，不扩大权限/等待或改系统设置。

本组初次启动因外层将stdout重定向到Worker自己拥有的run.log而失败，尚未编译；改用Worker原日志后增量继续。首轮x86编译检出独立test直接把WINAPI GetComboBoxInfo传给cdecl provider接口，已用测试内原生适配修正，无强制cast或生产ABI变化。原失败均保留。修正后双宽16公共/ABI通过（实际build约4.8/9.2秒），14个已有产品消费者在途；不重跑全部78命令。顺序完整读9项既有public control测试，核对font/DC/window/借用所有权和真实公共消费者，frozen131减至122；静态复核不记新运行。

本组14项真实产品回归全部通过：两宽combo三种布局、lifetime、control/scroll present及VB字体；公共24种结算断言不另算新进程用例，累计258去重不增加。第二角色对照改前源码确认：安装选择顺序及首次LastError采集、theme target写入/启用后scrollbar、成功theme_remove条件、取消覆盖ERROR_CANCELLED及失败时仅live remove均保持；root context与原attachment同地址，不新增异常catch或物理卸载。来源1048、原资源、两宽common/instance边界、治理13负例及diff通过。原生CIM和精确路径/reparse核对后清理本批8,603,612字节。全S10生产+3574/-2778净+796，58新C/H；正式仍S9。

效率检查：本轮完整初装/安装后结算分两能力组增量推进，复用既有模型并只对受影响的14条再次验证；没有叶文件全量交付。两次witness126未提供新证据后停止同条件重复，明确发生于原生未装样式child，保存到后续调查，不以其阻挡其他独立能力或记通过。新增公共单元首轮cdecl诊断和启动日志冲突属于执行错误，均已修正并保留，后续Worker日志由Worker唯一拥有。下一批按有限清单继续剩余测试及mixed role完整性，禁止再按字段拆出getter/重复管理器；S10退出前才完整交付。

## 既有原生测试归属复核批次

本批完整读17项既有公共/产品测试与helper，逐项记录于冻结台账：全局controller、FNT builder、font pair、theme tracker、两menu协议，以及checklist/capture/extension/protocol/visibility/ControlBar/owned-job/pending owner。它们是独立原生oracle和资源/失败断言，不是应下沉的产品机制；没有为减文件数把测试实现混入公共生产库。frozen122减至105，运行通过数不增加。creation相关测试明确S11接收，legacy isolated_process默认入口仍禁跑；显式worker/current desktop规则不变。全文件新建图形资源仅属自有测试辨识色块，原产品五资源不变。

审查构建本次diff及相关实际target注册：公共control只链接common图形/instance/unwind与系统API，测试foreach按实际源码名生成，toolbar bounds已指向新公共reader和显式worker；剩余CMake整体差异/映射继续核对，不把截断输出当全文件完成。当前无在途构建/测试，无新scratch，不为未改动测试重复构建发布。

随后完整复核7项global VB text/header owner/status font/progress-v5 witness/updown owner测试，frozen105减至98；源码包含故障注入是测试oracle，不是第二生产实现，process-lifetime观察钩子不称物理卸载。纯静态复核不增加通过数字，不引起二进制变化或额外构建。再次完整读special_paint与vcl_identity四C/H：已迁出的spin/paint scope不再存在，纠正过时partial标签；剩余graphical_target/frame_prepare/frame的公共bindings/native DC预检与VB data原生glyph采样明确交S13 control-root/renderer契约，保留具体函数/调用方。VCL分类保留实际风格支持选择，CoolBar固定class-owner转调需在下一整组代码中删除；attachment角色完整性仍S10，未以状态更名冒称完成。

最后完整读728行可选原版VB observer及其已审helper关系，记录来源明确的sample-action分支、PID/TID/job边界、witness/mapping/event清理、状态恢复和禁止默认desktop入口；frozen98减至97。原版外部样本本轮没有运行，不计通过，也不成为构建/验收依赖。没有为其重编译或创建新scratch。此次24项短文件加1项完整observer与四项production复核均有逐项理由；治理及13负例、diff检查通过。活动仍S10/T26，正式包仍S9，下一组按实际未审文件与剩余role完整性实施，不重做已迁public paint scope。

## 完整角色有效性检查批次

登记build/t26-package/s10-runtime-current（含package/modules/captures及改前guard/attachment/controls）为本批双宽增量和独立原生/实际消费者验证。完整current操作归公共attachment，按原rich→combo→group→grid→VB-data→VB-user→toolbar→tab顺序，逐步重新lookup并短路；VB两个完整支持策略及month拒绝位由调用方提供。删除六组私有valid实现/产品转调和controls聚合接线，产品只给公共操作与实际策略。不是把每个字段拆出getter，也不改变candidate尚未witness时的有效性语义。CoolBar固定class-owner转调同时删除，直接消费已有公共identity。每宽600秒、case60秒，当前桌面显式worker，不使用computer-use或私有桌面；终态归档清理，整S才完整交付。

完整current本组首轮双宽16公共/ABI及22实际命令全部通过，无skip/编译修正或native重跑。独立公共test验证六个witnessed-role拒绝、未witness候选仍current、native tab类/parent/RTL、data/user阶段短路、策略回调替换/移除root后重新lookup及无root目标；不链接Win3x painter。实际rich/box/combo三种/group/grid/toolbar/tab/lifetime/present消费者均使用唯一公共操作。public fully-attached与current的语义不同，未混淆。

第二角色对照三份改前source：原8次lookup仍在同阶段发生，data/user各自内部仍lookup；rich私有identity转调无额外API，直接调用同一public identity，month rejection仍由产品给MCS_MULTISELECT。查询短路和native LastError不增加归一化/恢复。六组profile函数/声明及产品转调、controls聚合函数/多余protos删除；CoolBar固定wrapper删掉后调用既有class-procedure helper，owner短路顺序保持。源码/资源/双宽common+instance边界通过，累计258去重调用不增加。改动净比前批少20生产行；全S10+3612/-2836净+776，58新C/H。删除函数后diff发现仅EOF多空行，native完成后仅清理该空行，保留测试时source摘要；最终S级同快照构建/包测仍必须执行。

相邻完整读5项grid/group/tab model及fixture，实际两宽本组亦通过，frozen97减至92；fixture是独立原生协议oracle，不迁为生产代码。终态scratch经原生CIM、精确路径和reparse核对清理9,911,950字节，保留原始结果/source/cache和稳定缓存。效率检查：按完整8阶段检查合批，避免六个新getter/新管理器，删中间转调后只选22条相关实际命令，不重复此前78条；剩余policy guard/初装装配及未审清单继续S10，正式仍S9，不冒称已整S交付。

## 晚期原生 control 安装结算批次

登记build/t26-package/s10-runtime-controls（含package/modules/captures与改前attachment）为本批双宽增量、独立原生生命周期与实际消费者验证。公共attachment持有晚期EDIT安装后的theme/scrollbar设置及移除结算；产品只选择具体render install/remove，借用整个公共root和完整操作配置，不增加平行状态机。保留初装与晚期不同的失败/清理契约，render remove失败仍按原行为撤theme并清除借用目标；取消、原native错误及异常边界保持。每宽600秒、case60秒，当前桌面显式worker，终态归档清理；不为叶文件完整交付。

本组最终双宽16公共/ABI及22实际命令全部通过，无skip。新增12个原生组合覆盖render/theme/scrollbar失败，render中取消、theme中取消仍按旧顺序继续scrollbar及outer runtime结算，link/combo/spinedit/singleline/other-role不启用EDIT主题，以及render remove失败仍清theme目标。首轮test误复用要求NULL owner的初装untheme oracle，bad=5已保留；为晚期契约增加验证实际owner/target的独立回调，未修改生产逻辑或弱化原断言，修后通过。

第二角色对照改前attachment确认：调用方原先retain由完整message持有，公共callback不新增/消费引用；render实际owner始终是root.instance.context，controls配置只在同步message栈上借用，public runtime不保存它。实际render选择/参数和取消指针保持，EDIT成功后才按原条件写themed→theme_enable→scrollbars；remove先保存render返回值，再按原条件theme_remove(w,owner)并清目标，不改成新的失败恢复或卸载。source1048、五原资源/声明、两宽common+instance及diff通过。原22actual覆盖toolbar/track/group/combo三类/lifetime/present及memo/link/spinedit；累计258去重不增加。

本批完整补审memo/spinedit/link模型和observer/frame两个helper，再完整读267行font_layout，frozen92减至86。长字体全矩阵/可选stress本批未执行，不把静态审查记为通过或宣称全过程无泄漏。终态scratch经原生CIM、精确路径和上下级reparse核对清理9,748,744字节，保留原始失败/成功/source/cache与稳定缓存。全S10生产+3637/-2837净+800，58新C/H；本批公共完整契约增加24净行，产品中的状态结算和remove回调已删，不冒称全仓代码减少。效率复核：实际剩余native资源所有权优先整组迁移，相关22条复用历史recipes并检查实际输出依赖，不做叶文件完整交付；下一批有限清单和剩余policy/创建边界继续，不重复已迁current与theme结算。正式仍S9，S10/T26保持开放。

## 原生协议与资源测试连续复核

完整复核26项Header/Tab/Progress/Spin、VB checklist/runtime/Data/MDI、VCL combo三种及fixture、status四类、group/panel和像素helper、Win2k主题/原生target与产品tray controller测试，逐项资源/调用/边界见台账；frozen86减至60。测试是独立native oracle，保留原始图形和原样输入/数据/回退断言，不迁进生产库。不把源码审查记新运行：Header v5旧间歇差异仍未解决；Tab剩余root明确由worker进程退出回收；font-alias无handle重用时返回2，不得把ALIAS_SKIP打印当通过或假装77环境skip；tray stub只验证控制器状态，不证明Explorer/UAC。无源码/二进制改变，无重复构建/新scratch。

本轮同时找到具体未收口生产机制：public seed之后，product仍直接组合publication失败时的protocol/related引用/raw owner释放。下一整组应审查并接入完整public publish/discard契约，保留原错误与失败释放顺序，不能直接换成ordinary dispose（font/free/reference顺序不同），也不能仅加空包装。剩余policy/创建API仍按当前S10/S11边界逐项复核，未据此关闭S10。

## 未发布附着事务公共化批次

登记build/t26-package/s10-runtime-publish（改前源码、Worker日志/结果及公共契约输出）为本批精确scratch。将初装发布及失败protocol/关联引用/raw owner清理合为公共attachment操作，产品直接传free，不复制普通dispose；保留协议→引用→owner次序和原错误。不改挂钩/绘制/目标范围；沿用稳定双宽增量及现有公共契约测试，不做单文件正式交付。终态提炼后清理，整S10交付条件不变。首轮编辑脚本以系统默认编码读取文档失败，尚未修改任何输入；改为显式UTF-8。

本批公共/ABI双宽16项通过（增量编译约8/10秒）。同一已登记scratch追加package/modules和自有当前桌面MDI/强制publication失败4项实际路径；fixture显式--private-worker提前返回，不创建桌面。当前DLL符号实时解析RVA并核对package摘要，不复用旧offset；每项60秒，终态提炼清理。

本批双宽16公共/ABI、4实际MDI（含强制SetWindowSubclass失败后的host引用平衡）全部通过，无skip。第二角色对照改前确认：失败仍先保存错误，释放protocol→关联引用→raw owner，最后恢复错误；不调用普通字体dispose，不新增异常恢复或DLL卸载。公共native测试实际覆盖成功发布保留初始call、重复发布拒绝、discard前两关联引用归还及discard改LastError仍保留原错。首轮native驱动错误重置JSON导致KeyError，首次未持久化结果不计通过；保留脚本/失败后修正并完成4项。来源1048、原资源/声明、双宽common+instance门禁通过；累计258去重调用不增加。

架构权威已精简为三层责任及公共对象表，迁移明细留本记录和有限台账；修正CURRENT历史lib-style目录陈述和本brief过时首批状态。治理13负例通过。效率复核：本批只做稳定缓存增量（x86约8秒、x64约10秒）与16+4相关测试，没有单文件完整发布；源码静态26项复核不算新运行。60个frozen输入及其他partial仍需完成；已找到并迁移的publication尾巴不再留在product。正式八文件仍S9，S10/T26保持开放。

原生验证后仅澄清public header注释：initialize留下的caller ownership在publish失败时由publish消费；没有声明直接style_instance_attach也会清理。运行时源码未改，测试时摘要保留；最终S级同快照交付仍待完成。

本批终态已原生CIM复查无存活自有fixture，确认精确workspace路径及全部上下级无reparse，归档后清理5,869,917字节；稳定Ninja/ccache与正式S9包保留。最终source1048及治理13负例、diff检查通过。

## Owner-draw 父关系 guard 收束批次

登记build/t26-package/s10-owner-guard（改前源码、双宽Worker日志/结果、原生package/modules/recipes）为精确scratch。现有公共runtime_roles_guard接收调用方是否存在owner通知关系的策略，拥有report.parent原生关系失效检查；该检查在profile资格检查之后、既有inplace/popup/MDI/nav之前，销毁消息仍跳过，scroll原有terminal检查不改。不新增管理器或状态字段。测试覆盖parent不变/变更、非owner、无parent、销毁、已退役及移除失败仍继续后续role；结合已有产品MDI/checklist/lifetime模型。CMake长源清单仅展开逐行，保持原编译顺序；整组增量验证，不单独交付。

本批native实际8通过、x86 VB5/6 checklist查询重入两项失败，clip为空且回调未发生；同夹具S9对照通过，不能归因为环境或视作相同基线。下一步仅在已登记scratch内编译旧profile父关系位置的A/B变体（公共ABI仍相同，guard参数FALSE），用现有Ninja真实编译/链接命令及复制archive，不修改worktree或稳定树产物，以区分本次迁移与更早S10变化；两项有界测试，所有结果保留。

本轮完整复核24项构建/映射及OCX、status、superclass、VB资源模型、VCL原生模块/创建helper和MDI测试，frozen60减至36。CMake复核范围为S10完整diff/受影响生产与测试链接闭包，其余S9未变定义沿用，不虚构全平台运行；source列表逐行展开保留原顺序。可选OCX的x86/授权组件前提、enum callback限制、144DPI的77跳过、worker退出所有权与模型非真实VB/Delphi边界逐项记录，不把静态阅读记运行通过。

本批双宽16公共/ABI通过，原生新增断言覆盖owner parent、终态排除及失败移除继续后续role，既有MDI/native scroll的生命周期不变。双宽增量约17/26秒，构建列表展开保留原顺序；现有root/guard直接接收完整通知策略，无新管理器或字段view。两处source receipt预期集合错误（CMake不属于local import receipt）已纠正并保留记录，Worker未重启；来源1048/原资源/双宽归档/治理门禁通过。

初始实际10项中8通过、x86 VB5/6 checklist2失败；S9同夹具对照通过。scratch内旧profile位置变体2项及当前实现再次2项均通过，原失败明确保留为未解释的native clip/绘制回调间歇问题，不当作修复或skip。变体只复制archive、使用当前真实编译/链接命令，未改worktree或稳定树产物；初次直接link误带CMake末尾cd语句导致链接失败，已保留并去除非编译器参数后完成，不改生产逻辑。无新证据不继续相同重试。

第二角色核对删除路径与真实public入口后，修正10个已删除私有文件的过时pending/presentation-retained状态；owner/public入口/实际consumer和原验证保留，不把仍存活混合实现一起标完。全S10生产+3652/-2839净+813，58个新C/H；累计258去重调用不增加。效率检查：本批完成24项审计、收束一个既有guard机制及10个删除状态，按16公共/10实际和有限A/B取证，不做完整发布；剩余36 frozen及其他partial继续S10，正式仍S9。

本批终态已原生CIM核验无存活自有目标，验证精确目录与所有上下级无reparse，归档后清理12,679,859字节（含A/B包和临时archive/object）；稳定Ninja/ccache及S9正式发布均保留。

## 控件协议与资源测试收束复核

本轮按剩余原生/VB文件顺序完整阅读13项trackbar/tree/updown、container/control/Data/file/font/graphical/UserControl测试及运行时模型，frozen36减至23。逐项原生协议、回退、所有权及限制见有限台账；原生输入消息与实际物理输入分开，总资源诊断计数不冒充无泄漏断言，144-DPI跳过不算通过，VB/DAO真实运行库不由synthetic model代替。native witness126及checklist空clip旧失败仍保留，无新增证据不重跑。

没有生产或测试源码变动，不新增构建/scratch/通过调用，不为静态阅读重复原16/10回归。公共mechanism测试及产品组合保持独立，所需资源由外层owned worker按既有契约回收。下一批剩余VCL及基础控件测试，然后按当前源码重新结算partial生产条目；不是将审计数量清零即收口，整S同快照发布测试及远端复核仍待完成。

后续完整复核18项VCL原生模块及fixture、popup/push/check/report/scroll/toolbar、profile lifetime/font测试，frozen23减至5。原生异常、exact HRGN释放负例与后续绘制、glyph保留、借用DC/位图换出、身份/parent/ownerdata重入、64/65边界与普通失败进程回收分别记录；不把打印的PASS特征扩张成所有图形/运行库都支持。无生产/测试源码变动，未创建新scratch或重复native运行。剩余5项是static、VCL trackbar、control present、list tabs和scroll geometry；完成后仍需按实际source结算partial，不用frozen清零替代整S收口。

最后完整复核5项STATIC/VCL trackbar/control present/list tabs/scroll geometry，冻结309项输入待首次复核已为0。明确STATIC live与PRINT的不同协议、长GDI模式条件、宏跟踪只覆盖实际include的生产源而非全部common归档、set-time tab已知态和手算scroll向量与真实绘制边界。仍有40项过渡状态待终整，逐项列表已写入台账review_checkpoint；必须确认真实公共入口/调用/删除或明确后续S接收者，不能把首次审完当作全部机制已迁移。整S10交付构建、两处发布、实际包测试和远端复核尚未完成。

职责终整先结算9项：native-combo公共C/H及既有未变测试的过渡标签归已审；choice-paint C/H确认只剩原glyph；control-guard C/H只供Win3x支持/renderer选择。control-scope C/H的scope_created/ancestor排除/计数及provider装配明确由S11 application/lifecycle+dialog接收，具体product control_scope.c调用已定位，不能把它们称纯年代逻辑。已对照当前源码与实际调用，40条过渡记录降为31，原先finding保留为历史。

效率检查：本轮36项测试首次复核全部完成并按当前代码结算9项；没有生产/测试源码或二进制变化，不重复构建、不创建scratch、不增加通过计数。仍有31项过渡归属和整S同快照交付待完成；下一批从attachment/control状态与安装、native presentation、renderer/font明确边界和产品接线按文件顺序结算，再准备S10整批交付，不重新准入或创建新T。

继续完整复核8项control_state/native_presentation/month_text/list_paint C/H并确认当前hash未变。month_text仅西文字集支持策略；其余公共状态、原生项目快照、绑定/退场已真实消费，剩余dialog字段计数明确S11，列表border DC和renderer/font操作合同明确S13。特别记录native_presentation临时字体retain/release当前为normal-return代码，S13须单独核验其异常资源合同，不能用BitBtn/spin等别的异常测试宣称这一分支通过。31条过渡归属降至23，未改生产代码或新增异常补丁。

## 列表主题租约安装生命周期收束

最终复核special_install发现真实S10尾巴：bindings已拥有list_theme_owned及移除，却由profile grid分支单独capture/enable/cancel。登记build/t26-package/s10-list-theme（改前源码、Worker日志/结果、package/modules与实际grid/directory模型）为精确scratch。将该现有事务合入style_control_bindings_setup可选完整主题操作，复用现有root/setup_finish，不新建安装器/组件；保留先capture、发布owned再enable，最后cancel优先的原顺序，包括callback中退役仍完成原阶段。普通setup默认选项不变。新增原生LISTBOX/native非list与失败/已捕获/cancel/退役组合断言，开发只增量及关联模型，整S才完整交付。

## 列表主题事务结算与S10整组审计终整

最后复核发现special_grid_install仍拥有theme retained/capture/enable/cancel，而释放已在公共bindings。已将完整事务并入现有style_control_bindings_setup的可选list_theme操作，不新建平行安装器。获取成功后先发布租约，再调用enable；capture/enable失败及回调退役仍保持原阶段，最终cancel优先，普通setup不启用该选项。

双宽16公共/ABI和6实际lifetime/grid/directory调用全部通过，无skip。12组原生列表/非列表断言覆盖retained、capture/enable失败、回调退役、预先cancel和失败后cancel及恰好释放。实际directory是本S新增的两个架构调用，其他为重测，累计260去重通过。普通网格客户区无修改，目录两态edge=2816/content=0。双宽common/instance归档边界、1048来源回执和五原始资源门禁通过；结果和工具链/原始日志/测试前源码保留开发证据第98组。

原生CIM确认无自有测试进程，逐层绝对路径和reparse检查后删除s10-list-theme共6,040,004字节；第一次查询误匹配只读检查自己的命令文本，排除自身PID并限定python映像后重新确认，没有终止任何进程。保留稳定Ninja/ccache及S9正式发布。生产相对HEAD +3670/-2844净+826，58新增C/H；不是按文件重复全量构建。

309项冻结输入首次review和23条过渡处置均已结算；每条保留历史初步判断，并写实际公共入口、消费者和精确后续函数。S11接收创建/旧对话框pending事务；S12接收composed-frame/surface；S13接收字体/字形缓存、DC/原生绘制preflight和最终renderer注册。尚存的style_present detached-pointer view也明确交S13，不称全部公共接口终整完成。当前源码归属审计完成不代替整S交付；下一步合并同一快照完成一次干净双宽八文件交付。

登记build/t26-package/s10-delivery为本S唯一整组干净源码快照、双宽八文件候选和Build-Core原始日志；每宽600秒，进程由本任务持有，后台监控，不重启有效工作。最终发布另登记精确备份目录后先发布再做实际同摘要包回归；完成即提炼并删除快照/测试副本，保留.ninja/.ccache。

登记build/t26-package/s10-publish为八文件两处发布备份，s10-package-tests为整S聚焦测试驱动/逐项原始输出/捕获。测试使用干净快照的实际C夹具，以开发已审当前桌面worker配方及S9实际包配方去重组合；先只读形成有限计划核验可执行文件/参数，再按本次发布摘要执行。保留已知失败及真实skip，不无证据重复排障；三处scratch用完立即提炼并清理。

发布后整组回归发现x64 core-uwp-frame在第二次移除后的USER资源计数19/18断言失败，GDI13/13；x86同项通过，原始结果留package-tests。登记build/t26-package/s10-modern-compare为该单项的HEAD/S9源码有限原生对照：只构建x64 core-uwp-frame及依赖，不再全包构建/发布；原生对照等整组结束再跑，避免相互影响。保留初次失败，不以随后通过称修复；用途结束提炼并清理。

## 整组候选实际发布与未通过结果

唯一整S干净构建完成：x86约248秒、x64约307秒，ccache直接命中818/906、各miss68。1142源码/测试/工具输入与快照零差异，双宽各4进程/21样式归档边界及原资源/ABI通过。同批八文件已先发布assets根及O:/winnt，随后416项去重当前桌面实际调用410通过、6失败、0 skip，完整结果见整组交付证据。未全过，不关闭S10、不声称已提交推送。

失败为x86 header特殊格式24像素差异、x64自建现代外框USER19/18，以及两宽VB5/VB6文件夹具在原生未启用样式阶段SetWindowsHookEx返回126。有限对照只做一次：S9源码目标化构建x64 modern-frame及候选均通过，S9 DLL/header也通过；没有生产修复，不能以对照通过覆盖初次失败。旧DWM恢复本次未重复计通过，旧month等间歇观察继续保留。

临时驱动首次把历史timeout=False解析成0秒，在第249项立即中断；原始结果保留，原生CIM确认无遗留子进程后修正驱动预算并从249继续，之前248项不重跑。未修改产品或测试断言。测试计划首次把VB夹具子目录摊平，路径预检拒绝后保留完整架构目录后缀；原生测试开始前已纠正。

效率复核：本轮终整23条归属并完成一次完整八文件交付构建/发布，开发末批只做增量；结果未全过，下一轮仅定位已观测差异/原生witness环境，不再对所有叶文件或通过项重复全交付。终态四处scratch均先归档，再核验进程/绝对路径/reparse后清理，保留稳定Ninja/ccache及两处已发布候选；具体回收字节以整组交付证据cleanup为准。

本轮四处终态scratch已全部清理381,105,787字节；最后原生CIM无自有进程，精确绝对路径及全部祖先/后代reparse检查通过，发布后八文件两处摘要再次一致。保留稳定.ninja/.ccache和已发布候选；没有仍在运行的本轮构建/测试。S10仍active，6项未通过不记为已修复。

登记build/t26-package/s10-failure-contract为六项未通过结果的一组定向诊断/夹具修正，先核对当前原生同线程witness模块/回调/单独错误及消息队列；header检查完整原生图像契约，不改产品资格或放宽像素断言。只构建双宽必要夹具和纯C自有线程探针，用已发布八文件验证；不全量重建/发布。日志/源码前后摘要与结果提炼后清理，保留缓存；当前无运行中的前轮构建/测试句柄。

## 定向夹具契约修正与交付复核

四项VB文件失败已定位到测试契约：双宽纯C探针证明模块/导出地址/MEM_IMAGE身份有效，带模块句柄的当前线程钩子均126；同一回调/线程hMod=NULL均成功且可解除，创建消息队列不改变前者结果。微软SetWindowsHookExW的同进程线程契约亦要求NULL。只修vb_files和vb_containers自身线程的两处调用；既有vb_queue_test/vb_graphical已如此，真实跨进程vb_intrinsic保持原模块句柄，产品挂钩不变。16项双宽文件/容器和header原生模型全部通过，实际未样式/已样式两种child及资源退役断言保留。

Header捕获改为每次保存/恢复借用DC，消除比较输入被前次native PRINT影响的可能；原无图像列表场景仍在，并明确图像索引，另增加同一真实图像列表。没有移除像素相等断言或原owner/sort格式，最终双宽v5/v6/default/aware八项通过，四种格式均保留。初次24像素差异仍存档，不声称发现并修复产品绘制错误。

上述仅3个C夹具及元数据变化，无产品源码/八文件变化，因此沿用已完成整组构建/先发布包，不再重复全量交付构建。原416项410过/6失败保持；后续24项及此前有限对照给出6个原失败用例身份的当前通过证据，不写成重新执行416全过。现代外框USER19/18首次波动未解释，S9/current配对18/18一致，继续由S12/S14复核，不称已修复或绝对无泄漏。

最终改动清单反查发现309初始冻结表漏列8个已修改共享依赖/产品消费者，已单独scope_additions补录完整HEAD原始SHA、当前SHA、公共入口、真实消费者和测试；不改309原始冻结记录，不冒称事先已冻结这8项。共317项既有输入及86新增输出完成归属；所有148个生产改动路径均由台账覆盖。六个公共生命周期文件的preflight/owned借用/setup_finish/theme分类及两个status/global-text适配已复核实际完整调用，S13 renderer/font接收边界不变。

S10实施范围具备当前聚焦验证证据，开始P1提交及远端复核；未复核前保持active，不提前S11。测试探针两次编译错误（缩进警告、-municode重复UNICODE）均在scratch修正，保留原失败；未修改警告或安全门禁。

暂存差异门禁检出新增message.c末尾多一空行，已只删除该空行；用相同双宽编译器和对象目标重新编译，前后COFF对象SHA逐字节相同，因此不重链接/重发布产品。源码格式SHA变化及原始文本/两宽对象摘要已保留，原资源不变。最终生产+3669/-2844、净+825行，58新增C/H。

## P1实际远端复核与S10收口

P1 `effe2abd6dca90e904bcf0b8f31d333da61ab19f` 已推送并实际fetch；逐项读取远端192现存blob、核验28删除路径及八文件，与工作树和O:/winnt摘要一致，见[t26-s10-remote.json](../etc/evidence/t26-s10-remote.json)。提交实际218文件，--no-renames核验220路径；生产+3669/-2844净+825，58新增C/H。源码1048回执/124决策/294include、23+9负例、原资源5头与说明、公共源及双宽4/21归档、治理13负例均通过。

单人复核角色已读取当前diff、原请求、317项既有输入（309冻结+8显式补录）、86新增输出、实际公共调用和删除路径、初轮失败/定向修正/独立DC与图像断言、包摘要和实际远端内容。按本S扩展控件/运行时抽离边界收口，自动准入S11创建与对话框；T26保持开放。原现代USER波动、原header观察和既有DWM等继续明确归S12/S14，不能把迁移收口称这些问题已修复或所有框架/真实应用全面支持。

定向scratch最后7,056,009字节已原生无占用及绝对路径/reparse核验后清理；本S无剩余在用scratch/进程，保留稳定缓存与同批已发布八文件。不为下一S创建空代码或提前宣称实现。
