# S41 Global Process: frozen audit and completion plan

## Baseline and scope

冻结基线为提交 `25558fba972d9e82ba45850fcf7b2f06360370d1`；另有正在进行的 `transaction.c/.h`、
`global_attach.c`、CMake、收据及测试未提交，作为 P10 候选，不能认定已交付。
构建 `s41-clean13` 在冻结时已运行；沿用其结果，不重启、不以旧包代替。
审计全集为下方逐文件符号清单，包括所有 global 文件、公共库及其产品调用者；
MinHook/HDE 上游源属于 process-hook 的现有实现，不重新复制。验证和最终状态
必须逐项回填，下面“待”均不是完成声明。

## Three orthogonal capabilities and dependencies

| Capability | Current production path and evidence | Status / required treatment |
| --- | --- | --- |
| Discovery and identities | `run16s_global_with_parent` -> `ntw16s_global_manager_start` -> `win32_process_global_window_start`, `win32_process_global_threads`, `win32_process_global_monitor_start`; `win32_process_global_policy_inspect*` | Public primitives exist; manager orchestration and per-target state remain coupled in `global_manager.c`, extract through callbacks. Filtering remains product supplied. |
| Registry, worker and failures | `global_manager.c` -> `process-global/registry`, `queue`, `dispatch`; `process_entry`, `reap`, `stop` own tickets | Index, FIFO and thread loop are public; target tracking, retries, stop and release coordination still product-coupled. Extract the reusable coordinator; caller owns policy, ticket callbacks and rendering actions. |
| Startup hook/staging | `run16s` and `injection/installer.c` -> `process-hook/staging`, `payload`, `helper`; `entry.c` -> `startup_hooks` | Existing public engine handles suspended new targets; x86/x64 helper route is separate. Product chooses payload and handler. Rollback is pre-commit; committed patches are retained. |
| Late hook / propagation | `global_thread.c` / `global_late.c` -> `ntw16s_late_process_init` -> `process-hook/late_patch` -> `ntw16s_attach_processes_using` and style registrations | **Not orthogonal today:** late init sets STYLE and PROPAGATE together; `context.c` requires STYLE. Separate supported process-hook/propagation selection and lifecycle from style without changing default product policy. Late guard cannot prove thread quiescence or creation exclusion. |
| Disable vs removal | `create_process.c::ntw16s_process_session_active`; `process-hook/late_patch.h`; `global_thread.c` unhooks WH_CALLWNDPROC | Session inactivity makes propagation callback pass through; target thread Windows hook is unhooked. Inline patches/trampolines are retained and late physical removal is not implemented. Do not describe pass-through as unload; pinned DLL may remain. |
| Style-only | `global_window.c::ntw16s_global_window_apply/remove` -> `attachment.c` -> win16-window/control/dialog | Same-thread HWND application/removal exists; needs an independent production-path test proving no hook installation/propagation. Product owns style classifiers, messages and restoration. |
| Composition | `run16s-exe/global.c`, `global_hook.c`, `global_thread.c`, `global_attach.c`, `global_session.c` | Product controller supplies session name/cookie, owner checks, target exclusions, apply/remove and user interaction. Generic orchestration must not depend on these headers/names/messages. |
| Module/loader transaction | `global_attach.c` -> `process-global/module`, `load`, `worker`, `remote_loader`, `remote_export`; proposed transaction | Existing generic mechanisms are called by product; proposed phase progression uncommitted. Validate no second implementation and error/ownership semantics. |
| Runtime notification | `global_hook.c` + `global_window.c` + `run16s-exe/global.c::sweep` | Product messages and style eligibility remain product; enumerating selected windows and bounded notification may be shared via generic callback without embedding message. |
| Session/recovery | `global_session.c`, `global_process.c`; `process-global/monitor`, `recovery` | ETW cleanup mechanism public with name parser callback; Win16-specific object names, cookie, process exclusion, security descriptor and style state remain adapter. Further generic handle/lifecycle pieces need audit. |

## Migration order and completion predicate

1. Preserve and validate the already-started P10 transaction, including both widths and failure/pending transitions.
2. Extract manager ownership/identity/dispatch and target lifecycle into a callback-driven public coordinator; adapter supplies policy and actions. Exercise discovery/management alone without hook/style.
3. Reuse process-hook backend; split propagation-only and style-only activation without changing default run16s behavior or expanding target/privilege scope. Cover same/cross-width existing and new targets where supported, and explicitly document physical-removal limits.
4. Verify style-only, hook-only and combined real paths separately; include target exit, retries, PID/TID reuse, timeout, stop race and resource ownership. Record visible drawing separately from attachment and hook initialization.
5. Audit every row and consumer below; add a repeatable dependency gate for product-private references and duplicate old paths. Do not claim completeness for skipped ETW, unavailable UAC, full matrix or unsupported restoration.
6. For each binary delivery: clean x86/x64 C17 GCC/Ninja/ccache build, same-snapshot four-file actual package test, deploy `assets/` and `O:/winnt`, SHA comparison, commit/push and remote tree verification. Close S41 only after all applicable requirements are met; keep M0/T21 open.


## Per-file dispositions at freeze

