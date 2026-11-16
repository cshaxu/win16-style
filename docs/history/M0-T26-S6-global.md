# M0 T26 S6 全局接入与切换

## 冻结审计、接口与实施次序

进入基线31010d3f（S5代码b08d601d），git干净，上一轮已返回/清理，无编译器/Ninja可见进程。审计包括ntw16s/global全部23个C/H文件、access/global既有UWP组件、EXE global/tray、普通传播和晚期注册、现代目标和application/global_window；原文件摘要在build/t26-s6/baseline.json，本批审计/开发暂存build/t26-s6，交付提炼后清理。不改兄弟仓库，不新建后端。

生产调用链：main提权并进入tray→用户选择→global owner→session对象/双宽桌面hook及32位host→manager（窗口/进程发现）→process-global engine/targets/transaction→attach（共享DLL→载荷DLL→目标worker）→thread/late→S5 runtime→目标GUI通知→application/global_window和实际风格操作。64位Win3.x另组合已有UWP global队列；这是既有资格，不能据此让Win2000接入UWP。停止统一为phase STOPPING→窗口移除通知→撤销自有临时/桌面hook→host signal/manager停止→保留或释放未完成ticket→关闭会话资源。托盘只维护一个desired选择，旧worker停止后再启动新选择，不为六种转换写六条路径。

| 文件/职责 | 最终归属/处置 | 具体策略与状态所有者 |
| --- | --- | --- |
| session.c/h | access/global/session | 公共40字节wire会话、名称/owner存活/active与style查询、映射/句柄关闭及owner身份cache；产品只提供prefix/magic/version/launcher/module名 |
| EXE global.c/h | access/global/owner协调；产品入口/报告 | 公共拥有mapping/events/handler/hook/host/manager全寿命和停止顺序；产品提供风格资格/包检查、DLL/导出/诊断配置 |
| tray_controller.c | 公共selection/control，托盘UI业务仍EXE | worker、desired/current、ready/stop/reconcile生命周期公共拥有；标签/ID/icon/Menu/ExitProcess行为保留，不改UX |
| manager.c/h | access/global/manager | 状态、发现与经典/UWP组合、期限/停止/统计公共拥有；继续唯一process-global engine，不复制队列/registry；目标允许策略与UWP能力由调用方提供 |
| attach.c/h | access/global/attach | load/start/control tickets、phase/错误/资源跨调用保留由公共拥有，复用底层transaction；模块名/导出/消息/ack与image_filter由配置提供 |
| remote.c/h、start.c/h | access/global的加载/worker装配 | 删除空产品转发，通用GUI thread身份/ready事件/worker生命周期归公共，继续底层remote_export/load/worker |
| thread.c/h | access/global/worker接收与必要产品导出适配 | 16字节packet/事件/owner验证/同桌面worker/hook等待公共拥有；late_init与实际hook入口/协议由产品配置，原export/ordinal保留 |
| late.c/h | access/global的lazy接入协调 | 调用既有background，公共拥有任务/module借用/期限/通知，产品给S5 late_init及协议策略 |
| process.c/h | 公共trace装配 + 产品namespace策略 | 复用monitor/recovery/birth；无新ETW机制。prefix与无context的parser接口通过最小策略适配，不在lib-win32反向依赖style |
| policy.c/h | 产品允许名单 + 已公共Win32 policy | Windows共享host排除名单仍产品策略；删除仅转发inspect/inspect_thread的冗余实现，消费者直接用公共policy并传filter |
| hook.c | access/global接收/host寿命 + 产品四个必要ABI入口 | 不复制render；公共持有process-once/reentry/会话映射/停止竞态，产品给完整窗口change操作及root导出配置 |
| window.c/h | application已有global_window + 产品风格选择 | owner/transition/child遍历已公共，Win3.x/Win2000的模块/导出/支持资格留产品并合并配置；通用child动作协调可下沉，不复制Win2000状态 |
| vb_text.c/h | Win3.x专属绘图适配，S10 | 不是通用global引擎；使用既有module-import，字体/文本策略不进入access。S6产品window_change回调组合它，不称其不可抽离 |
| uwp_global.c/h及uwp/target.c/h | 已公共队列 + 产品协议适配 | queue/dispatch池与app/frame/host身份已公共，保留已有运行时分派；Win3.x资格由配置提供，未来现代呈现仍S12 |
| injection/create_process、late_process | 已有S4/S5公共装配 | active/style cookie查询改用公共namespace；API集合仍调用方，不改变普通模式/传播含义 |
| application/global_window.c/h | 已公共，实际两风格共用 | 先移除再应用、GUI线程/owner/重入/原生恢复仍唯一实现；S6不得在接入层再建一套样式状态 |
| 所有直接编译/测试调用方 | 同步迁移公开接口/对应路径 | ABI必要转接与暂时编译桥区分；S6最终须删除多余产品文件/空包装，不能凭第一批代码先关闭 |

