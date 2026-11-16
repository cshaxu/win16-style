# T26 S1 产品接入层局部审计

这只是已阅读条目的局部审计，不代表全目录已完成迁移或验证。

| 当前条目 | 已共享与仍待处理 | 调用与测试线索 |
| --- | --- | --- |
| `ntw16s-dll/global/attach.c` | 已调用 `process-global` 的策略复核、模块定位、加载、事务推进和 worker 生命周期；两阶段共享 DLL/产品 DLL 加载的协调仍混在本文件。产品 DLL 名、导出名、cookie 和窗口控制消息必须由调用方提供，不能把整个文件直接搬入公共库。 | `global/manager.c`；`tests/ntw16s-dll/global_attach.c`、`tests/integration/global_headless_existing.c` |
| `ntw16s-dll/global/manager.c` | 目标队列和事务运行已调用 `process-global` 引擎；窗口通知、会话规则、UWP 协调和风格限定仍交织在管理器中。需逐项检查能否通过现有回调合并生命周期，不应另造一套引擎。 | `global/hook.c`；`tests/ntw16s-dll/global_manager.c` |
| `ntw16s-dll/global/remote.c`、`start.c` | 远程加载、导出定位、worker 启动已调用公共机制；DLL/导出名、线程数据包和就绪事件名称属于当前产品契约。仍需审查是否能消除薄包装并减少构建侧重复编译。 | `global/attach.c`；`core-global-attach` 构建入口 |
| `ntw16s-dll/global/process.c` | ETW 监控及孤儿恢复已调用公共机制；`Win16S-ETW` 名称格式是产品命名约定。保留约定的同时可审查是否缩减适配层。 | `global/manager.c`；对应全局管理器测试 |
| `ntw16s-dll/global/policy.c` | `ntw16s_global_image_allowed` 是当前产品对系统共享宿主的排除策略；两项 `ntw16s_global_policy_inspect*` 向 `process-global/policy` 传入该策略，身份和位宽检查不在此复制。不能把名单移入通用库；两层薄适配是否可以随产品边界合并仍待检查。 | `global/attach.c`、`global/manager.c`；`tests/ntw16s-dll/global_policy.c` |
| `ntw16s-dll/global/session.c` | 映射名称、magic/version、cookie、阶段和风格编号属于产品会话协议；进程出生时间使用 `process-global/identity`。`ntw16s_global_owner_matches` 同时含产品文件名、同目录约定和通用文件身份比较，后者已调用 `process-global/module`，调用链仍需独立测试和简化。 | `global/hook.c`、`global/thread.c`、`global/manager.c`；`tests/ntw16s-dll/global_session_style.c` |
| `ntw16s-dll/global/thread.c` | 数据包格式、会话命名、owner 校验和晚期风格初始化属于当前产品；钩子等待与释放已调用 `win32_process_global_window_hook_wait`。线程和事件等资源的打开及错误清理仍与协议交织，不能因共用等待函数就认定完整抽离。 | `global/start.c`；`tests/ntw16s-dll/global_thread.c`、`global_start.c` |
| `ntw16s-dll/global/hook.c` | 进程排除策略、控制消息、会话状态和具体风格附着保留在产品；可见窗口及子窗口分派与策略交织。它调用 `global/window.c`、`global/late.c` 和公共身份能力；不应将含 Win16 文本处理的整个钩子搬入通用库。 | `global/thread.c`、`global/manager.c`；`tests/integration/global_propagation_probe.c` |
| `ntw16s-dll/global/late.c` | 背景任务排队、去重和释放调用 `process-global/background`；目标模块路径、会话 cookie、初始化及样式控制消息属于当前产品适配。仍需审计向窗口广播的通用遍历部分，不能只改文件位置。 | `global/hook.c`、`global/thread.c`；公共背景任务测试及产品传播探针，单文件直接测试未发现 |
| `ntw16s-dll/global/vb_text.c` | 实现仅在特定 VB 运行时文本调用中借用 Win16 字体的产品策略；导入表查询及修改调用 `lib-win32/module-import`。风格字体、目标模块和四个绘制替换函数不能放入通用样式进程库。 | `global/hook.c`；`tests/ntw16s-dll/global_vb_text.c` |
| `ntw16s-dll/global/window.c` | 窗口状态和切换已调用 `style_global_window_*`；风格表、Win2000 伴随模块导出和全局消息仍由产品决定。未来新增风格时应审查选择接口，不能把具体 DLL 名放入公共代码。 | `lib-style/common/global_window`、`lib-win32/style/companion` |
| `ntw16s-dll/attachment.c` 的 `Ntw16sWindowThreadHook` | 消息匹配及结果属性回传现调用 `lib-win32/style/window_hook`；动作筛选、重入状态、线程归属校验、Win16 附着/移除、模块生命周期和调用下一钩子保留在产品。 | `tests/ntw16s-dll/window_thread.c` 的直接回调分支；真实线程钩子对照当前双位宽均返回错误 126，不能声称已通过。 |
| `ntw16s-dll/uwp/hook.c` 及 `lib-style/uwp/hook.c` | 产品提供 UWP 动作和 Win16 外框操作；公共 UWP 适配负责框架/核心窗口身份检查，再调用同一 `lib-win32/style/window_hook` 执行消息机制。 | `tests/lib-style/uwp/hook.c` 的直接回调与 `dispatch.c` 的真实钩子测试；后者仍返回错误 126。 |
| `ntw16s-dll/uwp/target.c` | 窗口调度调用 `style_uwp_dispatch_change`；目标动作、宿主位宽、导出及消息仍是产品适配。公共调度不等于已验证 Win2000 或其他风格接入。 | `global/manager.c`、`run16s-exe`；`core-uwp-target`、`launch-uwp-live` |
| `ntw16s-dll/injection/late_process.c` | 批量安装和传播使用 `process-hook`；会话采纳、初始化策略以及产品 hook 列表仍在产品层。其可共用协调边界和并发语义需进一步检查，不能把停用等同物理卸载。 | `global/thread.c`、`global/late.c`；`tests/integration/late_process_prototype.c` |

