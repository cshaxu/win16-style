# M0 T26 S9 — 基础控件机制与呈现分离

## 准入

沿用户确认的14个S顺序，在S8实际远端复核后自动准入。M0固定、T26开放；S8 P1 `54c8b269`为当前技术与八文件基线，259项发布后聚焦257通过/2项管理员ETW skip、两项独立DWM旧失败保持，不扩大为全覆盖。

## 有限审计与实施顺序

沿S1已冻结的文件顺序，不随机挑选叶函数：先控件根状态/guard/安装和退场，再基础消息/输入，随后按钮与静态、编辑、列表、组合、滚动的快照与呈现边界，最后查产品attachment/controls、Win2000现有按钮、测试和构建消费者。实施前形成有限生产/测试清单与每项接收者，已有application/control机制直接复用，不复制第二套。

公共对象实际持有GUI身份、重入引用、原生状态和本层资源；完整操作回调提供字体/配色/绘制/资格，不用私有control_style或字段指针view包装。共同消息转发、状态更新、失败释放在公共层执行；同一原生协议由现有风格适用调用方消费。不为本S新增Win2000控件覆盖；扩展控件/VB/VCL独有呈现S10、创建链S11、现代宿主S12和最终注册/图形S13，根生命周期及共用协议不能以这些标签留在产品侧。

开发合批增量构建；覆盖原生返回值/LastError、焦点/选中/字体/DC、重入移除/销毁、失败资源回收及现有基础控件行为。所有fixture自建、有界、清理；不使用computer-use或私有桌面。完成整个能力清单后一次同快照双宽干净构建、先发布八文件再测、提交推送并读取实际远端；不逐文件完整交付。

## 状态

S9 active，有限清单已冻结；后续实施和验证事实按下方批次记录，不虚构已完成整个S或发布。按已批准目标自动继续，不要求用户重复准入。

## 覆盖冻结与第一批

实施前冻结86项源码/约束/测试，见[清单](../etc/evidence/t26-s9-controls-audit.json)。先公共控件状态/字体与protocol/重绘/制表位资源，再安装退场和基础消息，随后滚动事务；按已列文件顺序逐项填写结论，不将冻结计为已审。Win2000只有无状态DrawFrameControl导出，无可迁移的控件根实例；不新增接管功能来伪造两个消费者。独立公共测试需验证不同字体资源操作，实际Win3x消费新公共状态。S10保留扩展绘制/框架角色，不能保留共同资源与协议。

## 第一批实施中：公共控件资源状态

application/control/control_state实际拥有font/source、protocol、重绘抑制、native-paint深度与tabstop数组；GUI身份仍由既有公共style_instance拥有，按S7一样组合，无私有对象参数或字段指针view。公共实现执行字体观察/原生setter替换与退出结算、set-time tabstop事务和资源dispose；Win3x只保留System/VB字体选择及哪些控件保持native字型。原有tabstop私有view和实现已删，普通/扩展消费者同步读公共状态；combo/font专有池及安装/移除仍在本S下一步，不称整根生命周期完成。

登记build/t26-package/s9-state-first作双宽all增量及原生control-link/list-paint/font聚焦验证，日志和600秒Worker预算；无完整发布，结束提炼后清理。

第一批双宽all增量和每宽2项control-link/list-paint通过（20.25/27.76秒编译）；选择中的font-lifetime并非现存CTest名，没有计入通过。新独立public control_state测试验证font lease/native forwarding/fallback/退场successor、tabstop失败与重入提交，登记build/t26-package/s9-state-contract，使用真实存在的font/font-dpi96测试名；不重启已结束的构建或重新做干净发布。

独立control_state及既有control-link/list-paint在x86通过；误选的旧core/font和font-dpi96在CreateDesktopW行返回5，未创建/切换桌面，原失败保留且不计通过。测试执行缺口已修正：S9实际相关font/control-lifetime/tabs/control-present四夹具去掉私有桌面创建/切换，保持原生窗口、DPI/像素/资源/重入全部断言，在当前桌面仅使用自有实例。登记build/t26-package/s9-state-native，双宽all增量及公共state、control-link/list-paint/font-pair、修改后font/font-dpi96/lifetime/tabs验证；不修改产品范围或安全设置。

