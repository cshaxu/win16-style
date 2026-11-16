# M0 T26 S8 菜单

S7 P1 cb84dac13c7401a16ed41d1bfb358af8c732cc63完成实际远端58个现存blob、3个删除路径及八文件复核后自动准入。M0/T26保持；本S仅菜单能力，不增加新风格、目标范围或权限。

## 有限审计与迁移顺序

冻结入口为application/menu、appearance/win3x/menu、Win2000 system_menu及frame内菜单部分、Win3.x frame输入/菜单状态/派发与产品menu_filter装配；现代外框仅核对其菜单动作与生命周期消费，宿主/表面S12。来源图形资源留appearance并保持字节，不迁入通用库。逐路径与起始摘要见[输入清单](../etc/evidence/t26-s8-menu-audit.json)，清单不是完成结论。

先按文件审计实际HMENU/itemData/文本/字体/弹出节点、线程hook、焦点和scope所有者，识别可复用的现有menu_items/snapshot/input/owner/row/host_menu机制；然后按popup观察与scope、菜单项准备恢复、栏/系统菜单输入与迭代三个完整能力批次实施，保持一个生产实现。错误本层释放后上抛，外层报告，不通过大量字段视图回调保留私有耦合。两风格实际支持相同机制时必须直接接入；不同布局、选择画法、字体、原资源与命令策略由完整renderer操作提供。

特别保持应用原HMENU、owner-draw、WM_INITMENUPOPUP/MENUCHAR/NEXTMENU及命令返回语义；同步回调可移除、销毁或替换owner，只有还存活的同一身份可继续访问。nested scope、非栈顶结束、SysShadow配对、打印坐标、菜单关闭与焦点恢复必须有唯一状态与资源所有者。不得为未来风格建立空实现，也不复制另一套挂钩或窗口生命周期引擎。

## 验证与退出

公共纯C独立测试、两风格真实自有菜单和产品组合验证：初始化/owner-draw/助记符/命令、栏与系统及嵌套popup、取消/重入销毁/替换/失败清理、字体与打印DC/坐标、重复启停和双位宽；完整窗口回归保持S7能力。实际消息协议和真实鼠标键盘分开报告，不能用定时器或合成消息冒充物理输入通过。

只在当前桌面运行有限等待/自行清理的自有fixture；执行前检查并排除所有私有桌面创建/切换路径，不用computer-use，不改系统安全配置。环境限制和既有Alt+Space/恢复债务如实记录，不拓宽本S绕过失败。开发优先增量，全部机制/消费者合并后一次同快照八文件交付：干净双宽构建、先发布再实际包测试、提交推送、真实远端复核及临时产物清理。

完成谓词：冻结每项有最终所有者/公共入口/实际消费者/测试，通用菜单状态与生命周期不再藏在年代或产品文件内；风格差异有具体原因；旧算法/空转发删除，资源/ABI和既有支持边界保持，规范交付完整。S8收口后自动S9，T26只在S14最终全范围复核后收口。

## 第一批顺序审计与运行时约束

S7 P2 809043f156d9319d7a37036e7034bd119d02563f亦已实际fetch并复核9个文档blob；八文件摘要仍与S7测试一致。当前S8未开始源码实施，没有活动构建或临时快照。

按冻结顺序完成前三个生产输入：Win2000 frame.c菜单部分、system_menu.c/h。frame内active_menu/WH_MSGFILTER、栏选择/焦点/键盘状态和迭代仍是可共享机制，compact/RTL/owner-draw资格、位移和绘制才是实际profile策略。system_menu内部scope栈、WH_CALLWNDPROC、popup子类/引用/停止及idle选择刷新必须下沉，不能仅把文件mv到公共目录；Tahoma配色/系统glyph/行绘制留appearance。现有menu_snapshot/input/present只是已共享的叶机制，尚非完整菜单引擎。

关联读取Win3.x menu_scope确认也有独立TLS scope/thread_hook及popup/影子/引用退场；该文件仍须依顺序台账正式处理，未把关联阅读当整批审计完成。需特别避免两个产品DLL各静态复制一份带TLS的引擎而保持两个独立后端。既有theme_state已示范公共代码由ntwshk承载并对调用方导出，但它显式延迟解析GUI函数，ntwshk包门禁固定只导入kernel32/advapi32。菜单公共化不得直接引入USER32/COMCTL32或loader-lock GUI初始化；追加CMake/共享导出/现有lazy模式/包门禁四个必要约束输入（共47项）继续审计，旧ABI保留。具体接口在完整清单审计后冻结，不为此增加T/S或新能力。