| Production file | Initial disposition / retained responsibility |
| --- | --- |
| `src/ntw16s-dll/global_attach.c` | 待：事务与生命周期公共化；Win16 控制消息及目标策略保留适配层，调用 module/load/worker。 |
| `src/ntw16s-dll/global_attach.h` | 待：事务与生命周期公共化；Win16 控制消息及目标策略保留适配层，调用 module/load/worker。 |
| `src/ntw16s-dll/global_discover.c` | 已：start/refresh/stop 转发 process-global/window；删除包装需核对全部调用者。 |
| `src/ntw16s-dll/global_discover.h` | 已：start/refresh/stop 转发 process-global/window；删除包装需核对全部调用者。 |
| `src/ntw16s-dll/global_hook.c` | 产品：WH_CALLWNDPROC 回调认证、窗口消息和样式调用；宿主生命周期可抽离，禁止公共库依赖消息。 |
| `src/ntw16s-dll/global_late.c` | 混合：线程调度及生命周期待分离；cookie 会话鉴权和样式通知保留适配层。 |
| `src/ntw16s-dll/global_late.h` | 混合：线程调度及生命周期待分离；cookie 会话鉴权和样式通知保留适配层。 |
| `src/ntw16s-dll/global_load.c` | 已：调用 process-global/load；DLL 文件名及导出解析为产品参数。 |
| `src/ntw16s-dll/global_load.h` | 已：调用 process-global/load；DLL 文件名及导出解析为产品参数。 |
| `src/ntw16s-dll/global_manager.c` | 待：发现、重试、目标状态、所有权与停止协调迁入公共协调器；窗口刷新和策略回调留产品。 |
| `src/ntw16s-dll/global_manager.h` | 待：发现、重试、目标状态、所有权与停止协调迁入公共协调器；窗口刷新和策略回调留产品。 |
| `src/ntw16s-dll/global_policy.c` | 已：通用检查/revalidate 在 process-global/policy；`ntw16s_global_image_allowed` 黑名单产品侧。 |
| `src/ntw16s-dll/global_policy.h` | 已：通用检查/revalidate 在 process-global/policy；`ntw16s_global_image_allowed` 黑名单产品侧。 |
| `src/ntw16s-dll/global_process.c` | 部分已：ETW 事件、孤儿回收在 monitor/recovery；名称解析和 peer 名称产品侧，peer 停止机制待分离。 |
| `src/ntw16s-dll/global_process.h` | 部分已：ETW 事件、孤儿回收在 monitor/recovery；名称解析和 peer 名称产品侧，peer 停止机制待分离。 |
| `src/ntw16s-dll/global_remote.c` | 已：loader 和 export 解析在 process-global；DLL 文件名与导出名适配保留。 |
| `src/ntw16s-dll/global_remote.h` | 已：loader 和 export 解析在 process-global；DLL 文件名与导出名适配保留。 |
| `src/ntw16s-dll/global_session.c` | 混合：映射协议、cookie、会话名属产品；进程生存时间及文件比较可复用，待抽离评估。 |
| `src/ntw16s-dll/global_session.h` | 混合：映射协议、cookie、会话名属产品；进程生存时间及文件比较可复用，待抽离评估。 |
| `src/ntw16s-dll/global_start.c` | 部分已：worker 生命周期公共；产品线程选择、就绪名称和 token 安全规则留适配，事件创建机制待审计。 |
| `src/ntw16s-dll/global_start.h` | 部分已：worker 生命周期公共；产品线程选择、就绪名称和 token 安全规则留适配，事件创建机制待审计。 |
| `src/ntw16s-dll/global_thread.c` | 混合：packet、会话身份、Win16 hook 回调及初始化在产品；线程等待/清理机制待分离。 |
| `src/ntw16s-dll/global_thread.h` | 混合：packet、会话身份、Win16 hook 回调及初始化在产品；线程等待/清理机制待分离。 |
| `src/ntw16s-dll/global_vb_text.c` | 产品：VB 字体适配和样式绘制，属于 win16-control 相关行为，不属于进程管理。 |
| `src/ntw16s-dll/global_vb_text.h` | 产品：VB 字体适配和样式绘制，属于 win16-control 相关行为，不属于进程管理。 |
| `src/ntw16s-dll/global_window.c` | 产品：GUI 线程上的样式附着/移除及会话查询，依赖 renderer；需证明独立样式控制。 |
| `src/ntw16s-dll/global_window.h` | 产品：GUI 线程上的样式附着/移除及会话查询，依赖 renderer；需证明独立样式控制。 |
| `src/run16s-exe/global.c`, `global.h` | 产品组合入口与用户体验；窗口枚举/有界通知及宿主通用生命周期待评估，Win16 消息、会话和策略由产品提供。 |
| `src/run16s-exe/main.c`, `tray_controller.c` | CLI/UAC 与托盘用户交互；调用全局管理器，不属于公共进程底层。 |
| `src/ntw16s-dll/injection/late_process.c`, `.h` | 待：通用晚挂钩批处理生命周期与产品 STYLE/PROPAGATE 耦合解开；现有 no-physical-removal 限制保留。 |
| `src/ntw16s-dll/injection/create_process.c` | 待：传播逻辑与样式载荷、会话策略分开；CreateProcess A/W 原生语义与失败清理必须保留。 |
| `src/ntw16s-dll/injection/entry.c`, `context.c`, `installer.c`, `injection.h`, `intercept.h` | 产品启动编排/载荷策略；STYLE 强制条件需针对独立传播能力解决；staging 不再复制。 |
| `src/ntw16s-dll/attachment.c`, `.h`, `controls.c`, `control_scope.c` | Win16 窗口和控件的产品适配/所有权；不应被 process-global 包含，保留实际样式应用与撤销。 |
| `src/lib-win32/process-global/*.[ch]` | 已入库机制；检查所有文件无产品名、私有头、重复产品实现；`transaction` 当前未提交待验证。 |
| `src/lib-win32/process-hook/*.[ch]`, `minhook/src/*` | 已有 C startup/late backend、同宽 staging 与载荷；移除能力按头文件契约，调用方不得复制 backend 或宣称物理卸载。 |

## Frozen test consumers and evidence gaps

- `tests/lib-win32/process-global/`: enumerate/registry、dispatch、module；transaction 当前未提交。它们验证局部契约，不证明完整管理器或实际绘制。
- `tests/lib-win32/process-hook/`、`tests/integration/late_process_prototype.c`：公共 payload/staging 与晚挂钩原型；不自动证明生产传播可独立于 STYLE。
- `tests/ntw16s-dll/global_{discover,policy,process,remote,load,start,attach,manager,manager_unsupported,thread,window,vb_text}.c`：产品调用链和局部失败；部分依赖私有桌面，ETW 集成需管理员，不能代称全矩阵。
- `tests/integration/global_*.c`、`tray_global.c`：已有/新建进程、跨位宽、生命周期及组合；逐例审计实际 marker 和环境。P9 跨目录曾与上版同样因托盘创建失败，尚未覆盖。
- 待新增或证明的独立测试：仅管理、仅传播、仅样式及三者组合；分别观察加载、初始化、后代传播、HWND 附着、实际绘制、停止/恢复；有限等待和自有实例清理。

## Frozen production file and symbol inventory

Names below are a broad source-token inventory (including callers/types, not a claim each is an export); compare to the final tree and account for removed, moved and retained entries. The underlying tree and diff remain authoritative.