修改后font/font-dpi96/lifetime/tabs保持原断言，两宽各8项均通过（16去重调用），没有私有桌面。继续同一根生命周期批次：已有window/install通用事务移动到lifecycle/install，重命名为style_instance_install并由窗口和控件实际消费；不是新增第二套安装器。setup失败的provider负责完整rollback/脱离；窗口provider显式调用同一个style_instance_detach保持原S7退场，控件保留旧setup中的资源回滚和ERROR_CANCELLED，失败detach不能被公共层强行二次脱离。独立测试新增无rollback时仍保留已发布所有权的断言，不把FALSE称物理移除。登记build/t26-package/s9-install-contract作双宽all及根事务、控件状态/链接/lifetime/tabs和原窗口回归；字体source未再变，不重复224字形全量像素测试。

共享安装事务双宽all及每宽7项聚焦通过；来源954回执、原资源五头及声明、公共源码边界通过。登记build/t26-package/s9-state-runtime，沙箱外自有窗口执行已移除desktop创建的两宽control-present实际6类绘制/字体/资源路径，有限45秒，结束提炼清理；不启用全局样式或新增发布。

## 第一批效率复核与下一批

公共资源状态和实例发布事务是本批实质变化，已被生产实际接入；来源954回执、原资源与公共源码门禁通过，双宽all增量及原生字体/制表位/重入退场/绘制资源测试有对应原始输出。开发去重结果见development证据，未以一项成功代替整个控件层。首次误选私有桌面旧入口被拒绝，改为先查并移除本范围四夹具的desktop setup、保留全部断言后实际执行；不改安全设置、不把失败算通过。

本批不做完整发布或P占位。下一批明确为根移除/terminal和combo子控件身份及字体租约，再接基础消息/快照，最后滚动输入/资源；Win2000无根实例不新增功能凑消费者。制表位独立测试当前仍在旧appearance测试路径，下批随公共协议最终归位；不将其留到T结束。所有已结束scratch先提炼再清理，保留.ninja/.ccache。T26和S9继续开放。

## 第二批：组合框子窗口与字体池

根retire需真正控制combo/scroll资源，不能只包住Win16清理回调，因此先迁组合框状态和原生编辑子类：application/control/combo持有列表/编辑身份、subclass引用、32个既有字体租约及WM_GETFONT/WM_SETFONT退场结算。子列表owner归公共control_state，独立替换的child不继承旧parent。Win3x私有combo_text源/头已通过git mv归公共层，产品空的edit_proc/unbind/list-retire转发删除；renderer只提供实际字体acquire，lookup返回公共state对象。生命周期stop和scroll后续继续，不将本批称整S完成。登记build/t26-package/s9-combo-contract，双宽all及已有安全lifetime/tabs/control-link/state和公共combo原生测试；结束提炼清理。

组合框首轮双宽all及每宽5项既有根状态/lifetime/tabs/control-link通过；新增独立公共combo原生EDIT测试，验证32/33租约、重复绑定、借用font返回、WM_SETFONT中移除及successor恢复和child替换。登记build/t26-package/s9-combo-native作双宽all增量与公共combo/根状态及已有生命周期回归。

组合框独立原生测试及双宽all/每宽6项均通过。登记build/t26-package/s9-combo-runtime：同一增量输入的八文件开发包，执行两宽VCL三类combo及VB字体租约实际消费者；isolated_process显式--private-worker在CreateDesktop之前进入worker，不创建/切换桌面。结束提炼清理，不作正式发布。

组合框实际VCL dropdown/simple/dropdownlist及VB字体池消费者双宽8项全部通过，均显式当前桌面worker，未发布开发包。继续同一批基础消息：basic_message源/头归application/control/basic，原生bitmap借用DC、separator NC/PRINT、native-overlay顺序、button/static的重绘与输入转发由公共实现执行；Win3x仅提供实际kind资格与完整render操作。登记build/t26-package/s9-basic-contract，双宽all和combo/state/control-link/lifetime/tabs增量验证；实际两宽control-present随后复验，不为单文件完整交付。

basic公共消息首轮双宽all及每宽6项通过；新增独立basic实际原生STATIC bitmap/HDC借用隔离和原生overlay前后返回/回调移除测试。登记build/t26-package/s9-basic-native，双宽all与公共basic/combo/state/实际lifetime/tabs回归，不重复未变化的全字体像素测试。