本批为证据和设计推进：未新建构建副本或运行未经检查的菜单fixture，未发布新二进制。下一批继续Win3.x菜单文件顺序审计，记录状态/资源/回调差异与测试消费者，再合批实施同一个公共scope与输入生命周期。

## 第二批顺序审计及验证入口修正

全部32项生产菜单输入及4项运行时约束已按清单阅读分类；10项测试已审，Win2000大夹具额外菜单模式仍待补读，不把47项清单称为全部审结。逐项原因和符号见审计JSON。Win3x bar快照、焦点/键盘状态、popup迭代、INITMENU/NEXTMENU跨窗口引用、owner菜单深度和产品GETMESSAGE hook都是待迁机制；System字体、MDI呈现单元筛选、列宽/边距、glyph和禁用文字点阵属于具体外观。

关键差异不能抹平：Win3x从WH_GETMESSAGE且PM_REMOVE观察，Win2000从WH_MSGFILTER/MSGF_MENU观察；同一公共tracking拥有栈与退场，按既有模式调用，不擅自改为一种消息来源。现代窗口目前借用完整style_window只为菜单，S8应以公共菜单owner替代这一耦合，表面/标题宿主生命周期仍归S12。

实际修正了menu_dispatch夹具的无条件CreateDesktop/SetThreadDesktop入口，保留原80轮UAH/资源及点击损伤验证，改用当前桌面自有窗口；尚待本批增量构建运行。其他isolated_process夹具已核对显式--private-worker在任何CreateDesktop之前返回，后续只能这样调用。未使用computer-use或私有桌面，未变更产品代码/二进制。进程只读CIM查询被当前沙箱拒绝，退到Get-Process确认没有ninja/cc1/gcc/cmake；没有据失败重启任何工作。

效率复核：本批集中阅读并分类菜单生产边界，未逐文件构建/发布；下一步补齐一个大测试的剩余菜单分支，冻结三批具体接口，再合批实现scope/owner/tracking。增量验证scratch登记build/t26-package/s8-menu-entry，用于双宽菜单dispatch构建与结果；测试完成提炼原输出后立即清理，保留.ninja/.ccache。首次证据写入因Python默认cp1252读取中文文档失败，测试源码和JSON已写入；后续显式UTF-8修正，没有覆盖既有工作。不得把审计、测试入口修正当作共享菜单引擎已完成。

增量构建首次完成x86目标，但我传入不存在的CTest模式，门禁正确以No tests were found拒绝（非测试通过）；不重复该命令。实际双宽原生运行改用已核实的core/menu-dispatch，登记build/t26-package/s8-menu-entry-native；以exec会话保持Worker生命周期，首个脱离父会话的后台PID1108已消失且空日志，没有仍在运行的构建。两次启动事实与结果均保留，不将启动成功算作构建成功。

## 有限审计冻结与三批实施契约

补读frame_target菜单余下分支，并加入原冻结清单遗漏但直接相关的两个input/dispatch头与三个现有菜单集成夹具（menu_initial/system_menu_visual/demo_system_menu）；总52项：38项生产/运行时约束、14项测试。逐项只审本S菜单部分，Win2000大夹具其他控件/对话框不冒称全文件功能均已复核。有限审计到此冻结，按下列三批实施，不继续横向无界搜索。

1. application/menu/popup拥有唯一线程scope/observer栈、popup/shadow身份、回调引用和脱离退场；外观提供完整资格/绘制及真实选择观察操作。实现由既有ntwshk承载，按需解析GUI API，兼容追加公共C符号；不增加DLL，不改console-safe导入门禁。两风格不得各链接独立TLS后端。
2. application/menu/owner拥有菜单depth、成功scope记录、临时item链和初始化前还原/初始化后准备/退出按depth还原/助记符转发；Win3x临时owner-draw和Win2000原生项策略保持不同，不能为了共用强行改变呈现。现代菜单消费此对象，删除其为菜单而借用整个style_window的耦合；宿主S12。
3. application/menu/tracking及bar快照/input拥有tracking栈、明确的现有hook模式、选择/键盘/焦点和popup迭代。完整operation提供几何/measure/支持与跨窗口目标资格；公共层保活NEXTMENU交接，不传私有字段指针。移除产品tracking_menu/filter/begin/end、Win2000同类TLS/hook及私有选择状态路径；实际命令、MDI呈现和箭头/鼠标策略保持。