| File | Global-related names and types observed |
| --- | --- |
| `src/ntw16s-dll/global_attach.c` | `Ntw16sGlobalThread`、`ntw16s_global_attach`、`ntw16s_global_attach_advance`、`ntw16s_global_attach_apply`、`ntw16s_global_attach_begin`、`ntw16s_global_attach_begin_thread`、`ntw16s_global_attach_close`、`ntw16s_global_attach_remove`、`ntw16s_global_files_equal`、`ntw16s_global_load_begin`、`ntw16s_global_load_discard`、`ntw16s_global_load_finish`、`ntw16s_global_policy`、`ntw16s_global_policy_inspect`、`ntw16s_global_policy_inspect_thread`、`ntw16s_global_policy_revalidate`、`ntw16s_global_policy_revalidate_thread`、`ntw16s_global_start_begin`、`ntw16s_global_start_close`、`ntw16s_global_start_ready`、`ntw16s_remote_global_thread`、`win32_process_global_mapped_identity`、`win32_process_global_select_path`、`win32_process_global_transaction_advance`、`win32_process_global_transaction_ops` |
| `src/ntw16s-dll/global_attach.h` | `ntw16s_global_attach`、`ntw16s_global_attach_advance`、`ntw16s_global_attach_apply`、`ntw16s_global_attach_begin`、`ntw16s_global_attach_begin_thread`、`ntw16s_global_attach_close`、`ntw16s_global_attach_remove`、`ntw16s_global_identity`、`ntw16s_global_load`、`ntw16s_global_policy`、`ntw16s_global_start` |
| `src/ntw16s-dll/global_discover.c` | `ntw16s_global_discover`、`ntw16s_global_discover_refresh`、`ntw16s_global_discover_start`、`ntw16s_global_discover_stop`、`ntw16s_global_discovered`、`win32_process_global_window_refresh`、`win32_process_global_window_start`、`win32_process_global_window_stop` |
| `src/ntw16s-dll/global_discover.h` | `ntw16s_global_discover`、`ntw16s_global_discover_refresh`、`ntw16s_global_discover_start`、`ntw16s_global_discover_stop`、`ntw16s_global_discovered`、`win32_process_global_window`、`win32_process_global_window_callback` |
| `src/ntw16s-dll/global_hook.c` | `Ntw16sGlobalHook`、`Ntw16sGlobalHost`、`ntw16s_global_image_allowed`、`ntw16s_global_late_schedule`、`ntw16s_global_manager`、`ntw16s_global_manager_start`、`ntw16s_global_manager_stop`、`ntw16s_global_manager_wait_idle`、`ntw16s_global_name`、`ntw16s_global_owner_alive`、`ntw16s_global_owner_matches`、`ntw16s_global_process_stop_peer`、`ntw16s_global_session`、`ntw16s_global_vb_text`、`ntw16s_global_window_apply`、`ntw16s_global_window_remove` |
| `src/ntw16s-dll/global_late.c` | `ntw16s_context`、`ntw16s_context_paths`、`ntw16s_global_late_mark_ready`、`ntw16s_global_late_schedule`、`ntw16s_global_policy_active`、`ntw16s_late_process_init` |
| `src/ntw16s-dll/global_late.h` | `ntw16s_global_late_mark_ready`、`ntw16s_global_late_schedule` |
| `src/ntw16s-dll/global_load.c` | `ntw16s_global_load`、`ntw16s_global_load_begin`、`ntw16s_global_load_discard`、`ntw16s_global_load_finish`、`ntw16s_remote_global_thread`、`ntw16s_remote_loadlibrary`、`win32_process_global_load_begin`、`win32_process_global_load_discard`、`win32_process_global_load_finish` |
| `src/ntw16s-dll/global_load.h` | `ntw16s_global_load`、`ntw16s_global_load_begin`、`ntw16s_global_load_discard`、`ntw16s_global_load_finish`、`win32_process_global_load` |
| `src/ntw16s-dll/global_manager.c` | `ntw16s_global_attach`、`ntw16s_global_attach_advance`、`ntw16s_global_attach_apply`、`ntw16s_global_attach_begin`、`ntw16s_global_attach_begin_thread`、`ntw16s_global_attach_close`、`ntw16s_global_discover`、`ntw16s_global_discover_refresh`、`ntw16s_global_discover_start`、`ntw16s_global_discover_stop`、`ntw16s_global_identity`、`ntw16s_global_manager`、`ntw16s_global_manager_changed`、`ntw16s_global_manager_refresh`、`ntw16s_global_manager_start`、`ntw16s_global_manager_stats`、`ntw16s_global_manager_stats_read`、`ntw16s_global_manager_stop`、`ntw16s_global_manager_test_process`、`ntw16s_global_manager_tracks`、`ntw16s_global_manager_wait_idle`、`ntw16s_global_name`、`ntw16s_global_owner_alive`、`ntw16s_global_policy`、`ntw16s_global_policy_init`、`ntw16s_global_policy_inspect_thread`、`ntw16s_global_policy_revalidate`、`ntw16s_global_process`、`ntw16s_global_process_start`、`ntw16s_global_process_stop`、`ntw16s_global_session`、`win32_process_global_dispatch`、`win32_process_global_dispatch_run`、`win32_process_global_queue`、`win32_process_global_queue_node`、`win32_process_global_queue_pop`、`win32_process_global_queue_push`、`win32_process_global_registry`、`win32_process_global_registry_find`、`win32_process_global_registry_init`、`win32_process_global_registry_insert`、`win32_process_global_registry_node`、`win32_process_global_registry_reap`、`win32_process_global_threads` |
| `src/ntw16s-dll/global_manager.h` | `ntw16s_global_manager`、`ntw16s_global_manager_changed`、`ntw16s_global_manager_refresh`、`ntw16s_global_manager_start`、`ntw16s_global_manager_stats`、`ntw16s_global_manager_stats_read`、`ntw16s_global_manager_stop`、`ntw16s_global_manager_test_process`、`ntw16s_global_manager_tracks`、`ntw16s_global_manager_wait_idle` |
| `src/ntw16s-dll/global_policy.c` | `ntw16s_global_identity`、`ntw16s_global_image_allowed`、`ntw16s_global_policy`、`ntw16s_global_policy_init`、`ntw16s_global_policy_inspect`、`ntw16s_global_policy_inspect_thread`、`ntw16s_global_policy_revalidate`、`ntw16s_global_policy_revalidate_thread`、`win32_process_global_policy_init`、`win32_process_global_policy_inspect`、`win32_process_global_policy_inspect_thread`、`win32_process_global_policy_revalidate`、`win32_process_global_policy_revalidate_thread` |
| `src/ntw16s-dll/global_policy.h` | `ntw16s_global_identity`、`ntw16s_global_image_allowed`、`ntw16s_global_policy`、`ntw16s_global_policy_init`、`ntw16s_global_policy_inspect`、`ntw16s_global_policy_inspect_thread`、`ntw16s_global_policy_revalidate`、`ntw16s_global_policy_revalidate_thread`、`win32_process_global_identity`、`win32_process_global_policy` |
| `src/ntw16s-dll/global_process.c` | `ntw16s_global_process`、`ntw16s_global_process_callback`、`ntw16s_global_process_event_pid`、`ntw16s_global_process_parse_name`、`ntw16s_global_process_start`、`ntw16s_global_process_stop`、`ntw16s_global_process_stop_peer`、`win32_process_global_monitor_event_pid`、`win32_process_global_monitor_start`、`win32_process_global_monitor_stop`、`win32_process_global_recover_orphans` |
| `src/ntw16s-dll/global_process.h` | `ntw16s_global_process`、`ntw16s_global_process_callback`、`ntw16s_global_process_event_pid`、`ntw16s_global_process_parse_name`、`ntw16s_global_process_start`、`ntw16s_global_process_stop`、`ntw16s_global_process_stop_peer`、`win32_process_global_monitor`、`win32_process_global_monitor_callback` |
| `src/ntw16s-dll/global_remote.c` | `Ntw16sGlobalThread`、`ntw16s_remote_global_thread`、`ntw16s_remote_loadlibrary`、`win32_process_global_remote_export`、`win32_process_global_remote_loadlibrary` |
| `src/ntw16s-dll/global_remote.h` | `ntw16s_remote_global_thread`、`ntw16s_remote_loadlibrary` |
| `src/ntw16s-dll/global_session.c` | `ntw16s_global_birth`、`ntw16s_global_files_equal`、`ntw16s_global_name`、`ntw16s_global_owner_alive`、`ntw16s_global_owner_matches`、`ntw16s_global_policy_active`、`ntw16s_global_session` |
| `src/ntw16s-dll/global_session.h` | `ntw16s_global_birth`、`ntw16s_global_files_equal`、`ntw16s_global_name`、`ntw16s_global_owner_alive`、`ntw16s_global_owner_matches`、`ntw16s_global_policy_active`、`ntw16s_global_session` |
| `src/ntw16s-dll/global_start.c` | `ntw16s_global_name`、`ntw16s_global_start`、`ntw16s_global_start_begin`、`ntw16s_global_start_close`、`ntw16s_global_start_ready`、`ntw16s_global_thread_packet`、`ntw16s_remote_global_thread`、`win32_process_global_worker_begin`、`win32_process_global_worker_close`、`win32_process_global_worker_ready` |
| `src/ntw16s-dll/global_start.h` | `ntw16s_global_start`、`ntw16s_global_start_begin`、`ntw16s_global_start_close`、`ntw16s_global_start_ready`、`win32_process_global_worker` |
| `src/ntw16s-dll/global_thread.c` | `Ntw16sGlobalHook`、`ntw16s_context`、`ntw16s_context_paths`、`ntw16s_global_birth`、`ntw16s_global_late_mark_ready`、`ntw16s_global_name`、`ntw16s_global_owner_alive`、`ntw16s_global_owner_matches`、`ntw16s_global_session`、`ntw16s_global_thread_packet`、`ntw16s_global_thread_worker`、`ntw16s_late_process_init` |
| `src/ntw16s-dll/global_thread.h` | `ntw16s_global_thread_packet`、`ntw16s_global_thread_worker` |
| `src/ntw16s-dll/global_vb_text.c` | `ntw16s_global_vb_text`、`ntw16s_vb_direct_text_allowed`、`ntw16s_vb_paint_active`、`ntw16s_vb_paint_font` |
| `src/ntw16s-dll/global_vb_text.h` | `ntw16s_global_vb_text` |
| `src/ntw16s-dll/global_window.c` | `Ntw16sWindowAttach`、`Ntw16sWindowAttached`、`Ntw16sWindowRemove`、`ntw16s_global_policy_active`、`ntw16s_global_window_apply`、`ntw16s_global_window_attached`、`ntw16s_global_window_remove` |
| `src/ntw16s-dll/global_window.h` | `Ntw16sWindowAttach`、`ntw16s_global_window_apply`、`ntw16s_global_window_attached`、`ntw16s_global_window_remove` |
| `src/lib-win32/process-global/dispatch.c` | `win32_process_global_dispatch`、`win32_process_global_dispatch_run` |
| `src/lib-win32/process-global/dispatch.h` | `win32_process_global_dispatch`、`win32_process_global_dispatch_next`、`win32_process_global_dispatch_run` |
| `src/lib-win32/process-global/enumerate.c` | `win32_process_global_thread_callback`、`win32_process_global_threads` |
| `src/lib-win32/process-global/enumerate.h` | `win32_process_global_thread_callback`、`win32_process_global_threads` |
| `src/lib-win32/process-global/identity.c` | `win32_process_global_desktop_status`、`win32_process_global_token_user` |
| `src/lib-win32/process-global/identity.h` | `win32_process_global_desktop_status`、`win32_process_global_token_user` |
| `src/lib-win32/process-global/load.c` | `win32_process_global_load`、`win32_process_global_load_begin`、`win32_process_global_load_discard`、`win32_process_global_load_finish`、`win32_process_global_loader_fn`、`win32_process_global_resolve_fn` |
| `src/lib-win32/process-global/load.h` | `win32_process_global_load`、`win32_process_global_load_begin`、`win32_process_global_load_discard`、`win32_process_global_load_finish`、`win32_process_global_loader_fn`、`win32_process_global_resolve_fn` |
| `src/lib-win32/process-global/module.c` | `win32_process_global_files_equal_fn`、`win32_process_global_mapped_identity`、`win32_process_global_mapped_identity_fn`、`win32_process_global_resolve_fn`、`win32_process_global_select_path` |
| `src/lib-win32/process-global/module.h` | `win32_process_global_files_equal_fn`、`win32_process_global_mapped_identity`、`win32_process_global_mapped_identity_fn`、`win32_process_global_resolve_fn`、`win32_process_global_select_path` |
| `src/lib-win32/process-global/monitor.c` | `win32_process_global_monitor`、`win32_process_global_monitor_callback`、`win32_process_global_monitor_event_pid`、`win32_process_global_monitor_start`、`win32_process_global_monitor_stop` |
| `src/lib-win32/process-global/monitor.h` | `win32_process_global_monitor`、`win32_process_global_monitor_callback`、`win32_process_global_monitor_event_pid`、`win32_process_global_monitor_start`、`win32_process_global_monitor_stop` |
| `src/lib-win32/process-global/policy.c` | `win32_process_global_desktop_status`、`win32_process_global_identity`、`win32_process_global_image_filter`、`win32_process_global_policy`、`win32_process_global_policy_init`、`win32_process_global_policy_inspect`、`win32_process_global_policy_inspect_thread`、`win32_process_global_policy_revalidate`、`win32_process_global_policy_revalidate_thread`、`win32_process_global_token_user` |
| `src/lib-win32/process-global/policy.h` | `win32_process_global_identity`、`win32_process_global_image_filter`、`win32_process_global_policy`、`win32_process_global_policy_init`、`win32_process_global_policy_inspect`、`win32_process_global_policy_inspect_thread`、`win32_process_global_policy_revalidate`、`win32_process_global_policy_revalidate_thread` |
| `src/lib-win32/process-global/queue.c` | `win32_process_global_queue`、`win32_process_global_queue_node`、`win32_process_global_queue_pop`、`win32_process_global_queue_push` |
| `src/lib-win32/process-global/queue.h` | `win32_process_global_queue`、`win32_process_global_queue_node`、`win32_process_global_queue_pop`、`win32_process_global_queue_push` |
| `src/lib-win32/process-global/recovery.c` | `win32_process_global_recover_orphans`、`win32_process_global_trace_name_parser`、`win32_process_global_trace_orphan` |
| `src/lib-win32/process-global/recovery.h` | `win32_process_global_recover_orphans`、`win32_process_global_trace_name_parser`、`win32_process_global_trace_orphan` |
| `src/lib-win32/process-global/registry.c` | `win32_process_global_registry`、`win32_process_global_registry_find`、`win32_process_global_registry_init`、`win32_process_global_registry_insert`、`win32_process_global_registry_node`、`win32_process_global_registry_reap`、`win32_process_global_registry_release_fn`、`win32_process_global_registry_remove_fn` |
| `src/lib-win32/process-global/registry.h` | `win32_process_global_registry`、`win32_process_global_registry_find`、`win32_process_global_registry_init`、`win32_process_global_registry_insert`、`win32_process_global_registry_node`、`win32_process_global_registry_reap`、`win32_process_global_registry_release_fn`、`win32_process_global_registry_remove_fn` |
| `src/lib-win32/process-global/remote_export.c` | `win32_process_global_remote_export` |
| `src/lib-win32/process-global/remote_export.h` | `win32_process_global_remote_export` |
| `src/lib-win32/process-global/remote_loader.c` | `win32_process_global_remote_loadlibrary` |
| `src/lib-win32/process-global/remote_loader.h` | `win32_process_global_remote_loadlibrary` |
| `src/lib-win32/process-global/transaction.c` | `win32_process_global_transaction_advance`、`win32_process_global_transaction_ops` |
| `src/lib-win32/process-global/transaction.h` | `win32_process_global_transaction_advance`、`win32_process_global_transaction_ops`、`win32_process_global_transaction_step` |
| `src/lib-win32/process-global/window.c` | `win32_process_global_window`、`win32_process_global_window_callback`、`win32_process_global_window_refresh`、`win32_process_global_window_start`、`win32_process_global_window_stop` |
| `src/lib-win32/process-global/window.h` | `win32_process_global_window`、`win32_process_global_window_callback`、`win32_process_global_window_refresh`、`win32_process_global_window_start`、`win32_process_global_window_stop` |
| `src/lib-win32/process-global/worker.c` | `win32_process_global_worker`、`win32_process_global_worker_begin`、`win32_process_global_worker_close`、`win32_process_global_worker_ready` |
| `src/lib-win32/process-global/worker.h` | `win32_process_global_worker`、`win32_process_global_worker_begin`、`win32_process_global_worker_close`、`win32_process_global_worker_ready` |
| `src/lib-win32/process-hook/helper.c` | `process_hook_create_w`、`process_hook_helper_apply`、`process_hook_install_helper`、`process_hook_run_helper` |
| `src/lib-win32/process-hook/helper.h` | `process_hook_create_w`、`process_hook_helper_apply`、`process_hook_install_helper`、`process_hook_run_helper` |
| `src/lib-win32/process-hook/late_guard.c` | `process_hook_late_guard`、`process_hook_late_guard_begin`、`process_hook_late_guard_end`、`process_hook_late_guard_test_after_suspend`、`process_hook_late_guard_test_before_suspend`、`process_hook_late_guard_test_fail_after`、`process_hook_late_guard_test_fail_free`、`process_hook_late_guard_test_identity`、`process_hook_late_region` |
| `src/lib-win32/process-hook/late_guard.h` | `process_hook_late_guard`、`process_hook_late_guard_begin`、`process_hook_late_guard_end`、`process_hook_late_guard_test_after_suspend`、`process_hook_late_guard_test_before_suspend`、`process_hook_late_guard_test_fail_after`、`process_hook_late_guard_test_fail_free`、`process_hook_late_guard_test_identity`、`process_hook_late_region` |
| `src/lib-win32/process-hook/late_patch.c` | `process_hook_late_guard`、`process_hook_late_guard_begin`、`process_hook_late_guard_end`、`process_hook_late_patch`、`process_hook_late_patch_discard`、`process_hook_late_patch_install`、`process_hook_late_patch_install_batch`、`process_hook_late_patch_prepare`、`process_hook_late_patch_test_fail_protect_after`、`process_hook_late_region` |
| `src/lib-win32/process-hook/late_patch.h` | `process_hook_late_guard`、`process_hook_late_patch`、`process_hook_late_patch_discard`、`process_hook_late_patch_install`、`process_hook_late_patch_install_batch`、`process_hook_late_patch_prepare`、`process_hook_late_patch_test_fail_protect_after` |
| `src/lib-win32/process-hook/payload.c` | `process_hook_payload_reader`、`process_hook_payload_take`、`process_hook_payload_write` |
| `src/lib-win32/process-hook/payload.h` | `process_hook_payload_reader`、`process_hook_payload_take`、`process_hook_payload_write` |
| `src/lib-win32/process-hook/staging.c` | `process_hook_payload_consumer`、`process_hook_stage_payload`、`process_hook_stage_process`、`process_hook_stage_read`、`process_hook_stage_restore` |
| `src/lib-win32/process-hook/staging.h` | `process_hook_payload_consumer`、`process_hook_stage_payload`、`process_hook_stage_process`、`process_hook_stage_read`、`process_hook_stage_restore` |
| `src/lib-win32/process-hook/startup_hooks.c` | `process_hook_startup_abort`、`process_hook_startup_attach`、`process_hook_startup_begin`、`process_hook_startup_commit`、`process_hook_startup_committed`、`process_hook_startup_count` |
| `src/lib-win32/process-hook/startup_hooks.h` | `process_hook_startup_abort`、`process_hook_startup_attach`、`process_hook_startup_begin`、`process_hook_startup_commit` |
| `src/run16s-exe/global.c` | `Ntw16sGlobalHook`、`Ntw16sGlobalHost`、`ntw16s_context`、`ntw16s_context_paths`、`ntw16s_global_birth`、`ntw16s_global_manager`、`ntw16s_global_manager_refresh`、`ntw16s_global_manager_start`、`ntw16s_global_manager_stop`、`ntw16s_global_manager_wait_idle`、`ntw16s_global_name`、`ntw16s_global_process_stop_peer`、`ntw16s_global_session`、`ntw16s_validate_image`、`run16s_global`、`run16s_global_with_parent` |
| `src/run16s-exe/global.h` | `run16s_global`、`run16s_global_with_parent` |
| `src/run16s-exe/tray_controller.c` | `ntw16s_global_name`、`ntw16s_global_session`、`run16s_global_with_parent` |
| `src/run16s-exe/main.c` | `ntw16s_context`、`ntw16s_context_paths`、`ntw16s_install`、`ntw16s_validate_image_ex`、`run16s_global_interactive_console` |
| `src/ntw16s-dll/injection/late_process.c` | `ntw16s_attach_com_dialogs_using`、`ntw16s_attach_file_dialogs_using`、`ntw16s_attach_folder_dialogs_using`、`ntw16s_attach_processes_using`、`ntw16s_attach_titles_using`、`ntw16s_late_process_init`、`ntw16s_process_context`、`process_hook_late_guard`、`process_hook_late_patch`、`process_hook_late_patch_discard`、`process_hook_late_patch_install_batch`、`process_hook_late_patch_prepare` |
| `src/ntw16s-dll/injection/late_process.h` | `ntw16s_late_process_init` |
| `src/ntw16s-dll/injection/create_process.c` | `ntw16s_attach_processes`、`ntw16s_attach_processes_using`、`ntw16s_global_policy_active`、`ntw16s_hooks_attach`、`ntw16s_install`、`ntw16s_process_context`、`ntw16s_process_session_active`、`ntw16s_system_print_host_a`、`ntw16s_system_print_host_w`、`ntw16s_unwind_scope` |
| `src/ntw16s-dll/injection/entry.c` | `Ntw16sFinishHelper`、`ntw16s_attach_com_dialogs`、`ntw16s_attach_file_dialogs`、`ntw16s_attach_folder_dialogs`、`ntw16s_attach_processes`、`ntw16s_attach_titles`、`ntw16s_bootstrap`、`ntw16s_context`、`ntw16s_context_read`、`ntw16s_hooks_abort`、`ntw16s_hooks_begin`、`ntw16s_hooks_commit`、`ntw16s_process_context`、`ntw16s_run_helper`、`ntw16s_stage_restore` |
| `src/ntw16s-dll/injection/context.c` | `ntw16s_context`、`ntw16s_context_paths`、`ntw16s_context_read`、`ntw16s_packet`、`ntw16s_packet_parse`、`ntw16s_payload_guid`、`ntw16s_stage_read`、`process_hook_stage_read` |
| `src/ntw16s-dll/injection/installer.c` | `ntw16s_context`、`ntw16s_create_w`、`ntw16s_install`、`ntw16s_install_helper`、`ntw16s_packet`、`ntw16s_packet_parse`、`ntw16s_process_machine`、`ntw16s_stage_process`、`ntw16s_validate_image`、`ntw16s_validate_image_ex` |
| `src/ntw16s-dll/injection/injection.h` | `ntw16s_context`、`ntw16s_context_paths`、`ntw16s_context_read`、`ntw16s_create_w`、`ntw16s_install`、`ntw16s_packet`、`ntw16s_packet_parse`、`ntw16s_payload_guid`、`ntw16s_process_machine`、`ntw16s_validate_image`、`ntw16s_validate_image_ex` |
| `src/ntw16s-dll/injection/intercept.h` | `ntw16s_attach_com_dialogs`、`ntw16s_attach_com_dialogs_using`、`ntw16s_attach_file_dialogs`、`ntw16s_attach_file_dialogs_using`、`ntw16s_attach_folder_dialogs`、`ntw16s_attach_folder_dialogs_using`、`ntw16s_attach_processes`、`ntw16s_attach_processes_using`、`ntw16s_attach_titles`、`ntw16s_attach_titles_using`、`ntw16s_classic_folder`、`ntw16s_context`、`ntw16s_creation_begin_dialog`、`ntw16s_creation_dialog_observer`、`ntw16s_creation_scope`、`ntw16s_dialog_style_scope`、`ntw16s_folder_observer`、`ntw16s_font_dialog_chrome`、`ntw16s_modern_dialog_active`、`ntw16s_old_dialog_chrome`、`ntw16s_old_dialog_folder`、`ntw16s_process_context`、`ntw16s_process_session_active` |
| `src/ntw16s-dll/attachment.c` | `Ntw16sCreationBegin`、`Ntw16sCreationEnd`、`Ntw16sScrollChanged`、`Ntw16sWindowAttach`、`Ntw16sWindowAttached`、`Ntw16sWindowRemove`、`Ntw16sWindowTerminal`、`ntw16s_bit_theme_frame`、`ntw16s_box_witness`、`ntw16s_checklist_draw_target`、`ntw16s_class_is`、`ntw16s_control_protocol`、`ntw16s_control_protocol_release`、`ntw16s_controlbar_edge`、`ntw16s_controlbar_edge_active`、`ntw16s_controlbar_edge_allowed`、`ntw16s_creation_begin_dialog`、`ntw16s_creation_dialog_observer`、`ntw16s_creation_result`、`ntw16s_creation_scope`、`ntw16s_dbcombo_ansi_witness`、`ntw16s_dbedit_ansi_witness`、`ntw16s_grid_valid`、`ntw16s_group_button_valid`、`ntw16s_group_choice_button`、`ntw16s_group_frame`、`ntw16s_group_push_button`、`ntw16s_inplace_button`、`ntw16s_inplace_dropdown`、`ntw16s_inplace_paint`、`ntw16s_modern_spin_blit`、`ntw16s_modern_spin_dc_created`、`ntw16s_modern_spin_dc_deleted`、`ntw16s_modern_spin_object_deleted`、`ntw16s_modern_spin_selected`、`ntw16s_modern_spin_theme`、`ntw16s_modern_spin_theme_begin`、`ntw16s_month_arrow`、`ntw16s_month_header_text`、`ntw16s_month_selection_background`、`ntw16s_month_selection_text`、`ntw16s_month_text_borrow`、`ntw16s_month_text_room`、`ntw16s_native_get_combo_info`、`ntw16s_native_get_scroll_info`、`ntw16s_nav_theme`、`ntw16s_panel_polyline`、`ntw16s_rich_valid`、`ntw16s_rich_witness`、`ntw16s_spin_frame`、`ntw16s_static_call`、`ntw16s_static_call_begin`、`ntw16s_static_call_end`、`ntw16s_static_call_suspend`、`ntw16s_static_text_font`、`ntw16s_status_frame`、`ntw16s_status_hooks_enable`、`ntw16s_status_icon_frame`、`ntw16s_status_simple_font`、`ntw16s_status_simple_return`、`ntw16s_status_text_frame`、`ntw16s_status_theme_begin`、`ntw16s_status_theme_end`、`ntw16s_status_theme_scope`、`ntw16s_theme_cancel_stage`、`ntw16s_theme_capture`、`ntw16s_theme_commit`、`ntw16s_theme_enable`、`ntw16s_theme_remove`、`ntw16s_theme_stage`、`ntw16s_theme_terminal`、`ntw16s_unwind_scope`、`ntw16s_vb_data_attached`、`ntw16s_vb_data_valid`、`ntw16s_vb_fonts_clear`、`ntw16s_vb_paint`、`ntw16s_vb_paint_begin`、`ntw16s_vb_paint_end`、`ntw16s_vb_picture`、`ntw16s_vb_user_edge_dormant`、`ntw16s_vb_user_edge_valid`、`ntw16s_vb_user_label_target`、`ntw16s_vcl_combo_candidate`、`ntw16s_vcl_combo_valid`、`ntw16s_vcl_tab_valid`、`ntw16s_vcl_toolbar_valid` |
| `src/ntw16s-dll/attachment.h` | `Ntw16sCreationBegin`、`Ntw16sCreationEnd`、`Ntw16sScrollChanged`、`Ntw16sSetWindowTheme`、`Ntw16sWindowAttached`、`Ntw16sWindowRemove`、`Ntw16sWindowTerminal`、`ntw16s_creation_result`、`ntw16s_creation_scope` |
| `src/ntw16s-dll/controls.c` | `Ntw16sWindowRemove`、`ntw16s_button_type`、`ntw16s_checklist_draw_target`、`ntw16s_class_is`、`ntw16s_control_protocol_is`、`ntw16s_grid_valid`、`ntw16s_group_button_valid`、`ntw16s_group_choice_button`、`ntw16s_group_push_button`、`ntw16s_managed_scrollbars_active`、`ntw16s_native_get_combo_info`、`ntw16s_native_get_scroll_info`、`ntw16s_native_set_scroll_info`、`ntw16s_rich_valid`、`ntw16s_theme_capture`、`ntw16s_theme_enable`、`ntw16s_theme_remove`、`ntw16s_theme_retained`、`ntw16s_theme_terminal`、`ntw16s_unwind_scope`、`ntw16s_vb_button`、`ntw16s_vb_checkbox`、`ntw16s_vb_combo`、`ntw16s_vb_data_attached`、`ntw16s_vb_data_valid`、`ntw16s_vb_drive`、`ntw16s_vb_frame`、`ntw16s_vb_graphical_button`、`ntw16s_vb_graphical_choice`、`ntw16s_vb_list`、`ntw16s_vb_option`、`ntw16s_vb_plain_choice`、`ntw16s_vb_user_edge_dormant`、`ntw16s_vb_user_edge_valid`、`ntw16s_vcl_combo_candidate`、`ntw16s_vcl_combo_valid`、`ntw16s_vcl_tab_valid`、`ntw16s_vcl_toolbar_valid` |
| `src/ntw16s-dll/control_scope.c` | 内部回调/类型，详见文件 |