公共basic独立与原消费者两宽各7项通过。根retire现由application/control/retire持有removing幂等、输入通知重入引用、实际subclass脱离、失败保留和退出后的原生surface刷新；provider仅提供完整live-bindings释放/恢复操作，已调用公共combo。Win16的颜色/字体/分隔符呈现选择及尚待迁的scroll caret/timer仍在明确的资源释放边界，不用该回调宣称全部资源机制已抽完。登记build/t26-package/s9-retire-contract作双宽all及根lifetime/公共basic/combo/state/tabs回归。

根retire独立原生测试及两宽各8项通过，覆盖通知中销毁、递归退场、解除失败保留和最终释放后刷新；登记s9-retire-native结果，前六批scratch已提炼清理。继续合批滚动资源：公共scroll_resources拥有caret/timer/借用capture，caret创建/显示/尺寸替换及foreign保护归公共实现，Win3x仅保留实际thumb/inset矩形；删除字段view和三个产品清理转发。制表位原测试迁application/control/tabstops，仅链接公共库，保留原断言。登记build/t26-package/s9-scroll-resources作双宽all及资源/根生命周期回归；仍不称整个滚动输入完成或S9交付。

滚动资源首轮all构建发现geometry夹具仍用已删除的caret字段view，编译失败且未运行测试，不计通过。已迁为公共资源对象；原detached空回调view断言随接口删除，由真实根lifetime验证退场不进入sync，资源测试实际检查caret尺寸/隐藏/foreign保护。登记build/t26-package/s9-scroll-native沿用增量；附加geometry真实输入/重入/拖拽断言保持。

资源测试新增目标需要Win10 API声明（scroll_state含GetDpiForWindow）；首轮修正geometry后暴露独立测试目标缺少_WIN32_WINNT，已只补测试编译定义，不改变生产API或策略。登记build/t26-package/s9-scroll-verified合批增量验证；两次编译失败均保留原输出，不重复改生产逻辑试错。

登记build/t26-package/s9-controls-runtime为最终本批（非S9正式发布）原生实际路径验证：同增量输入八文件、两宽VCL三类combo与VB字体、control-present六类绘制资源；只运行自有当前桌面fixture，全部有界，无全局启用。结束保存结果并清理。

本批滚动资源双宽all及每宽10项通过，实际产品VCL三类/VB字体及control-present两宽10项通过；四个错误夹具路径的原失败与修正对照保留。已清理全部完成scratch，累计开发40项去重调用。继续列表框原生消息整个能力：list_message.c迁application/control/list，私有头删除、实际呈现声明并入dispatch；公共层负责tabs/PRINT偏移/DC隔离、redraw/native消息及后置刷新，年代hover策略与实际画法由profile提供。产品set_tabs两个转发删除。登记build/t26-package/s9-list-native双宽all及独立公共list+已有root/geometry/tabs回归，不作逐文件发布。

列表批次配置发现core-style-control-list是既有快照测试目标；新controller测试改名core-style-list-message，保留既有目标及断言。已搜索旧私有头引用并移除产品include，登记build/t26-package/s9-list-controller继续增量。配置失败保留，不计运行通过；后续新目标先查重。

列表controller构建进一步暴露list.h与既有control_list.h的include guard同名（定义被跳过）；改用唯一STYLE_CONTROL_LIST_MESSAGE_H。两次失败均给出明确不同证据，未重启或改安全策略；效率调整：下一批先审既有target/guard/include图再增量验证，不重复纯机械试编译。登记build/t26-package/s9-list-verified。

新增list原生夹具实际安装公共实例，需像既有basic/combo/retire测试显式链接style-instance；补齐测试目标链接，不改变产品链接。登记build/t26-package/s9-list-result作为该批修正后的验证；先前configure/compile/link失败均保留。

## 本批复核与效率决定

列表controller完成双宽all增量和每宽12项聚焦，已有公共list快照及新的真实LISTBOX消息测试均通过，旧private路径/include不存在。当前开发去重44调用，来源964回执、自检、原资源和两宽各2个公共归档边界通过；原生VCL/VB/present的最新运行在list迁移前，迁移后以公共原生LISTBOX和实际产品tabs/lifetime覆盖消息协议，不称随后整个绘制套件重跑。全部已结束scratch的原始必要结果、失败与修正对照已提炼并清理。