具体接口和删除标记记录在JSON migration_batches/completion_contract；三批相关修改合并后只做一次S8完整交付。公共独立测试覆盖nested/non-top、回调中退出、线程归属、失败/取消、打印坐标和同选择不重画；两风格实际native回归与现代菜单动作继续检验，不用直接renderer手绘成功代替实际产品路径。

menu_dispatch双宽当前桌面原生回归已通过：有效增量x86构建0.67秒/测试8.16秒，x64构建5.20秒/测试7.81秒；源/包ABI检查也通过。结果仅证明测试入口修正及原菜单基线，非共享引擎完成，未发布新产品包。原始输出、失败模式及工具结果见[增量证据](../etc/evidence/t26-s8-menu-development.json)。

本批第二角色复核：对实际测试diff仅删除私有桌面创建，未削减80轮及损伤断言；双宽原始CTest结果各1通过、无skip，不能推导新共享实现正确。治理及13个拒绝负例通过，git diff --check通过；两个已终止scratch核验绝对路径/reparse后移除共31,363字节，无存活自有fixture/编译器，cache保留。下批直接实施已冻结的popup公共运行时，不重开有限审计或重复本次基线测试。

## popup公共运行时实施中

按冻结第一批实施application/menu/popup.c/h：共享scope/observer与节点引用、阴影匹配/恢复、native forwarding和退场由公共对象拥有；两种风格的scope私有TLS/hook/subclass实现已替换为实际呈现操作。新增兼容named导出style_menu_popup_begin/end/find/scope_context/idle/window/context/live，旧ordinal不动，依既有ntwshk按需解析GUI模式；尚待构建验证，不能称交付。renderer的选中观察保留Win3x完整64位mask和Win2000首个native选中索引，原生NC/PRINT与键盘刷新差异按实际operations提供。

独立appearance测试须显式提供公共popup实现，实际产品只由共享ntwshk承载，不能给每种renderer静态装入第二个TLS后端。当前开发scratch登记build/t26-package/s8-popup-first，Worker日志及超时由Build-Core统一管理，双宽增量及聚焦独立/产品测试后提炼清理。菜单owner/输入仍在后续批次，不称S8完成。

首轮all增量发现ntw2ks导入库位于静态renderer之前导致未解析公共符号；公共实现已编译、ntw16s已链接，未冒称整批成功。调整现有产品链接顺序，不让renderer反向依赖产品；独立公共popup测试同时编译静态与共享runtime两种承载。第二轮scratch登记build/t26-package/s8-popup-native，双宽all及popup/menu-input聚焦验证。

第二轮新测试回调accept与winsock声明重名，编译期拒绝；改名accept_popup，公共源本身已编译。第三轮增量及两种承载原生测试scratch登记build/t26-package/s8-popup-verified；前两轮终态日志待提炼后清理，不覆写失败证据。

第三轮显示其余独立静态测试同样受GNU archive顺序影响；改用测试侧明确的LINK_GROUP:RESCAN组装renderer与所选公共实现，不给生产renderer增加指向ntwshk的反向构建依赖，不修改生产权限或行为。先只构建/运行两个公共承载测试和menu_input拿到运行证据，再全all检查，避免全树链接失败挡住独立验证；登记build/t26-package/s8-popup-contract。

公共popup静态与共享承载的双宽原生契约均通过，每宽各3测试（含既有input）；nested/non-top/foreign-thread/native/accept与paint回调退场已实测。下一轮登记build/t26-package/s8-popup-consumers，用all增量核实所有静态消费者及当前两风格实际菜单回归，不把独立测试当作全S已验证。

双宽all增量及每宽4项popup/input/menu-dispatch均通过（x86构建4.29秒，x64构建14.88秒），未新增私有桌面。登记build/t26-package/s8-popup-runtime作同一增量输入的八文件开发包及实际风格菜单样本：不发布/不当最终干净快照交付，测试只创建自有窗口，有界超时，结果提炼后立即删除。原资源/来源及公共反向依赖门禁已通过，939项来源回执。

## popup第一批实际验证与第二角色复核