配置按真实差异构成：namespace（prefix、wire身份、launcher/module）、模块/导出/控制协议、target允许策略、完整window_change与late_init、UWP能力及诊断；不接收整个私有产品上下文再按字段回调。公共组件拥有会话/manager/ticket/control状态，callbacks只选策略。SESSION style是opaque DWORD，公共层不硬编码Win3.x/Win2000名单；原数字/二进制布局由产品绑定保持。

实施分内部批次，仍是同一个S6：1 namespace/会话与worker切换状态；2 attach/start/thread/late完整接入事务；3 manager/owner/host与产品配置精简；4全部旧路径/来源/测试调用者终审及同快照一次完整交付。内部批次优先聚焦增量，最后统一all；不每文件清洁构建/发布，不把只迁部分当S6完成。

## 验证与退出

公共namespace独立多配置隔离、PID/birth/cookie/phase/style与LastError，控制器六转换/重复请求/停止中改选/失败；真实双宽DLL加载/初始化/传播/GUI附着/绘制与撤销分别观察，manager自有PID过滤夹具及实际包组合。全局生产验证前逐项排除私有桌面路径；允许当前桌面的原生自有实例，但不使用computer-use。现有管理员ETW权限或输入桌面限制须skip并记录，不改安全配置、不以stub冒充真实覆盖。DWM恢复旧失败仍S7/S14，不计通过。资源/异常只能按既有可恢复边界报告，停止不是物理卸载。

## 本轮实施与效率检查

公共session/control已实际拥有会话查询和选择worker状态；产品托盘保留菜单/图标/Exit行为。start事务迁入access/global/start，模块名和worker导出由产品配置提供，产品start.c/h已删除，attach与原生测试直接调用公共入口。16字节packet与40字节session保持，复用process-global worker/remote_export，不增加卸载能力。产品session临时转发仍待后续消费者统一清理，不能据此收口。

双宽增量编译通过，新公共控制器测试覆盖opaque IDs 41/99/500、六转换、停止中改选、无worker重叠、失败上报、超时保留和join。此前8项会话/托盘/真实风格桥接加2项控制器通过；4项线程回调/停止与owner退出通过。另6项实际DLL start/start-remote/attach失败，ready或begin返回5；已观测加载/导出成功不代表初始化成功，未定位原因，不算通过，不调整生产权限或目标策略。线程测试未启用产品初始化，不能代替DLL测试。逐项命令和输出见[开发证据](../etc/evidence/t26-s6-global-development.json)。

原生测试入口已去除这组测试对私有桌面名称的要求和相应CMake --private参数；仍只创建自有目标，已有同名产品会话被拒绝，不创建或切换桌面。所有本轮编译和测试会话均已返回，测试输出显示子进程正常退出；build/t26-s6只暂留审计摘要、日志与未完成诊断所需结果，增量夹具留在.ninja供后续完整S6验证，不产生完整快照或发布备份。

本轮按会话/控制器与启动事务合批增量，没有逐文件干净构建或发布。下一批先区分DLL拒绝发生在owner身份、晚期初始化或目标核验，再继续既定attach/thread/late与manager/owner/host抽离；完整S6源码和测试合并后才统一完整交付，不能只交付前三个公共文件结束S6。

## 目标端线程与lazy协调抽离