本批实质推进combo、basic、list与retire、scroll资源；期间新增测试的目标/guard/链接和夹具参数审查不足造成多次短失败，下一批先检查既有构建与接口命名/消费者，再合并编辑和滚动输入改动验证，不重复全字体像素或每叶函数全套构建。当前仍有明确的scroll actions/input字段view与edit native协议待处理；不把有限公共化计为S9完成。正式八文件仍S8，下一完整发布只在S9退出条件全部落实后执行。

## 编辑与组合原生协议合批

恢复时前一goal turn判定为progress：实际迁移并验证list/retire/combo/basic/scroll资源，未收口S9。已重读活动包/规则及架构，CIM确认无存活ninja/gcc构建，保留全部未提交改动。

edit_message.c迁application/control/edit，旧私有头删除、render操作声明归既有dispatch；公共层执行native文本/撤销/选择消息、重绘抑制、PRINT DC保存恢复及后置surface刷新。边框资格仍由Win3x实际完整border操作决定。原combo消息从私有control_state迁既有公共combo，复用buffered/direct完整draw契约，绑定协议与消息协议可独立使用。三个现存滚动角色frame/edit/list归公共scroll_resources，后续输入引擎使用同一对象，不复制角色；原surface私有view仍明确待消除。

新edit原生fixture与扩展combo测试先核对目标名、头guard、Win10声明和style-instance链接；保留实际EDIT文本/选择/undo、原生PRINT返回/DC隔离、重入退场和COMBO选择/重绘断言。登记build/t26-package/s9-edit-combo，双宽all增量与公共edit/combo/list/资源、实际tabs/lifetime/geometry；600秒worker预算，日志run.log/run.err/result.json，结束提炼清理。整个基础控件批次完成后再一次正式交付，不逐文件发布。

本批首轮构建捕获scrollbars_paint临时快照的旧role字段引用；已将该只读快照逐role改为公共对象，不复制timer/caret/capture所有权。原失败留存。登记build/t26-package/s9-edit-native继续同一增量；源码搜索确认剩余旧字段仅为待迁的绘制view和attachment服务名。

原edit.c的profile资格判断迁dispatch后，需要显式包含公共control_class.h；已补直接依赖，不依赖旧头的间接包含。第二轮编译错误留存，登记build/t26-package/s9-edit-result验证。

编辑/组合公共原生测试、两宽all增量及每宽13项全部通过。登记build/t26-package/s9-edit-runtime，同一增量输入八文件开发包运行VCL三类combo、VB字体及control-present两宽10项实际生产消费者；当前桌面自有窗口、显式--private-worker在任何desktop创建前进入，有限超时并清理。edit-clipboard默认创建私有桌面/共享剪贴板，不在本批执行；新的原生edit测试已覆盖buffer/selection/undo而不触碰用户剪贴板，不将其称剪贴板验收。

## 编辑批次复核及滚动控制器边界

两宽all增量、每宽13项聚焦和两宽VCL三类/VB字体/control-present十项实际消费者均通过；开发累计46项去重调用。原始失败及修正保留，已清理完成scratch，仅保留.ninja/.ccache。来源965回执、五原资源、公共control/instance双宽归档及13项治理负例通过。单人复核实际diff：profile边框资格在调用时判断；公共edit直接读同一scroll role对象而非布尔快照，保留原生重入变化；scrollbars_paint只复制role/presentation快照，不复制timer/caret/capture资源；combo消息独立于edit-child绑定可用，完整renderer同步借用上下文。普通API/产品ABI/八文件未增加，正式包仍S8。

下批按已审scroll_layout/track/native/message顺序完成一组控制器，不逐叶函数发布：公共层定义layout/part/action结果契约并持有track、role、epoch、reporting/key、capture/timer/caret；Win3x保留固定thumb测量/命中优先级、重复延时与拖拽容差策略。现存动作应用、原生input与消息协调迁common controller，provider提供完整layout/hit/render操作，删除action/input/embedded字段view及产品装配。保留全部原geometry/通知时序/重入断言并增加公共不同几何/时序的原生测试；不能将Win3x固定thumb算法直接命名为所有年代通用算法，也不新增Win2000控件。完成后再合批重跑实际消费者和S9清单终审。

## 滚动控制器能力批次

上轮判定progress：edit/combo消息协议实际公共化、双宽与原生生产路径验证、临时文件清理；本轮CIM确认无已有构建。公共scroll_geometry只定义调用方相对的layout/part结果，Win3x的固定thumb测量与命中优先级仍在appearance。scroll_track迁公共层，hit、initial/repeat与拖拽容差通过完整policy传入；实际Win3x保留200/50ms、4/1逻辑像素，未发明Win2000控件。