`run16s` 的构建入口当前直接编译若干 `ntw16s-dll/global` 和
`ntw16s-dll/uwp/target.c` 文件，说明依赖方向和适配职责仍需整理；是否
能够完全消除，需在逐项审计和保留实际行为后确定。

## 子窗口遍历

`style_global_window_visit_children` 现负责枚举子孙窗口并筛选当前进程的
当前 GUI 线程，接受调用方回调和上下文；回调决定是否继续。原产品侧
`ntw16s_global_window_children` 实际调用它，仍决定目标是否适用及采用
Win16 或 Win2000 操作。公共机制不依赖产品头或具体风格。

双位宽增量构建及公共测试通过，覆盖多级子窗口、提前停止和空回调；
产品组合测试覆盖已有与新建子窗口的 Win16/Win2000 互切与移除。
线程身份的独立正负例在 `tests/lib-win32/style/window_identity.c`。补充的
双位宽原生测试在同一父窗口下创建另一 GUI 线程的子窗口，确认它被跳过，
并等待该线程和窗口退出；真实全局会话仍未测试。本改动尚未干净构建或发布。

## 普通启动事务

`win32_style_startup` 统一启动期事务的开始、准备失败回滚及提交；Win16
与 Win2000 的实际 DLL 入口分别提供原有安装顺序和策略。它调用已有
`process-hook/startup_hooks`，不包含风格名字、目标选择或绘制代码，也没有
增加晚期接入或物理卸载能力。开始失败不调用准备或回滚；准备失败回滚；
提交失败沿用原有返回契约。

双位宽增量构建及纯 C 事务测试通过。自建父子目标覆盖 Win16 与 Win2000
各四种父子位宽组合；Win16 检查父子窗口附着与退出，Win2000 检查父子
外框和控件附着、移除及主题状态。测试副本已清理。本改动尚未干净构建或
发布；真实全局会话与先前列出的失败项不在本次验证范围。

## 样式操作表去除转发层

`ntw16s-dll/global/window.c` 在同一 `InitOnceExecuteOnce` 初始化中，直接将解析到的 Win2000 附着、应用、移除函数填入公共窗口操作表，删除三个只转发调用的产品包装。样式选择、按需加载、窗口切换和回滚仍由现有产品适配及 `style_global_window_transition` 负责；未改变挂钩、传播或窗口选择规则。