本轮属于进展：worker将原目标线程packet核验、desktop关联、会话/owner验证、模块PIN及hook等待/资源退场迁入access/global/worker；产品保留必要ABI入口并提供完整初始化回调。late将已有background任务、cookie完成状态、窗口通知及phase重查迁入access/global/late，产品只提供初始化和控制协议，不复制background机制。普通启动API集合仍使用S5公共运行时，两处路径/初始化操作已合并，未增加物理卸载。

两宽worker/late及DLL增量编译通过；公共lazy独立原生测试不启用任何风格或函数挂钩，覆盖inactive、失败可重试、busy、完成去重、模块归属、窗口通知与worker退场。迁移后线程回调/停止与owner退出四项重新通过，去重开发结果16通过/6失败。当前x64实际DLL仍返回ready错误5；目标侧会话/owner存活/owner模块路径核验都通过。同夹具换成已发布S5 DLL，两宽也返回5；只能认定同条件旧DLL亦失败，不能据此记为接入通过或随意扩大权限。诊断与baseline结果已提炼进开发证据。

所有本轮构建和原生测试已终结。已核验绝对路径/归属及reparse后删除只用于对照的build/t26-s6/baseline-fixture，共3,902,486字节；小型结果/日志和审计摘要暂留供未完成S6，.ninja/.ccache供后续增量。未创建完整构建副本或发布备份。尚需迁移attach/manager/owner/host与接收链，并最终删除产品过渡转发/整理配置；不能将本轮公共worker/late当作S6完整交付。

效率复核：同一拒绝的重复尝试已改为owner分段诊断和S5 DLL对照，获得新证据后继续迁移，避免无证据重复UAC/整包重建。下一批补齐底层已有resolver/identity回调的上下文传递，使attach可使用配置且不引入TLS绑定或多份实现；保留已有process-global公开入口兼容性，优先单一核心实现。

## attach与manager公共状态所有权

本轮属于进展：公共attach实际拥有目标身份、process/load/start ticket、path/cookie/phase/control_error及失败保留/释放；模块/导出/控制消息/ack与target filter由产品配置提供。公共manager实际拥有session map、发现、engine目标调度、统计、期限与停止回收；仍调用唯一process-global engine/transaction，不新增队列或hook后端。删除产品attach.c/h、manager.c/h；只被测试使用的remote三个空转发从产品删除，测试直接装配原公共加载/解析入口，兼容入口仍被测试使用。连同此前start，目前删除八个产品C/H文件，global产品目录由23变17个C/H（加入config/services真实策略装配），其余仍须按冻结清单收口，不只追求文件数。

process-global的load_finish、mapped_identity、select_path增加同步借用context的using入口；原公开签名由小型适配调用同一核心，不复制算法、不使用TLS或全局回调绑定。公共attach通过这些入口传配置。双宽原生module/finish四项通过：旧选择接口、nested context、映射代码核验、resolver错误上抛、timeout保留和成功/失败资源清理。双宽产品DLL/x64启动器及主要全局消费者增量编译通过；公共style-global双宽归档与完整access/application源码依赖门禁通过，无产品/风格反向依赖。

管理器本地未完成loader保留的两项通过；全局target/unsupported/停止竞态六项仍失败，真实接入六项重新执行仍拒绝。新增关键证据：manager target和unsupported双宽均观察TokenIsAppContainer=1，现有经典进程策略明确拒绝该目标；这不是允许修改安全/准入策略的理由，不将未施样式的像素变化冒充绘制成功。worker ready及停止竞态初始拒绝仍未完全定位。当前去重22通过/12失败，原始退出码与环境限制保留在开发证据；不是完整产品矩阵通过。

为在当前桌面安全运行，manager夹具在扫描前限于自建目标PID，新建第二实例加入测试白名单；停止竞态启动时先排除全部外部目标，创建自有child后才准入该PID。仅测试编译启用中性STYLE_GLOBAL_MANAGER_TEST，生产无此过滤或范围改动。未使用私有桌面/computer-use；四个已记录manager目标PID均复查不存在，所有构建和测试会话返回。scratch只保留后续S6所需小型结果/日志与审计摘要，没有整包克隆或新发布备份。