全部双宽all增量链接已通过；独立static/shared各两宽，加既有input及menu-dispatch共8项去重通过。实际两风格开发包另18项通过：Win3x cancel矩阵每宽30场景（15场景×2种DPI上下文）及normal/hover/native-string；Win2000每宽live/switch/keyboard/hover/custom。合计26项独立测试调用，不重复累计开发重跑。第一次Win2000命令把--2k放在--wait之后而未进目标，保留十项失败原输出；按已有CLI顺序修正后十项全部通过，未改产品解析器。公共门禁939来源回执及全部负例通过，共享DLL实际PE非CRT导入仍仅kernel32/advapi32、旧helper ordinal1保留；新named接口均存在。不是干净交付包/发布或全S完成，不把消息/WM_PRINT样本冒称物理键鼠。

第二角色检查实际源码：两种风格旧scope TLS、CALLWNDPROC/subclass及shadow/ref退场已删除，公共对象拥有这些资源，renderer仅保留真实paint、selection、资格和字体/配色。Win3x完整highlight mask与Win2000首个highlight观察保持各自语义；context仅成功begin后转交，最后popup回调返回才dispose；同线程/非栈顶/accept中结束/paint中结束已由真实原生菜单覆盖。仍有Win2000 begin/set/idle/end调用适配和Win3x兼容scope API，下一owner批次继续精简，不把所有菜单应用机制称已抽完。没有改变普通/global目标范围、权限或传播后端。

效率复核：首次链接顺序、测试回调命名及静态测试依赖组的失败已逐项记录，改为先跑独立契约后all消费者，避免在同类失败上盲目重跑全树。唯一生产实现只进入ntwshk，独立测试选择静态承载，产品renderer无反向依赖；所有正式资源未变。下一批直接实施公共menu owner深度/项恢复与消息协议，随后tracking/bar状态，不再重开popup有限审计。

本批原资源五头/来源逐字节与6负例通过；公共popup x64归档undefined-symbol边界通过。原始构建和native输出已提炼入development证据后，六个终态scratch核验路径/reparse并删除5,555,857字节，另清理本次PE检查产生的8,579字节pycache；未保留开发包，未删除.ninja/.ccache或正式assets。产品侧状态仍留owner/tracking待拆，不能用本批通过收口S8。

## owner第二批实施中

公共application/menu/owner.c/h拥有depth/成功loop/token/item链、失败内层退出、初始化前还原/后准备、native优先助记符与最后停止；实际native菜单项和adapted owner-draw是显式既有策略，不把Win2000强制改为Win3x呈现。普通Win3x、Win2000 frame及现代surface/owner已切换。现代对象直接内嵌公共menu owner，删除仅为菜单分配完整style_window和借用其字体字段的路径。

Win3x menu_message.c/h删除，真实字体/配色/draw/measure操作合并menu_items；frame_state三条菜单生命周期实现删除，所有停止路径直接调用公共owner。Win2000 begin/set/idle/end垫片删除，只提供实际popup呈现操作；frame对象的销毁负责公共owner停止。旧control_link伪造私有menu_loop的断言迁为独立public owner测试的真实failed-inner入口，保留其字体销毁断言。独立测试同时覆盖native/adapted协议、原itemData恢复、app mnemonic优先和回调中停止，不减少原契约。

本批scratch登记build/t26-package/s8-owner-first：双宽all增量和owner/popup/input聚焦测试，Worker日志/600秒预算；结束提炼后清理，无新完整交付。所有既有popup批次改动保留，当前无其他运行构建，不重启已有工作。

owner首轮CMake生成拒绝将通用menu_items与其依赖方renderer一起重扫描造成的组依赖环；缩小组到所选owner/后端提供者，公共menu_items按原拓扑解析，不为链接顺序制造反向依赖。第二轮增量scratch登记build/t26-package/s8-owner-contract；首轮未编译，保留生成错误。

第二轮双宽首架构all编译通过，独立owner测试合成DRAWITEM未给原生HDC，返回无效句柄而未到达renderer；补齐真实DC/菜单句柄并输出native返回与回调计数作诊断，生产协议未改。第三轮增量scratch登记build/t26-package/s8-owner-verified，继续原契约，不把首轮测试失败算通过。

owner双宽all及每宽4项聚焦原生契约通过。第二角色精简：owner已直接保存scope token，不再需要lookup，移除本S尚未发布的style_menu_popup_find候选导出；同时删除非导出Win3x end空转发，消费者直接调用公共end，真实palette创建仍由renderer提供。不改变既有发布ABI。登记build/t26-package/s8-owner-consumers进行最终增量和相关产品/现代消费者验证，再提炼清理，不为单文件做完整交付。