## 冻结审计后的进展（S41 P10–P13）

上文为迁移前冻结基线；以下记录增量，不把尚待完成的条目视为已解决。

| 阶段 | 实际变化和验证 | 仍待完成 |
| --- | --- | --- |
| P10 | 接入事务状态推进迁入 process-global，产品提供具体操作回调；双宽事务及实包聚焦通过。 | 独立协调与独立传播接口。 |
| P11 | 产品内部支持仅传播策略；八种父子位宽/A/W 组合观察子孙加载、未附着、停用与重启。 | 公共传播 API、真实像素及物理卸载；父探针样式注册为桩。 |
| P12 | targets 管理目标索引、去重、队列、重试、工作状态与回收；双宽自有进程仅管理测试通过。 | 发现和工作线程的完整公共协调。 |
| P13 | targets 提供关闭、失败重试及进程查询；双宽仅样式 Button 的 WM_PRINT 绘制变化与恢复、两个创建入口字节不变；18 项实包聚焦通过。 | 屏幕实际显示、更多控件及独立管理器/传播 API；该测试不证明所有钩子均未安装。 |

P13 构建 `s41-clean16`、测试 `s41-package16`、发布 `s41-publish13`；双宽候选 DLL 与测试构建 DLL 摘要一致。管理员 ETW、完整矩阵和跨目录既有限制未因聚焦通过而解除；保留 S41 开放。