application/control/scroll_controller持有track/resources/epoch/report/key/object，执行控制条/嵌入条input、SetFocus/捕获/计时器、native通知和结束；旧actions/input/embedded字段view及产品拼接删除。renderer只借公共控制器完整只读对象与palette，不再借逐字段指针。原geometry测试的所有命中/拖拽/时序/重入断言接同一公共事务；新增独立track两种命中/延时策略和原生controller通知/捕获/计时器/epoch/销毁测试，不链接Win3x。

登记build/t26-package/s9-scroll-controller，双宽all增量及公共scroll-track/controller、所有已改base控制器与实际geometry/lifetime/tabs；600秒Worker预算，日志run.log/run.err/result.json，终态提炼清理。native scroll消息与管理API仍待后续同S完成，不把input公共化称整个S9完成。

滚动controller首轮两宽all及每宽15项全部通过，测试结果已保存并立即清理scratch。继续同批管理机制：scroll_managed公共实现拥有active/原生get/set/role-enable/frame-remove，provider仅给public instance+controller绑定、真实native后端、完整draw/资格；Win3x只留实际palette更新，不再保留通用get/set实现。set后的fresh binding绘制使用既有公共实例retain/release，保证render回调中退场时资源保持到回调返回，不改native返回值/LastError和redraw规则。style-control-common显式声明已有style-instance依赖，消费者不再靠偶然链接顺序。

登记build/t26-package/s9-scroll-managed作双宽all及独立managed（native error/redraw、report/track、资格拒绝、原生setter中替换、draw中退场引用）、公共track/controller和实际geometry/lifetime/tabs回归；日志与600秒预算同前，结束提炼清理。

scroll_managed两宽all及每宽16项首轮全部通过。登记build/t26-package/s9-scroll-runtime，同一增量八文件开发包在当前桌面运行VCL三类combo、VB字体、VCL滚动default/aware及control-present两宽14个实际消费者，另两宽scroll-native原生API边界对照（不冒充产品接入）；显式--private-worker/无desktop API，有限超时并清理。结束后统一提炼，保留.ninja/.ccache。

本批原生调度发现执行错误：core-scroll-native旧入口仍直接CreateDesktop/SetThreadDesktop，预检只读了部分文件未搜完整入口；x86已运行返回0，但创建私有桌面违反当前边界，不计本批符合约束的通过。代码无SwitchDesktop、进程已退出，桌面引用随进程释放；未切换输入桌面。已按精确命令行/PID/启动时间只停止自有Python调度器PID13216，阻止x64对应旧入口；当时自有VB字体子进程PID4448仍有界自行完成，不结束用户进程。原始11项结果保留。

测试修正仅删除scroll_api桌面创建/分配及过时注释，所有API/列表/编辑/原生边界断言保持，在当前桌面自建窗口执行；生产源码及开发八文件不变。登记build/t26-package/s9-scroll-api-current，双宽core-scroll-native增量及原生API验证；不因入口错误重做产品构建或改变安全设置。之后只补调度未完成的x64四项（font、VCL scroll两种、present），同一开发包，不重复已通过项目。

## 滚动批次证据与效率复核

公共track/controller/managed两轮all增量及双宽16项聚焦均首轮通过；两宽VCL三类combo/VB字体/VCL scroll default+aware/control-present十四项实际消费者通过，修正后的原生API双宽两项通过。开发累计58项去重调用，不计旧私有桌面运行结果，不把同一用例重复轮次累加。来源973回执、保留固定source blob/声明及五原图形、公共源码/两宽control+instance归档、治理13负例通过。原scroll_track头由retain明确改adapt，记录实际public契约改动，不假称字节保留；一次绝对回执重复及未更新来源receiver均被门禁拒绝，已修正后复验。

全部本批scratch已提炼并清理，CIM确认无存活包占用及被中止调度的自有子进程。生产C/H含新文件对HEAD净增加242行，实际复用已有输入/动作/native API代码并删除字段view；此统计不含测试/文档，不把全部新增测试算产品footprint。统计改用单个git cat-file --batch，避免上一轮逐文件git show的无效耗时。