效率复核：按attach+manager完整状态迁移合批，不逐文件完整交付。当前剩余重点为owner/host及接收链、trace/namespace配置精简、删除产品过渡转发，随后一次同快照交付。源码来源回执及全部调用方将在S6代码合并后统一终审，当前未宣称完整来源门禁或发布通过。

## owner、host及接收链

本轮属于进展：公共receiver拥有process-once资格、TLS重入抑制、会话/owner/module核验、GUI线程接收、PIN与停止竞态回滚；产品hook只提供完整风格change操作及协议绑定，原VB文本及root/child应用顺序不变。公共host拥有会话/owner/ready/stop句柄、desktop hook、manager及等待/关闭顺序，产品仍通过原Host导出处理返回错误/ExitProcess。公共owner拥有mapping/events/security/module、双宽rundll32 host、sweep/发现、消息泵及停止顺序；EXE只保留完整包资格/路径、产品风格选择、管理员诊断及最终报告。两个风格继续同一组合，不生成六条独立切换实现。

host/owner在manager停止失败时通过retained转交对象，公共契约要求同线程重试且代码/config继续存活；原产品错误出口保持，不据此声称所有异常均可恢复或DLL可物理卸载。owner只支持进程内一个活动实例（原console handler/stop信号契约），父句柄借用，不新增并行owner能力。删除产品name/owner_alive/owner_matches/session_matches四个空转发与policy inspect两个空转发，消费者直接调用公共入口；active/style两个适配保留为外观与启动资格的产品namespace回调绑定。

新接收器/host/owner六项双宽原生测试通过：receiver自有父子窗口直接调用真实公共接收器，验证owner/cookie、once、reentry、停止中取消、remove和LastError，不启用风格或真实全桌面hook；host仅验证缺失/错误会话及50次失败清理；owner仅验证父方已停止、opaque style42、包预检失败、20次资源退场和对象碰撞不修改已有mapping，在安装hook前停止。不能用这些结果代替真实双宽host接入。产品unsupported-style及sweep超时两项通过，session/tray四项迁移后重新通过。当前去重30通过/12失败；既有AppContainer限制和未定位失败未消失，原始失败保留。

双宽DLL/x64正式入口及产品失败夹具增量编译通过；所有access/application源码及双宽style-global归档依赖门禁通过。旧测试源码内联产品global.c来拦截等待/窗口遍历，已改为同时对实际公共owner.c做同样故障注入，不添加生产测试开关。所有本轮原生测试/构建会话已返回，未创建完整包副本或发布备份；scratch只保留后续S6所需小型证据，缓存保留。

效率复核：接收器+host+owner按同一会话寿命合批抽取，聚焦增量完成，没有为单文件完整交付。下一批完成trace namespace/monitor装配和产品配置终整，统一来源回执与全部消费者后再进行一次同快照干净构建、先发布再测及远端复核。S6与T26仍开放，不将当前开发证明改写为全局产品通过。

## trace与产品装配终整、执行环境对照

trace规范名称/PID与birth校验、monitor启动及orphan协调迁入access/global/trace；底层recovery只增加同步借用context的using入口，原签名仍走同一实现，不使用TLS绑定。manager/owner/host直接调用公共trace，产品仅保留Win16S-ETW-前缀。删除产品process.c/h；线程、late初始化与三个必要导出合并至hook.c，删除late/thread两组C/H。产品global最终暂为11个C/H：session命名/协议与callback绑定，config目标模块/控制协议，services现代资格/诊断，hook ABI/风格动作，policy名单，window/vb_text风格策略。没有再保留通用生命周期副本。

关键环境纠正：exec默认沙箱会使自建child成为AppContainer，既有classic策略按设计拒绝。相同二进制/相同自有PID过滤夹具在沙箱外原生执行后，x86/x64各6项全部通过：real-DLL start、remote-load、root/child apply-remove、existing/new manager、stop-race、unsupported-later。未改权限、目标范围或系统设置。原12项沙箱失败另存sandbox_case_history，不从历史中删除；当前适用原生矩阵转为通过。manager styled绘制变化可观察，但restored_equal=0仍不得称完整像素/DWM恢复；该旧限制归S7/S14。