P14：`process-global/dispatch` 提供公共线程池启动、限时等待与释放；`global_manager.c` 使用该接口并继续提供处理回调。双宽独立池测试覆盖超时后重试及重新启动，双宽干净构建和 14 项实际包聚焦通过；独立发现协调和公共传播接口仍未实现。发布 `s41-publish14`。

P15：`process-global/engine` 组合目标表与线程池，负责启动、消息泵空闲等待、停止和失败重试；`global_manager.c` 提供策略/接入回调并使用公共入口。双宽独立自有目标生命周期测试和 16 项实际包聚焦通过；`global-e2e` 私有桌面托盘创建失败（2147500037，manager_not_active=2），上一版 P14 同条件同错，未计通过。发布 `s41-publish15`；产品侧发现编排和公共传播接口仍待处理。

P16：`process-global/enumerate` 提供存量线程扫描和单进程首个就绪线程/暂未就绪回退协调；`global_manager.c` 提供产品许可、会话与桌面检查。双宽独立线程选择测试及 16 项实包聚焦通过；发布 `s41-publish16`。全局端到端托盘和管理员 ETW 限制保持不变，公共传播接口仍待完成。

P17：`process-hook/propagate` 负责原有创建拦截、暂停创建、重入保护、成功恢复与失败回收，`process-hook/unwind` 提供原有 C 异常展开；`injection/create_process.c` 提供会话、排除与载荷回调并调用公共入口。独立同位宽真实钩子测试验证无样式 DLL 的纯载荷子孙、A/W、停用、拒绝及 193 失败回收；产品实包 28 项通过，含双宽/AW 的仅传播和组合各八项。停用不等于物理卸载。`global-e2e` 的私有桌面托盘限制、管理员 ETW 和最终逐项审计仍待处理。发布 `s41-publish17`。