效率问题：核心批次已避免目标/guard/链接试错，首轮构建通过；测试入口审查仍不合格，必须在选择每个legacy fixture前对全文件和入口helper搜索CreateDesktop/SetThreadDesktop/SwitchDesktop/CreateWindowStation及实际参数，不能以局部读段/文件名或头注释代替。新选中的scroll-api现已移除桌面API；隔离helper消费者必须显式worker。下批只处理scroll/frame/sizebox原生消息与基础surface/item快照，不重跑未变化的全字体像素套件，不逐文件正式发布。S9及T26继续开放。

## 滚动消息与基础原生绘制快照批次

上轮判定progress：track/controller/managed实际公共化、两宽原生回归、误选desktop入口修正和清理。恢复后核对活动包/既有修改及CIM，无存活构建，不覆盖已有工作。按既有架构继续，不单独准入新任务。

scroll_message源/头迁application，直接调用公共controller，三个原生路径frame/control/sizebox保持返回、参数改写及重绘顺序；native_border归公共应用状态，Win3x的progress overlay留完整profile绘制操作。旧scroll服务结构和两个产品转发删除。list_snapshot拥有原生可见行/selection/text临时所有权、单/多列原生坐标与DC隔离；Win3x只保留配色/字体选择/tab文字及focus绘制，旧font/tab/detached字段指针接口删除。scroll_surface统一客户区/嵌入条原生信息和DC坐标快照，两处Win3x消费者实际接入，不混入固定thumb算法。

新独立三测试先查guard/target/链接/入口，均不含desktop创建或间接helper。登记build/t26-package/s9-base-snapshots，双宽all增量及新scroll-message/list-snapshot/scroll-surface、已有公共base协议和真实geometry/lifetime/tabs；600秒Worker预算，日志run.log/run.err/result.json。能力合批完成后一次正式交付，当前只做增量开发验证。

首轮双宽all在x86执行新scroll-message测试时失败，其余18项通过：测试误把普通原生SCROLLBAR当成已绑定verified runtime协议，期待disabled键盘继续转发77。原生产代码只对protocol_is的已绑定运行时分支转发；普通原生控件由input直接消耗返回0。已修正测试oracle，明确断言返回0/未进入native witness/未产生key状态，不改生产代码。绑定运行时disabled鼠标/键盘仍由实际VCL fixture验证，未放宽协议资格。登记build/t26-package/s9-base-native沿用同一增量。

scroll-message/list-snapshot/scroll-surface修正测试oracle后，两宽all及每宽19项全部通过。根安装审计另定位protocol acquire和初始tab副本仍由profile手工拥有；同批补归control_state_prepare，初始化资源由同一公共对象与dispose管理，调用方仅选择资格/初始像素并在构造失败释放其根对象、原错误向上。新增初始tab独立副本/释放和无效参数原生测试，不重复安装器或新增容错路径。登记build/t26-package/s9-base-resources，双宽all增量和同19项合批复验后再一次实际消费者测试。

根资源构造合批后双宽all及每宽19项全部通过。登记build/t26-package/s9-base-runtime，同一增量八文件开发包运行两宽VCL三类combo、VB字体、VCL scroll default/aware和control-present共14实际消费者；本轮完整搜索每个源与helper的桌面入口，六个legacy消费者显式--private-worker在CreateDesktop之前返回，present无桌面API，不运行其他未知入口。均有限超时，终态保存并清理；不重新计数重复调用或作正式发布。

## 本批复核与剩余范围

公共滚动消息、列表/滚动surface快照和根resource prepare已实际接入；双宽all/每宽19项及两宽14个实际消费者均通过，累计64项去重开发调用。来源980回执、原资源五头/说明、公共源码/双宽两归档和治理13负例通过；错误disabled测试oracle的首轮失败保留，生产分支未为测试改变。完成scratch已按终态、CIM和reparse核对清理；生产C/H对HEAD净增336行（含新文件、排除测试文档），保留.ninja/.ccache，正式包仍S8。

单人第二角色复核实际diff：SBM改写/native return/DC restore顺序保持；verified runtime disabled由实际VCL覆盖，普通native由独立测试覆盖；初始tab为owned副本，根构造者失败释放并bubble原错误；字体/颜色/label布局由profile提供。