双位宽增量编译通过，公共窗口事务与自建窗口双风格切换测试通过，覆盖两种方向、移除、既有窗口接管及新子窗口。测试输出 `refreshed=0`，该字段不是其通过条件，不能据此声称该刷新路径通过；真实全局会话亦未验证。测试未创建或切换隔离桌面。本次尚未干净构建或发布。

复核发现 `WM_NCPAINT` 成功处理原生区域后提前返回，漏掉了公共 `style_invalidate_native_menu_overlap`。现已在该返回前调用同一刷新机制；原有失败回退路径仍在末尾处理。测试加入 `WM_PAINT` 计数，并在存在菜单重叠时要求已同步重绘或存在更新区域。双位宽自建切换回归从 `update=0, repainted=0` 变为 `update=1`，同时原有切换、客户区像素和子窗口检查通过；它不证明真实全局会话。改动为未发布增量结果。

发布状态更新：上述适配简化和重叠刷新修复已纳入[第十五批候选](M0-T26-S1-shared-candidate-15.md)，发布后验证范围以该记录为准。

进一步在自有进程内使用真实启动钩子实现：准备阶段建立原生跳板后主动
返回错误，公共事务回滚并恢复原入口；随后提交并验证跳板与目标调用。
x86/x64 均通过，既有 88 槽及第 89 个拒绝的边界回归同时通过。此测试
不代表可移除已提交钩子，也不覆盖真实全局会话。

## VB 绘制作用域的产品接线

删除 `ntw16s-dll/vb_text.c` 中只转发的作用域入口；产品附着层直接调用已有 Win16 控件实现，并将 `ntw16s_vb_user_label_target` 作为策略传入。控件通知处理直接使用同一实现及调用方策略，不再通过额外的产品函数指针；独立测试也直接调用实际实现。字体、绘制作用域与目标资格仍属于 Win16 路径，本次不将其冒充跨年代共享能力。

双位宽增量构建通过；直接 worker 运行的 VB 字体租约和全局文本模型均通过，自建启动器加载增量 DLL 的 VB 控件目标在两位宽通过字体、窗口绘制、附着与清理检查。测试没有创建或切换桌面，不证明真实全局会话或其他主题的控件支持；本改动尚未发布。

## 新三层架构下的全局、UWP与启动器复核（P107）

本批逐文件读取global的23文件、uwp的4文件、run16s-exe的19个C/H文件和ntwshk的9个C/H/def文件，复核CMake真实产品/测试消费者。源身份见下表；审计不是已完成迁移或运行验证。