P18：`process-global/window` 提供带截止时间和取消的窗口树遍历，控制器和管理器分别提供过滤与样式消息；`process-global/transaction` 负责原有接入等待、取消及重试，产品保留会话、身份、载荷与通知。双宽独立测试和 20 项实包聚焦通过，发布 `s41-publish18`；真实全局端到端托盘限制及最终全项审计仍待处理。

P19：`process-global/identity` 提供进程创建时间与桌面指纹，`process-global/module` 提供模块文件字节比较；产品会话/ETW 代码以及公共策略/恢复复用，保留产品对象命名。`tools/check_process_global_boundary.py` 检查四个公共组件源码和构建静态库的产品私有引用，已接入打包门禁；双宽干净构建、独立测试及 20 项实包聚焦通过，发布 `s41-publish19`。全局端到端托盘和管理员 ETW 限制、最终审计仍未解除。

P20：`process-global/window` 负责线程钩子安装、就绪、等待和撤销；`process-global/background` 管理后台调度和完成标记；`process-global/recovery` 只在同伴进程已退出且身份匹配时停止调用方命名的跟踪。`global_thread.c`、`global_late.c`、`global_process.c` 提供产品策略和协议。双宽自有窗口钩子及后台测试、无跟踪会话负例和实际包 20 项聚焦通过，含双宽本地全局钩子与子孙传播；真实管理员 ETW 正例及完整端到端未覆盖。发布 `s41-publish20`。