下一批明确为choice_view/combo_paint的native数据和text所有权、旧font/detached接口及根combo子对象setup，仍属S9，不能因可编译而收口。basic_paint只有Win3x角/阴影/原位图绑定可留；choice_paint的runtime/DC/模块见证/TLS重入交S10，control_scope创建期间combo祖先跳过交S11，other_paint::fit_push_text的DC/bitmap缩放交S13，都是通用机制且有接收者，不笼统称年代绘制。审计元数据曾将同名旧测试路径误作生产路径，写入前异常终止；改用精确src前缀，保留测试的既有迁移记录。其余冻结项继续逐文件真实审查。

本轮按消息+列表/滚动快照+根资源合批，只在合并后复验实际消费者，不重复未改字体大套件，不逐文件正式发布。S9及T26保持开放。

## 按钮/静态/组合快照及子对象setup批次

上轮为progress：scroll消息/list与scroll快照和初始资源对象真实公共化、双宽/实际路径验证并清理。恢复时读取CURRENT/brief及规范；CIM确认无存活构建，沿用全部未提交工作。

button_snapshot拥有BUTTON状态/check、STATIC icon借用与窗口text副本，角色和font由完整provider给出；combo_snapshot拥有native info/text/drop/simple/editable读取与text清理，字体/子HWND借用。Win3x choice/combo主绘制接口改为公共instance/state，旧detached/font字段view删除；common_dialog_combo只在构造时设定，作为不变绘制配置值传入，仍在profile决定原生子内容裁剪与实际像素。读顺序、空font回退、图标大小和文字布局保持。

combo_setup统一现有child-list发布/取消及父回滚，provider只提供native info、完整list安装/remove和受支持edit动作；private setup不再保留这套事务。失败解除保留原所有权，根引用由既有install事务pin；不添加物理卸载、权限或目标。独立combo测试新增七种查询/安装/编辑失败、安装/编辑中移除、回滚失败和成功路径。新button/combo snapshot测试验证实际native角色/check/text/三种combo/借用font/icon、query失败和回调移除清理。

已查新target/guard/链接/Win10声明和完整测试入口；无desktop API/间接helper。登记build/t26-package/s9-button-combo，双宽all增量及新snapshot/扩展combo、既有基本协议和actual geometry/lifetime/tabs，共21项每宽；600秒Worker预算，run.log/run.err/result.json，终态提炼清理。合批完成后一次实际消费者回归，不逐文件正式交付。

首轮x86 all构建通过、20项通过，combo_snapshot合并断言失败；不改生产getter，改将初始pressed与安装前直接CB_GETDROPPEDSTATE独立基线比对（不假定simple关闭），并打印kind/句柄/font/text/flags以给出新证据。登记build/t26-package/s9-snapshot-native继续同一批增量及21项。

实际CTest完整输出确认CBS_SIMPLE原生pressed=1，dropdown/dropdownlist均0，快照与原生一致；初始失败是测试假设，不改生产代码。两宽21项随后通过。剩余other_paint图像审计定位BM_GETIMAGE/BCM_GETIMAGELIST及尺寸读取，合并到button_snapshot的borrowed-image契约；私有painter只保留排列/状态图索引和实际绘制，detached指针也替换为公共instance。独立按钮测试追加bitmap/list尺寸及列表借用、image-query中移除；fit_push_text按原记录交S13。登记build/t26-package/s9-native-images，同21项增量复验后统一实际消费者，不重复正式发布。

图像读取合批首轮编译识别SIZE.cx/cy为LONG而native图像尺寸API参数是int*；不用强转掩盖，改局部int宽高取得后赋SIZE，保持32/64位原生类型契约。保留失败日志。登记build/t26-package/s9-image-native继续同21项增量。

类型修正后双宽all与每宽21项全部通过，原资源/ABI未增加。登记build/t26-package/s9-snapshot-runtime：同一增量八文件开发包运行两宽VCL三类combo、VB字体、VCL scroll default/aware与control-present共14实际消费者。已完整核对每个入口及helper，所有isolated_process消费者显式worker在CreateDesktop前返回，present无desktop API；新unit均无间接桌面创建。有限超时，终态提炼清理，不重复未改字体大套件或新建正式发布。

## 快照批次完成及文件顺序复核

按钮/静态与普通组合框的原生状态、复制文本、字体和子窗口快照已由application/control实际拥有，profile借用完整instance/state；图标、bitmap和image-list仍属应用，只查询尺寸，不销毁借用句柄。普通combo setup的查询、子列表安装、owner发布、编辑绑定及取消/失败回滚由公共事务执行，独立测试覆盖七条成功/失败/重入路径。实际Win3x消费者接入，没有新增Win2000控件。