| 来源 | 应由公共组件承担的完整操作/状态 | 产品或风格保留 | 对应验证 |
| --- | --- | --- | --- |
| global/manager.c/h | access/global的启动/停止/排空和传统/UWP组合；复用process-global engine/targets/transaction/window，不复制队列 | 允许目标、session选择、风格能力及最终诊断报告由调用方提供 | global-manager/unsupported/stop-race/abandon；停止失败保留对象后重试，不能提前释放借用context |
| global/attach.c/h | access/runtime的进程句柄/身份/path/load/worker整体票据；shared再style两阶段加载与advance/apply/close | DLL/导出/消息及cookie通过配置提供；位宽/权限目标不扩大 | global-attach/start/load/remote；ERROR_NOT_READY与remote线程未完必须保留票据，不误作物理卸载 |
| global/start.c/h、remote.c/h | access/runtime的目标UI线程复核、ready事件和worker生命周期；远端导出/加载复用process-global | packet内容、导出及模块名是产品兼容配置 | global-start/thread/remote；ready安全属性沿用，existing event拒绝保持 |
| global/thread.c/h | access/runtime的worker开始/等待/退出、映射/event/thread资源及身份复核；调用既有late init和window_hook_wait | 当前packet版本、名称、cookie/owner匹配与模块pin策略明确配置 | global-thread/owner-loss；同位宽，模块pin持续至目标退出，不称卸载 |
| global/hook.c | access/global的host启动与消息循环、观察控制、目标本地验证及操作分派 | 目标排除、session协议配置、style操作及Win3.x VB文本策略不混入通用process-global | global-propagation、headless-existing/late-dialog；保留LastError和CallNextHookEx链 |
| global/late.c/h | access/runtime的后台就绪协调和本进程窗口通知；复用background去重/清理 | session active、路径/消息/初始化集合作参数 | 既有background/late-process与传播目标；不新增工作线程管理器 |
| global/session.c/h | access/global的映射打开/关闭、owner存活与session读取生命周期；复用identity/module | 40字节兼容包、magic/version/名称、风格编号与当前owner路径约定可作为产品协议配置；不因此把整套映射生命周期留产品 | global-session-style/identity/policy；跨组件值契约与PID出生时间保持 |
| global/policy.c/h、process.c/h | process-global已拥有policy/ETW/recovery；样式层只组合现有机制，薄转发合并 | 共享宿主排除表与ETW名称约定由产品配置 | global-policy/discover/process事件及权限skip；权限不足沿用window discovery，不改系统策略 |
| global/window.c/h | application/lifecycle的style transition和window的同线程子孙应用；已有global_window唯一复用 | Win16/Win2000操作表和DLL导出由风格注册装配；目标支持由策略 | global-window/style-bridge六转换和子窗口；已有普通附着所有权契约保留 |
| global/vb_text.c/h | application/control运行时adapter的import观察/临时字体scope，底层module-import不复制 | VB6支持和Win3.x字体呈现为风格能力；不随global整体迁入接入层 | global-vb-text/vb-font；当前四ANSI入口范围不扩大 |
| uwp/hook.c/h、target.c/h | adapters的title/core识别与actions能力、access/runtime的thread dispatch和pin；复用uwp/hook/dispatch/native/window_hook | 产品消息/export兼容名，风格apply/remove/action渲染由装配给出 | uwp-hook/dispatch/target/frame；当前明确x64，非所有UWP/WinUI支持 |
| run16s global.c/h | access/global的session资源、双位宽host、ready/stop/sweep与消息循环整体迁移；复用manager | CLI触发、提权时机、错误输出和风格选择为产品；不把EXE文件名写入公共引擎 | global startup/headless/transition和托盘组合；timeout是部分完成，ExitProcess不清理的UX保持 |
| run16s tray_controller.c/tray.h | 通用icon/menu消息已经归lib-win32/tray；无需再造第二库 | 菜单文本、checked目标、reconcile选择、点击Exit直接退出留产品 | core-tray-controller菜单一直可点、快速选择、worker完成后重启；icon资源不变 |
| run16s window_target.c/h | access/runtime的模块兼容核验、thread hook、请求/结果property事务；复用window_dispatch/companion/module现有基础 | 具体模块/export/message/action由产品装配 | window-target-errors及现代窗口目标；旧映射偏移/映像匹配范围保持 |
| run16s explorer.c/h | adapters/win32的启动前snapshot、新窗口差集与歧义拒绝，应用类别数据驱动 | 只识别无参数系统Explorer或既有Terminal范围是产品策略 | explorer-selection；256窗口上限、200ms稳定期和5秒截止保持，不附着旧桌面/任务栏 |
| run16s console_host.c/h | adapters/win32的既有console/terminal宿主定位与定向接入，复用相同dispatch | NtQueryInformationProcess内部信息类别和系统路径匹配是现有受限adapter，缺失时不接入；不升级为任意宿主扫描 | console-host/errors；目标已退出、父子/出生/窗口复核，128thread上限保持 |
| run16s uwp.c/h | adapters/uwp的激活manager、created_pid/复用实例、等待frame和bounded消息泵，access复用同一target-change | calc别名及包/AUMID映射留产品表，不装作所有app通用解析；当前只Win3.x | launch-uwp-live/alias及公共target；真实环境限制如实skip，比较文件可复用已有文件身份能力 |
| run16s main/elevation/startup_error及头 | 普通启动prepare/resume可复用进程能力；参数解析/目标模式分派保持集中 | CLI、--2k支持边界、UAC原始尾参数、private-console/输出错误属于产品 | launch-failures/elevation/console和参数矩阵；不改变命令、等待和用户体验 |
| run16s process_lifecycle.c/h | 无风格含义的suspended-child activate/wait应归lib-win32/process-launch，保持创建者唯一回滚所有权 | 产品prepare回调组装ntwshk安装 | core-process-lifecycle的prepare/resume失败及句柄；不是接入层另造进程库 |
| ntwshk context/installer/helper/system_process及头 | 通用stage/payload/helper/传播/位宽机制已在process-hook/image；style-specific上下文序列化/模块路径和安装组合归access/startup | 现有payload版本、module名、helper/export兼容契约及打印宿主排除配置，DllMain被动入口和def留共享DLL装配 | packet/installer/helper、双位宽创建传播，原始8文件产物与ABI不变；当前跨宽helper不改实现 |