去重开发46项通过，2项orphan ETW集成返回77（需要管理员权限），0项当前失败。trace parser/事件格式及直接worker退场双宽通过；管理员ETW未跑不算通过。双宽全目标增量编译完成，来源门禁927文件、原资源、公共源码/归档依赖门禁通过。构建和原生夹具均终结；scratch只留小型证据，不存在旧完整S6包。准备唯一完整交付build/t26-package/s6-global，完成测试提炼后清理，缓存保留。

## 按冻结清单复核（交付前）

| 能力/旧生产位置 | 唯一公共入口/状态所有者 | 产品保留与真实消费者 | 当前验证及边界 |
| --- | --- | --- | --- |
| session身份/名称/查询/owner cache | access/global/session：session_name、owner_alive、session_matches、policy_active/style、owner_matches | session.c只绑定prefix/magic/version/module与active/style回调；EXE/tray、自动传播和全部fixture直接消费 | 双宽独立namespace/opaque IDs、40字节wire、LastError/句柄；两风格session回归 |
| 托盘选择与worker停止再启动 | access/global/control：init/select/update/finished/close | tray_controller只保留菜单/icon/Exit、UI泵和product run回调 | 六转换、rapid choice、无重叠、失败、超时保留；原菜单回归 |
| 目标GUI worker启动与模块身份 | access/global/start；lib-win32 process-global/module/load/worker | config.c提供模块/导出；产品start/remote已删除，attach及实际DLL夹具直调 | 双宽真实DLL/remote-load、PID/TID/cookie/collision/ready、packet释放/停止 |
| 接入事务/失败状态/资源 | access/global/attach：begin/advance/apply/remove/close；底层唯一transaction | manager直接调用，config仅module/control/filter | 双宽真实root/child、click/remove/cleanup；bounded control timeout；底层context/重入/失败释放 |
| 目标线程接收/晚期初始化 | access/global/worker、late；runtime/process仍唯一晚期批次 | hook.c保留原GlobalThread ABI，提供完整API初始化操作；旧late/thread文件删除 | 双宽direct worker、owner退出、独立lazy失败重试/完成/通知；实际DLL初始化原生通过 |
| 目标发现/状态与manager | access/global/manager，复用lib-win32 engine/targets/enumerate | services提供UWP能力/报告，config提供目标策略；产品manager删除 | 双宽既有/新建/退出/替换/stop-race/unsupported，local-loader abandon；不把style恢复当卸载 |
| GUI hook接收与停止竞态 | access/global/receiver：receiver_handle | hook ABI给完整window change/资格；Win3x VB文字/两风格动作保留产品策略 | 自有父子direct消息验证及真实DLL接入；synthetic CWPSTRUCT本身不等于全桌面hook |
| host/session/双宽owner协调 | access/global/host/owner：host_run、owner_run | Host导出只保留原退出行为；EXE global只提供资格/包路径/产品诊断 | 双宽独立失败清理、collision和parent cancellation；x64入口编译；真实全桌面托盘/管理员host仍需正式包适用验证 |
| trace命名与monitor/recovery | access/global/trace；lib-win32 process-global/monitor/recovery | services仅提供前缀；旧process文件删除 | 两namespace/canonical格式/事件parser；orphan管理员项skip，不算通过 |
| 全局窗口/root-child与恢复 | application/lifecycle/global_window的transition/visit_children；appearance各风格绘制 | window.c按真实支持资格选择Win3x/Win2000及提供child操作，vb_text归S10；没有私有遍历/附着引擎 | 两风格bridge/children已有聚焦回归；非客户区恢复归S7，窗口API创建链归S11，现代呈现归S12 |

以上是实现/开发审计，非S6交付收口。所有旧产品机制文件/入口搜索与源码、link门禁已逐项复核；source receipts保留已有notice/disposition并登记迁移，原图形资源未改。完整包及发布后矩阵、实际远端仍未完成，不能提前关闭S6或T26。

## 同快照交付与单人第二角色复核