最终增量两宽各21项全部通过；同批八文件开发包的VCL三类combo、VB字体、普通/aware VCL滚动及六类control-present两宽14调用全部通过。累计68个去重开发调用（54 CTest和14实际产品调用）。来源986回执、原资源、公共control/instance双宽归档边界及13项治理负例通过。原生simple组合框的CB_GETDROPPEDSTATE返回1，测试改为与同窗口原生结果对照；首次错误断言和图像尺寸int/LONG编译失败保留，不算通过。

两个最新scratch的结果/八文件摘要已提炼，CIM无存活自有实例，核验绝对前缀及reparse后立即清理；稳定.ninja/.ccache和正式S8包保留。文件顺序复核将扩展状态/通知/见证及产品attachment明确交S10，创建scope及old-file-dialog pending/子控件协调交S11，fit_push_text的通用DC/bitmap和最终render契约交S13；没有把这些未迁移部分标作年代绘制。基础消息dispatch现为公共完整操作的真实消费者，扩展分支还须后续S处理，不能声称产品已全面干净。

效率复核：本批实际生产消费者与公共独立测试均有新证据；恢复后的终审没有代码变化，不重复构建或全字体像素测试。下一步完成有限清单的剩余终审，再进行一次S9整批干净八文件交付；不为单个文件或审计标记发起完整发布。T26/S9保持开放。

## 整S交付准备

103项有限输入（原冻结86项及后续实际消费者）全部已有逐项处置；跨S文件明确只完成本S边界审计，不称整个混合文件已抽离。基础控件机制实施/开发验证完成，准备S9 P1的唯一整批干净构建。登记build/t26-package/s9-delivery为同快照源码/双宽构建及八文件，build/t26-package/s9-publish为两处备份，build/t26-package/s9-package-tests为发布后实际包测试；精确用例选择在启动前核对所有入口，不执行默认private-desktop分支。长任务直接Worker受监控执行，构建每宽600秒；结束提炼并清理，保留稳定增量缓存。最终发布/测试/提交/推送/远端复核均仍待完成，不提前收口。

## S9 P1验证与发布事实

同快照八文件干净构建及全部门禁通过：x86约160秒、x64约187秒，1080个源码/测试/工具输入摘要无差异，双宽各21个公共样式归档及4个进程归档边界通过。原五资源/声明、18栅格表和产品ABI保持。八文件先发布assets根及O:/winnt并核对SHA，再执行146项去重实际包聚焦调用，全部通过；两项Win2000 DWM restored=0旧失败仍失败，不计通过。不是所有第三方程序或物理键鼠验收。

首轮传播夹具因驱动遗漏WIN16STYLE_CAPTURE_DIR在入口断言失败；原结果保留，补齐每用例capture目录后从失败项继续，未重复14个已通过调用，无产品改动。最终详细结果、工具链、来源/资源/依赖、包摘要和失败边界见[整批交付](../etc/evidence/t26-s9-delivery.json)。三处本S临时目录的源码副本/构建/测试输出/发布备份在CIM无存活自有进程、绝对前缀/reparse核验后立即提炼清理，保留.ninja/.ccache和正式两处八文件。

当前verified/published，提交推送及实际远端逐blob复核尚待完成，不能提前closed。生产源码含新接口净增478行（+1606/-1128），删除旧basic/combo/edit/list/scroll协议路径，实际公共状态和生命周期已接入；产品扩展/创建/现代及最终renderer契约分别S10-S13接收。

## S9收口与S10自动准入

P1 `f9de34045556b25147a3ced86506afacc674abee`已推送并实际fetch：107个现存blob、16个删除路径及八个产品摘要均复核，见[远端证据](../etc/evidence/t26-s9-remote.json)。单人复核读取实际diff/公共对象、产品消费者和失败原始输出后确认：本S基础状态、安装退场、原生协议与快照、滚动输入资源由公共层持有，profile只留下现有呈现/资格及明确后续S接收的扩展机制。S9按有限范围收口，既有DWM等未通过不被关闭，T26保持开放，自动准入S10扩展控件与运行时适配。关闭记录无生产输入变化，不再次构建或发布。