### 公共状态及接口冻结方向

1. process-global继续只拥有无风格目标身份/队列/worker/load等基础；access/global拥有上述manager、session组合和stop/sweep。目标状态不复制，两种管理对象通过完整执行回调组合。
2. access/runtime一次接入票据拥有process/load/worker/path，返回错误时标明是否仍持有未完成资源；close成功才转移为已释放，stop失败可由同一owner重试。现有load abandon保留远端路径至目标退出的范围如实记录，不加危险强制回收。
3. 风格注册向application交付已有window/control/dialog操作及能力，access选择注册项调用同一apply/remove机制；生命周期与style policy的active/cookie核验分开。不能将manager已有Win16/Win2000 if分支换个目录后继续硬编码未来风格。
4. product适配只保留兼容导出/协议配置、CLI/托盘业务和支持范围选择，资源操作及协调迁入公共实现。既有不同位宽各自manager与host依旧存在，不能声称一个x64回调已经覆盖所有x86目标。
5. 普通显式启动、目标窗口接入、global三入口共享底层dispatch/attach，但发现目标和恢复边界不同，不能误把普通启动扩成全局枚举；UWP关联到共享宿主时只操作选定frame，保持现有不传播该宿主的限制。

CMake当前将global/session/remote/start/process/policy/attach/manager和uwp/target产品源码再次编入run16s及部分测试，是明确迁移消费者；S2将归位为样式接入target，S4–S6再迁状态/协调，测试也链接实际公共生产target，不能继续以复制产品源或test-only替身代替共用。Win3.x全局VB IAT字体适配不能跟随整个global目录盲搬；应归appearance与control运行时协议的组合。

已有单独startup钩子与global会话退出后是否继续受style policy影响，以当前context flags/cookie及global_window所有权为准，不依据旧注释重写行为。只承诺保持现状，并在组合回归逐项观察传播停用、窗口恢复、线程hook解除；目标pinned DLL及已提交trampoline不做物理卸载。

### 本批源身份