双宽干净构建通过：x86 144.97秒，793 direct hits/0 preprocessed hits/2 misses；x64 171.64秒，877 direct/2 preprocessed hits/4 misses。1024个生产/测试/构建输入与工作区逐摘要一致。来源927文件、原资源逐字节及ABI/ordinal/PE导入门禁通过，双宽各18个公共样式归档及4个process归档无反向产品/风格依赖。产物仍为五组件八文件，无C++、兄弟仓库或额外产品运行依赖。

先将同批八文件发布到assets根与O:/winnt并逐摘要核对，再在沙箱外运行相同摘要包的162项原生聚焦回归：160通过、2项管理员orphan ETW返回77跳过；另2项Win2000 DWM恢复旧失败与S5相同，不计通过。真实DLL start/remote-load/attach、已有/新建/替换/退出目标、停止竞态、迟到/既有unsupported窗口、六种选择/窗口风格转换及ordinary启动/晚期四位宽传播均有适用测试；直接构造消息/独立host失败测试的边界仍如前述。不能据此声称所有第三方UWP/WinUI/真实托盘全局场景或完整原生像素恢复均通过。完整命令/输出、八文件与工具链摘要见[交付证据](../etc/evidence/t26-s6-delivery.json)。

第二角色重新读取原三层目标、冻结清单、实际diff与原生结果：product保留的是包/风格/协议资格、完整风格动作与ABI导出，没有私有manager/attach/remote/start/thread/late/process机制副本；新增公共配置是完整操作回调，未接收产品私有字段视图。旧回调签名兼容入口均复用单一核心；scope、权限和ordinary传播未扩展。保留的active/style namespace适配用于产品资格回调，Win3x VB绘图和各风格window资格仍由S7/S10接续，API创建链S11、现代呈现S12不提前宣称完成。

本地交付验证已满足，仍待清理本轮快照/测试副本/备份、实际提交推送与远端逐blob复核后收口；本段不把工作区状态等同远端完成。

原生夹具/构建会话已终结，并在沙箱外只读确认没有从S6快照/scratch执行的存活进程。核验绝对归属和reparse后清理scratch、同快照构建及双目的地备份274,160,225字节，缓存保留，见[清理证据](../etc/evidence/t26-s6-cleanup.json)。正式assets/O:/winnt未删除；提交前重新核对八文件摘要与测试包证据一致。

## 最终源码快照与发布复核

提交前暂存区检查发现两个新文件的EOF多余空行，仅规范尾部LF并更新来源回执。因PE时间戳导致重建八文件摘要不同，未复用首轮包的测试结论：最终快照重新完成双宽干净构建、先发布再跑全部162项。最终x86 158.12秒，790 direct/2 preprocessed/3 misses；x64 176.42秒，874 direct/6 preprocessed/3 misses。1024个生产、测试、工具与构建输入摘要一致，来源、ABI和依赖门禁通过。

最终发布包160通过、2项管理员ETW跳过；另2项DWM旧失败仍未通过，归S7/S14。完整命令、输出、八文件摘要和初次交付记录同时保留于交付证据，不能混用两批二进制。无存活自有夹具后核验绝对路径及reparse，额外清理最终测试、快照及发布备份274,215,807字节；两轮合计548,376,032字节，保留增量与ccache。产品与源码均待真实远端复核才收口。

效率复核：S6按全局会话完整能力合并实现并统一交付，未逐文件打包。最终格式问题造成一次额外完整交付；后续须在干净快照前先做暂存区差异检查，避免因未跟踪文件漏检造成重复构建。独立失败契约、合成消息、真实DLL目标与未验证的第三方场景已分别标明，不用自测通过替代未覆盖项。

S6 P1 `4d1cdcb70ef1c7917965f4eb6437c39ba0372f00`已推送并实际fetch，95个现存blob逐SHA-256与工作区一致、14删除路径远端不存在，八文件与O:/winnt及测试摘要一致，见[远端记录](../etc/evidence/t26-s6-remote.json)。按既定范围收口S6并自动准入S7。ETW跳过、DWM旧失败和第三方宿主未覆盖不改为通过，T26保持开放。