P21：`process-hook/late_batch` 管理已有晚期钩子批量准备、重试、失败清理和危险状态保留，产品 `late_process.c` 选择注册传播及样式回调；安装后没有物理卸载。仅样式测试增加双宽 A/W 子进程 DLL 缺席和入口字节对照，已发布 run16s 双宽负例退出 42。双宽干净构建及 28 项实际包聚焦通过，发布 `s41-publish21`；底层 88 条上限需增加公共入口早期验证，ETW 正例和真实全局托盘仍未覆盖。


## P22 绘制恢复对照

实际管理器自有目标同一 Button 的 `WM_PRINT` 像素摘要在 x86/x64 增量对照均为原始 343534586、应用 2033954149、移除 2875890938：应用确实改变绘制，但移除后不等于原始，不得称为完全恢复。P20 已发布 DLL 在同一探针中得到同样结果；不点击该按钮仍复现，控件样式位相同，`WM_GETFONT` 均为 0。原因未确认；采样为内存 DIB 的真实绘制，不是屏幕截图。管理器测试要求绘制变化并记录恢复结果，故其通过不表示恢复像素相同。用户允许无法完全恢复时的残影；保留限制并在最终报告说明。

P22：`PROCESS_HOOK_LATE_PATCH_LIMIT` 公布 88 条，并在 `process_hook_late_batch_begin` 超额时返回 `ERROR_INVALID_PARAMETER`；双宽干净构建及批处理负例通过。实际包 `s41-package26` 20/20 聚焦通过，双宽管理器日志分别记录 `changed=1 restored_equal=0`；发布 `s41-publish22`。该聚焦通过不表示像素恢复通过。


## 冻结清单逐项复审（P23，尚未收口）

此表基于 P22 已发布源码；冻结时的旧分类保留在上文供比较。每行所列测试是该项证据，不能推广为完整矩阵；`global-e2e` 私有桌面托盘失败与管理员 ETW 未覆盖继续有效。