| 文件 | SHA-256 |
| --- | --- |
| `src/ntw16s-dll/global/attach.c` | `982dafe59d85f9f4209cdeb42d01c1c17d9a2aeb4ffeb2d8c9477bdac668d267` |
| `src/ntw16s-dll/global/attach.h` | `9537a395e0a3acb3004e772b3b27d8b243e7a422fab09fcfad7ba28a2f4ccf47` |
| `src/ntw16s-dll/global/hook.c` | `32a63df5f25e2ab0982db44de3e5d3441b5afaa292f08f23be704d5159f7628e` |
| `src/ntw16s-dll/global/late.c` | `5c4c11323c085222fa01e9581d37135322b5f7f63cbb361cfd5023ff13958bc3` |
| `src/ntw16s-dll/global/late.h` | `ebf3f5a6bdf90c291d84c64fa7c1681b0b5e2be54c8b3e67da6f1bc0ba0ea11e` |
| `src/ntw16s-dll/global/manager.c` | `688827034d165c6783efc06e5b1af8935534d5ce5c68788a07f7fd130ef02cf2` |
| `src/ntw16s-dll/global/manager.h` | `5b20d88cb6584ed785dc4618e774c1bf0f03d9a87a2a3a73130afc268d82696d` |
| `src/ntw16s-dll/global/policy.c` | `bf8c3cead7caf4302e55cdd0723a092fbeffe02decb7f9c3e58e9ddf00fff05e` |
| `src/ntw16s-dll/global/policy.h` | `98f4c4f3708def9842b63567f2f87487ea87f6d778eb448545e2dc0235ec43b4` |
| `src/ntw16s-dll/global/process.c` | `482ce1b0256016cf4a4f65b74599a5bb2108662853f724080ee9e55043bf0f38` |
| `src/ntw16s-dll/global/process.h` | `7eaae6de7774a74eec9dfd79f595985df8ece347535d5bcec8d025a44080e836` |
| `src/ntw16s-dll/global/remote.c` | `4fb41690f3c4355c164ab8869d140a599251b8b2f95ad981cedd985e94b7ca00` |
| `src/ntw16s-dll/global/remote.h` | `6ff170c39ca25027c40948704e1bf03e495e1d05fd3f3e9ff719330a9fc9854c` |
| `src/ntw16s-dll/global/session.c` | `c329ecf8b7be79accdc42b5a583a962a478998759f2921bfeff001bb70e9a90c` |
| `src/ntw16s-dll/global/session.h` | `6b6b85c47eb303e182c8013187d4c26eaae5654d476ac1f90daea84b2d8d41b3` |
| `src/ntw16s-dll/global/start.c` | `946089b179217244b2e8c3a8508262e6026129233055d4c6f9012918bba170e2` |
| `src/ntw16s-dll/global/start.h` | `488d2a9f2a23876bcea9f0484a36a00744993b9b5fce9a9b9009886aad66d627` |
| `src/ntw16s-dll/global/thread.c` | `8954e9d0b89f864adfccb12654dd7d187fa05f86844c42df5085ba28d8137238` |
| `src/ntw16s-dll/global/thread.h` | `f83259f6ca42ad24d497e71459fa17e95267199a75c97a8a3a9cfdcd245400f9` |
| `src/ntw16s-dll/global/vb_text.c` | `6a61aca5e12d9e99fae191ee512e9801219721d578ea30d83e9424881a139ca6` |
| `src/ntw16s-dll/global/vb_text.h` | `2cb836aaa78e9cd97f1e5ca17fbcb61303b40b2ab9adab2d8b70ee3b1da3678f` |
| `src/ntw16s-dll/global/window.c` | `72831c78fecf0bfe0a1348fd54675d28ebdebb17beff22524b7b35777b7c032b` |
| `src/ntw16s-dll/global/window.h` | `0d132845dfd8c51b7ef09d6bd251bd77190a355ab8d8881e06e6049a6a64f4d1` |
| `src/ntw16s-dll/uwp/hook.c` | `630afef0be94baddf5513822619d81ef1b412245b7617fbd6adbc01b833854c8` |
| `src/ntw16s-dll/uwp/hook.h` | `f116ce527e8907a9c85c3ef2f22a50fd93c27be573d35973649d4860db6cfc10` |
| `src/ntw16s-dll/uwp/target.c` | `3f43c3e7cf662e9bdbfdb52e19a5b76cb8664d74f23a714fda19326861bab5b9` |
| `src/ntw16s-dll/uwp/target.h` | `41d5c134d0962d9f4eced8f6528fd54e6f80588229c430336a6054dde3c89fa4` |
| `src/run16s-exe/console_host.c` | `012aa5a2f37dad369345e6ffe865445aa2ce9feacd345a2139fb1390d9817c7d` |
| `src/run16s-exe/console_host.h` | `c3859558bce097afe948f57905e6e3b9e0007e8625f6a2200cfd40900f05d1cc` |
| `src/run16s-exe/elevation.c` | `f3ad7d77cadf1dc3bbaa656e156dc4d684c2972a2541e4b246f4a1bae32da5c3` |
| `src/run16s-exe/elevation.h` | `1efaf78f468b93ac71d5c9d5f8a30019e393142edc9e6e3dd922bd4ab40e88ca` |
| `src/run16s-exe/explorer.c` | `077c539bc340c375bbf06713295aebe30de2dc57b67fc5716e2bb534a6145b3d` |
| `src/run16s-exe/explorer.h` | `7dcd5967a4283baca8fd037e406deab5e842dedf06527115e88332fb33a435ca` |
| `src/run16s-exe/global.c` | `8545a509f0d46144467b468d960afac8475e1d325bddc43597d1ce57fae8eaa0` |
| `src/run16s-exe/global.h` | `b8ddfb392020ed96bd6d4f54aeebdf208882bc551878d0b005e0b2b16751aa22` |
| `src/run16s-exe/main.c` | `4f4ef3076fddfbc10a86d068c1052196522e0703124857f0e7c0d64bada20bae` |
| `src/run16s-exe/process_lifecycle.c` | `0605ff4883ca08bbabe52c53a39a58a1c6ecbaaaeaa5910604fdfa4c719dcafb` |
| `src/run16s-exe/process_lifecycle.h` | `842360028f05c273ec3c8bb82d7b9fb333453da2c2525274eec54f8b47518b3d` |
| `src/run16s-exe/startup_error.c` | `314b2a8e8b2a2acd07160c97fd1fea14a60a3075c8dc78bd47b70eba5222ec6c` |
| `src/run16s-exe/startup_error.h` | `6cc25f791d932f7410fb5ab806e36f7e88ffdade085944baeddb1c00eb52ef15` |
| `src/run16s-exe/tray.h` | `8b198469b5337fc60aae4d7e5c724568e347474f4be18adee104b78ba514ad13` |
| `src/run16s-exe/tray_controller.c` | `8795eb50e6b9e0cdb7b8a2539216f4fcb13a7a511c7ecde053064932ba04350b` |
| `src/run16s-exe/uwp.c` | `2c09838dd0364842857b53ce37f1681d912ba62609c3c0f00fd67892ec8e75bb` |
| `src/run16s-exe/uwp.h` | `230f5a21ebdeff5ea854d760b5c29c48336aba68f494ec3ed09d1a0564d83926` |
| `src/run16s-exe/window_target.c` | `b33952b6777bfb41f32853e6f39a60f9ae75ffb2421d419f37f72dac32f28068` |
| `src/run16s-exe/window_target.h` | `aceba4577ac0e41ebe21e4763edeb74222d3602866d4419f2c6d27a196df0026` |
| `src/ntwshk-dll/context.c` | `cb0e7bdb56e58b4bc8003c2a4441c3e19ec1f13e725d2def4f69cd3ea070408a` |
| `src/ntwshk-dll/dllmain.c` | `24f11b55b59b9f67b073f34da632f010bd5b8a15fbfc5c5b2efc2d7b23ba9793` |
| `src/ntwshk-dll/helper.c` | `01e553ae718d138a2b24cf636fa62ae806b4d8b70ce30c4d1acf71672626e18a` |
| `src/ntwshk-dll/injection.h` | `e26830db4a8405bbe46f41f4fd49754903129f79addc5fae04b2957b9ee21c3a` |
| `src/ntwshk-dll/installer.c` | `f0abefcea10481529d4c6bb4323e388ed69b44bd8b98c63977bf30481182c329` |
| `src/ntwshk-dll/ntwshk.def` | `e7b8b4093f059263cd6ea7df46323d57d462427bd8909cb022688aaf60ea5a1d` |
| `src/ntwshk-dll/staging.h` | `bfa39ce0ec11379a1c1b4c2ca7a93a71aeeaadb1c50931a751e8b862c8307c70` |
| `src/ntwshk-dll/system_process.c` | `f6365216fe65f855f7955111d0a197392354942b85417c25c7203bf2fa4a65ab` |
| `src/ntwshk-dll/system_process.h` | `1e20b4caabfbf01ffca0fc8f13dc4a1c4a7137bea08b55a7916b97dd471bce09` |

单人复核与效率检查：追查每个注册/stop动作到现有公共实现和实际CMake消费者，新增证据是manager并非纯产品策略、window_target与通用dispatch的重叠、process_lifecycle的非样式归属及global/vb_text的反向迁移方向。没有产品代码修改、原生测试、整包重建或scratch；保留已有在途第41项。下一批对照公共库与产品根attachment/controls/theme的剩余接口，生成最终迁移构建清单及S1完成矩阵；未将局部审计计为S1收口。