双宽all及每宽6项公共/菜单重绘/Win2000实例生命周期全部通过。复核把新owner测试中停止后scope为空的间接断言改为保存实际已创建token、检查其确实已从公共scope移除；只增量重编该测试，登记build/t26-package/s8-owner-test。另登记build/t26-package/s8-owner-runtime：同输入八文件开发包验证实际Win3x/Win2000及modern菜单消费者，有界自行清理；不是最终发布。

## owner第二批验证及复核

双宽all增量通过，每宽6项公共owner/popup/input、菜单重绘和Win2000实例生命周期通过。scope退场断言增强后只重编owner测试，两宽通过。实际消费者另24项全部通过：每宽DC失败/嵌套初始化移除、control-link字体销毁、modern frame/actions/菜单内标题销毁、Win3x 30场景取消矩阵及normal/hover/native-string、Win2000 live/switch/keyboard/hover/custom。共36项去重调用；现代fixture显式--private-worker直接进入当前桌面worker，历史打印的private-desktop字符串不代表实际创建/切换桌面。

八文件增量开发包与各自build摘要、PE/导出/资源18表/版本及完整导入门禁通过；ntwshk仍无GUI静态导入，当前候选仅7条新popup符号。来源940回执及负例、原资源五头/声明与6负例、公共源码边界通过。不用开发包代替最终同快照干净交付，正式assets/O:/winnt仍S7。

第二角色按实际diff复核：owner公共对象持有items/loop/depth/system/停止状态；风格只提供真实begin呈现、system绑定、measure/draw及选中绘制，原生native policy不做owner-draw转换。三条owner使用同一message实现，Win3x旧menu_message源/头和frame_state菜单函数已删除，Win2000旧scope四条适配已删除。modern不再分配style_window，字体仍由host自己的font_pair拥有。跨窗口bar/tracking仍是第三批，未以此收口S8。

本批双宽owner/popup归档nm反向依赖门禁通过。六个已终止scratch经绝对路径/reparse核验并清理5,568,821字节，PE检查生成的pycache亦移除，正式包和.ninja/.ccache保留；原始结果已入development证据。治理形状检查通过，负例与diff检查收尾；下一批只推进既定tracking/bar输入与快照，不重跑无关完整包，不以本批结束关闭S8。

## tracking第三批实施中

application/menu/tracking.c拥有唯一线程栈和两种既有消息源的hook生命周期，兼容追加四个style_menu_tracking_{begin,end,current,update} named入口；旧ordinal不动。共享runtime仍仅延迟解析GUI，实际input执行在GUI调用方的公共tracking_input.c，不让ntwshk增加GUI静态导入。navigation表达columns方向与enabled native popup两种已有语义；几何hit由调用方提供完整操作，没有私有字段指针视图。

Win3x产品tracking_menu/menu_filter/current/begin/end和Win2000 active_menu/menu_filter已删除，普通bar和system loop直接接公共栈，MENUSELECT与INIT/UNINIT保持各自原消息时序。Win3x menu_input字段指针view及执行函数删除，公共input拥有原生箭头、鼠标armed、escape和双击协议。bar选择/焦点、快照和iterator仍在本第三批继续，不能提前称S8完成。开发scratch登记build/t26-package/s8-tracking-first，双宽all增量和既有菜单聚焦回归，600秒Worker预算，结束提炼清理。

tracking首轮all构建发现control_link还在调用已删除的私有input_view；已迁同样的鼠标/escape断言到公共state/input，未减断言。新增独立static/shared tracking测试，观察真实PeekMessage的PM_REMOVE、单次nested派发、混合消息源/非栈顶、跨线程拒绝、回调中结束及自有native popup输入。第二轮scratch登记build/t26-package/s8-tracking-contract，先公共契约并all编译，保留失败输出。

tracking双宽all与每宽4项（static/shared真实hook、input及菜单重绘）通过。继续同一第三批：bar焦点/keyboard/逻辑选择与实际highlight归公共owner.bar及bar_state.c；Win3x字段指针state view与Win2000三条Menu/Keyboard/Focus私有property删除，两风格共用select/leave，原生enabled项扫描也共用。Win2000物理toggle夹具改为观察真实GUI_INMENUMODE退出，不再依赖被删私有property。renderer view的现有绘制字段仍保留到S13，不将其误改为state字段。