| 冻结条目 | 当前公共入口与实际产品调用位置 | 留在产品侧的具体原因 | 对应验证及边界 |
| --- | --- | --- | --- |
| `global_attach.c/.h` | `transaction_advance/run`、`select_path`、`load_*`、`worker_*`；`ntw16s_global_attach_begin/advance/close` 和 `global_manager.c::process_entry` 调用 | ticket 内 cookie、目标身份、注册消息、APPLY/QUERY/REMOVE 及 ACK 值是产品协议；调用方回调决定如何处理 | P22 双宽 attach/manager/stop-race；超时、无窗口及失败由 `tests/lib-win32/process-global/transaction.c` 验证 |
| `global_discover.c/.h` | `window_start/refresh/stop`，产品包装仅转换既有 API；`global_manager_start/refresh/stop` 调用 | 保留既有产品内部接口及测试调用者；没有第二套枚举算法 | 双宽 `global-discover` 与自有目标测试；窗口身份须在使用前重验 |
| `global_manager.c/.h` | `engine_start/submit/wait_idle/stop`、`scan_existing/scan_process`、`targets_*`、`transaction_run`、`window_walk` | 会话阶段/排除、DLL 路径、接入 ticket、重验证及窗口消息是产品特定；公共引擎拥有队列、去重、工作线程、失败和关闭 | 双宽仅管理、manager、停止竞态、目标退出与替换；PID/TID 重用仅用篡改出生时间模拟，未强制系统真实重用 |
| `global_policy.c/.h` | `policy_init/inspect/inspect_thread/revalidate`、`identity`；产品 `ntw16s_global_image_allowed` 回调 | Windows 宿主排除及产品目标资格是产品选择；公共 policy 是可选的既有同用户/桌面及 AppContainer 限制，调用方可使用其他策略 | 双宽 `global-policy`，含错误 PID/TID、位宽、出生时间过期；其他 window station 启动失败时为 skip |
| `global_load.c/.h`、`global_remote.c/.h` | `load_begin/finish/discard`、`remote_loadlibrary`、`remote_export`；产品提供路径及导出名称 | 具体 DLL、导出和身份约束属于产品，不嵌入公共库 | 双宽 load/remote/attach；跨位宽由对应位宽宿主或 process-hook helper 配合，非单个 API 跨宽任意装载 |
| `global_start.c/.h` | `worker_begin/ready/close`，产品 `ntw16s_global_start_*` 调用 | 线程 packet、事件命名及低完整性用户 SDDL 属现有安全策略，必须保留原 ACL 和错误行为；通用远端内存、线程与就绪资源在公共 worker | 双宽 global-start、thread、attach；未通过更改权限掩盖 ETW 限制 |
| `global_thread.c/.h` | `window_hook_wait` 负责线程钩子、就绪、等待和撤销；产品 worker 调用 | packet/session cookie、所属进程验证、DLL 固定、具体 WH_CALLWNDPROC 回调和 Win16 初始化为产品职责 | 双宽独立同进程钩子接收/撤销、global-thread 与 owner-loss；移除线程钩子不等于移除 inline trampoline |
| `global_process.c/.h` | `monitor_start/stop/event_pid`、`recover_orphans`、`trace_stop_peer`；产品提供名称解析 | 只停止产品命名的会话；保留名称生成和已退出同伴身份约束 | 双宽解析、无会话及已退出自有进程负例；真实管理员 ETW 正例未执行，不计通过 |
| `global_session.c/.h` | `birth`、`desktop_key`、`files_equal`；`ntw16s_global_name/owner_alive/owner_matches` 调用 | 映射布局、owner/cookie、产品可执行文件和 DLL 名及身份认证属于产品协议；旧 helper 名称仅为头文件别名 | 双宽身份/模块测试及产品策略、接入；不视别名为第二套实现 |
| `global_late.c/.h`、`injection/late_process.c/.h` | `background_schedule/mark`、`late_batch_begin/prepare/commit`、`late_patch_*`；产品注册回调 | 会话校验、DLL 固定、STYLE/PROPAGATE 组合及样式消息为产品职责；公共批处理 88 项上限、失败后持有资源 | 双宽后台、late-batch、16 个产品 A/W 位宽组合；线程创建不被 guard 排除，不能安全物理卸载 |
| `injection/create_process.c`、`entry.c`、`context.c`、`installer.c`、`injection.h`、`intercept.h` | `propagation_attach`、`staging`、`payload`、`helper`、`startup_hooks`；产品提供会话许可、排除、载荷和初始化 | 产品载荷字段、DLL 路径、helper 与 session 策略需由产品提供；普通 run16s 默认保持原组合 | 独立同宽无样式 DLL 子孙载荷 A/W、停用/193；产品双宽与跨宽 A/W 验证；独立通用 helper 跨宽全矩阵未证明 |
| `global_hook.c`、`global_window.c/.h`、`global_vb_text.c/.h`、`attachment.c/.h`、`controls.c`、`control_scope.c` | 产品调用 `Ntw16sWindowAttach/Remove` 与 win16-window/control/dialog；不被公共 process-global 依赖 | 窗口线程内的产品鉴权、样式分类、绘制和恢复策略；不是通用进程生命周期 | 双宽仅样式 DIB 绘制及 A/W 子进程无 DLL；组合 manager 中 DIB changed=1，但 restored_equal=0，屏幕采样未做 |
| `run16s-exe/global.c/.h`、`main.c`、`tray_controller.c` | `window_walk`、`engine`、`process-hook`、`lib-tray`；`run16s_global_with_parent` 组合 | 托盘交互、会话对象/安全描述符、x86 宿主、具体回调、消息及启动停止顺序是产品组合 | 已发布包局部 manager 与 local-hook 双宽；真实 `--global` 私有桌面托盘创建失败，不能视为端到端通过 |
| `process-global/*`、`process-hook/*` 及 MinHook | 公共 C API 见各头；产品调用点见上各行。`check_process_global_boundary.py` 在打包检查源码与两个位宽静态库未定义符号 | MinHook 版权/资源来源不变；其 late guard 不能排除新建线程或保证调用退出；保留运行期 trampoline | `s41-clean25` 源码/静态库门禁、ABI；停止会话使传播 pass-through，不能称为卸载 DLL 或还原所有代码 |

### 独立与组合证据边界

- 仅管理：`tests/lib-win32/process-global/targets.c` 启动自有子进程和窗口，验证发现、去重、目标处理、关闭失败重试及无样式 DLL；未证明系统所有 PID/TID 的真实重用。
- 仅传播：`tests/lib-win32/process-hook/propagate.c` 使用真实本进程 A/W 拦截和同位宽纯载荷子孙、暂停/禁用/拒绝/193 回滚；跨宽测试目前在产品适配器和对应 DLL 中。
- 仅样式：`tests/ntw16s-dll/global_window.c` 直接应用和移除，DIB 绘制改变并恢复，A/W 自有子进程无 DLL；普通 run16s 双位宽启动相同子探针均返回 42，证明加载负例可检出。
- 产品组合：`tests/ntw16s-dll/global_manager.c` 双宽自有现有/新建目标、点击、退出、停止竞态；DIB 显示实际绘制改变而移除后仍不等。`global-hook-local` 双宽看到子孙 DLL 和退出，但不能取代真实托盘端到端。
- P22 的恢复差异、窗口截图未覆盖、真实管理员 ETW 与私有桌面托盘端到端失败均保留；停止后的 inline 钩子和固定 DLL 可能留在目标进程。

P23：`window_start_ex` 允许公共调用方显式发现自身窗口，产品仍通过旧 `window_start` 排除自身；双宽增量测试覆盖已有和新建窗口。已发布包 `s41-audit1` 聚焦 15 项通过，但双宽其他 window station 子项均输出 SKIP（exit 12），未提权控制器提示 ETW 不可用；自有父进程退出后的控制器会话清理通过，测试没有目标窗口。

P23 交付证据：`s41-clean27` 双位宽干净构建、ABI、来源与资源及公共源码/静态库依赖门禁通过；`core/process-global-window` 干净树双宽通过。`s41-package27` 同批实际包聚焦 23 项通过，测试本身报告零顶层跳过，但两个 policy 日志分别包含其他 window station 子项 SKIP（exit 12）；父进程日志报告未启用管理员 ETW。manager 双宽均为 changed=1、restored_equal=0。四文件经 `s41-publish23` 发布到 assets 与 O:/winnt，摘要一致；真实托盘端到端、管理员 ETW、真实 PID/TID 重用及上述恢复差异仍未证明。
