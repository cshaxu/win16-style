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