本批增量scratch登记build/t26-package/s8-bar-first，双宽all及tracking/owner/菜单重绘验证；菜单栏快照与跨窗口iterator仍待最后合并，不以partial状态收口。

bar首轮编译发现新state文件未包含公共sentinel声明，已补公共tracking头，并一次迁移剩余control/window link的旧private view/step消费者；原有焦点state断言保留，虚拟paint callback计数由实际native消费者绘制回归承担，不在测试重写产品if-paint。公共owner测试增加真实EDIT焦点归还/不夺回焦点/停止后拒绝选择及step语义。登记build/t26-package/s8-bar-contract继续双宽all增量和公共菜单聚焦测试。

bar双宽all增量与每宽4项聚焦通过；登记build/t26-package/s8-tracking-runtime作同输入开发八文件包，验证普通两风格、modern菜单、Win2000键盘/hover/arrow/nested及原生回退。只用当前桌面的自有实例和有界等待，结果提炼后删除，不作正式发布或物理输入通过声明。

## tracking与bar状态实际验证、剩余控制器

双宽all增量通过；本轮每宽4项公共tracking static/shared、owner/bar焦点及菜单dispatch通过，另40项实际消费者通过，去重48项。每宽含普通DC故障/初始化移除、control-link、modern menu-title teardown、Win3x 30场景取消矩阵、两风格system菜单呈现与Win2000八项menu-input/key/F10/F10-select/hover/arrow/nested/special。消息和CallMsgFilter/native popup仅证明对应原生协议，未声称物理键鼠或第三方宿主。Win2000物理toggle夹具对私有property的观察已改为真实GUI_INMENUMODE，但本批没有运行物理模式。

开发八文件包的build摘要、PE/导出/18资源表/版本和导入门禁通过；946来源回执及负例、公共源码边界通过。新增tracking四named入口，当前候选popup七条，均为本S尚未发布接口；旧发布ABI不变，ntwshk仍无GUI静态导入。

第二角色复核：产品Win3x与Win2000私有tracking TLS/hook/filter确已删除；Win3x input与bar_state字段指针view已删除；Win2000三私有菜单property已删除。输入执行和enabled项扫描只有公共源实现，profile仅提供几何命中和真实navigation差异。renderer view字段仍是既有S13边界，替换私有state时已恢复其正确字段名，没有把view当owner对象。

本第三批尚有两块：bar的文本/项/owner-draw身份与几何快照仍在Win3x menu_bar.c，应以公共snapshot持有并由两风格实际读取消费，年代measure/MDI筛选/布局是profile；menu_iteration.c和Win2000 menu_input/menu_keyboard_input仍有控制协议，应组合公共instance/menu owner/target操作，统一native跟踪迭代、WM_NEXTMENU引用交接及输入分派，保留真实窗口/字体/几何和native command策略。下一步直接实施这两块，不重开有限审计、不扩大到控件或现代宿主；完成后再一次S8完整交付。

本批第二角色边界复核：两宽owner/popup归档nm门禁通过；五个终态scratch核验绝对路径/reparse并清理5,433,013字节，PE检查pycache亦清理，.ninja/.ccache及正式assets不动。原始结果与包摘要已保存development证据，治理形状和diff检查通过。下一步按已冻结职责直接实施snapshot与完整bar控制器，不重开研究或重复无关回归。

## snapshot公共化实施

application/menu/bar_snapshot拥有复制文本/项/几何、owner-draw身份和同步原生测量/绘制的DC隔离；Win3x保留真实System字体度量和MDI呈现筛选，Win2000保留compact资格/原生矩形偏移和Tahoma配色，两者实际消费相同快照。删除Win3x私有快照结构、free/identity/mnemonic和无消费者bare-reader。登记build/t26-package/s8-snapshot-contract作双宽all增量及独立snapshot/owner/tracking/菜单重绘验证；完整跨窗口controller仍待本批继续，未作新全量交付。

公共bar_controller开始接管两风格真实打开/迭代/键盘/助记符/鼠标和WM_NEXTMENU引用交接；公共target只关联已有instance/menu owner及完整snapshot/hit/paint/lookup操作。Win3x menu_iteration.c/h及Win2000私有menu_input/menu_keyboard_input已删除。两种已有navigation分别保留返回命令、native通知、消息源和系统格策略，不新增支持范围。登记build/t26-package/s8-controller-contract作双宽all增量和共同菜单回归，快照首批两宽各5项通过，原输出待提炼。

controller首轮all发现Win2000 remove仍调用已删除的leave_menu_keyboard，改为直接common finish；无新增私有wrapper。第二轮双宽all及每宽6项通过。第二角色发现NEXTMENU目的端snapshot可能同步重入，因此将目的instance retain提前到read之前、失败释放；共同控制器补真实原生popup/NEXTMENU目的read中移除测试。公共input先筛选相关消息，避免每个绘制消息都分配菜单snapshot。登记build/t26-package/s8-controller-final继续必要增量验证，原失败不计通过。

controller最终增量双宽all及每宽6项聚焦通过，包含真实原生popup/NEXTMENU目的端read中移除、先retain再read及失败release断言。下一登记build/t26-package/s8-controller-runtime作同输入八文件开发包，复用既定40项真实消费者回归；结果提炼后清理。未新增发布或物理键鼠声明。

## 菜单完整能力批次复核

第三批最终两宽各6项公共/dispatch及40项真实两风格/现代菜单消费者通过，去重52项，不重复累计先前重跑；原生popup/NEXTMENU目的read中移除证明目的引用先于回调取得，资源恰好退场。来源951回执、原资源五头及声明、双宽各三个menu归档反向依赖门禁和治理13负例通过。冻结52输入已逐项填写最终职责、保留具体原因与公共入口，最终发布验证待补；产品及profile没有第二套tracking/owner/iterator。Win3x保留System字体/原glyph/MDI策略，Win2000保留native矩形偏移/Tahoma/compact资格，不将差异硬编码公共层。

效率复核：本批按snapshot、controller完整能力做增量，x86最终编译8.83秒/x64 10.54秒；没有逐文件发布，首次删旧函数遗漏已用一次有限调用扫描修正。生产净行数与测试增量见development证据，公共接口/测试带来一定增加，不宣称总行数减少。S8合批交付登记build/t26-package/s8-delivery，暂存检查后仅一次同快照双宽干净构建、先发布八文件再测，完成远端复核后才收口S8。

## S8 P1交付与本地复核

唯一干净快照双宽all完成，x86 158.63秒/x64 187.65秒；ccache直接命中813/901、miss均0；1046项src/tests/tools/CMake输入与当前源码逐项一致。原ABI/原资源/来源951项、双宽各21个公共样式归档及4个进程归档、依赖只来自快照/编译器根均通过。八文件已先发布assets根和O:/winnt，再执行259项去重原生回归：257通过、2项管理员ETW skip；另两项Win2000 DWM恢复旧失败仍返回3/restored=0，不计通过。实际assets/O:/winnt的两风格启动亦通过，最终八文件摘要再核一致。见[交付证据](../etc/evidence/t26-s8-delivery.json)。

首次回归runner漏设hidden-file测试要求的WIN16STYLE_CAPTURE_DIR，dialog-link-32返回41；原输出已保留，补齐夹具既有环境后双宽该项和完整259项均通过，没有修改产品源码、包或权限。不是产品修复重编，不重复累计早先通过项。原生通知/TrackPopup/CallMsgFilter不冒充物理键鼠，现代自有fixture不代表全部第三方宿主。

最终复核：私有menu_message及menu_iteration四源/头、两风格TLS/filter、Win2000私有bar properties和Win3x字段指针view均删除。公共对象真正持有scope/observer/node/ref、owner loop/items/depth、tracking栈/消息源与bar状态/快照/controller；两风格既有native/adapted项和columns/native-popup语义通过完整操作配置，未引用年代/产品私有头。保留的palette/font/glyph、System字体及MDI cell资格、Win2000 compact/native矩形偏移属于实际呈现，现代宿主和最终view/注册分别归既定S12/S13。生产+1238/-1005净+233行，测试+467/-51净+416行；没有宣称总代码减少。完成远端复核后才关闭S8并自动S9；M0/T26保持。

所有构建及测试句柄已终止，未发现本S scratch下存活自有进程，绝对路径/祖先及子树reparse核验后清理交付快照、原生测试和发布备份288871223.0字节；原始结构化结果已提炼交付证据，.ninja/.ccache及assets/O:/winnt正式包保留。
