# Shared style test map

| Source family | Native test adapter |
| --- | --- |
| `creation_call` | `lifecycle/creation_call.c` (inactive/prepare retry/once/result priority/native exception cleanup and post-unwind reuse); real window/dialog API consumers |
| `creation` | `lifecycle/creation.c` (real thread hooks, nested barriers/order, prepare/install/OOM, optional subclass and failed hook restoration); `tests/ntw16s-dll/dialog_scope.c` (actual renderer first-show and delayed initializer); real DLL nested/WNDPROC callers |
| `instance` | `lifecycle/instance.c`; `tests/lib-win32-style/appearance/win2000/window/instance_lifetime.c`; Win3.x frame/control and modern host production consumers |
| `present` | `graphics/present.c`; `graphics/present_fault.c`; `graphics/present_print.c`; `tests/ntw16s-dll/win16_style_control_present.c` |
| `clip` | `graphics/clip.c` (full and nonrectangular clipping, DC mapping restoration, print clip snapshot and failure); Win16 spin paint and color/date print consumers |
| `paint_frame` | `graphics/paint_frame.c` (frame thickness, interior and outside pixels); Win16 basic control frame consumers |
| `pixel_grid` | `graphics/pixel_grid.c` (indexed colors, transparency, brush and fractional scaling); Win16 combo, choice and scroll glyph consumers |
| `pixel_dib` | `graphics/pixel_dib.c` (indexed colors, scaling, clipping and brush preservation); Win16 checklist glyph consumer |
| `icon_size` | `graphics/icon_size.c` (color and monochrome sizes, invalid handle and cleanup); Win16 static and button image consumers |
| `window_text` | `window/window_text.c` (Unicode, independent snapshot and empty text); Win16 choice and combo consumers |
| `dc_geometry` | `graphics/dc_geometry.c` (viewport, mapping and RTL); Win16 control painting consumers |
| `text_metrics` | `graphics/text_metrics.c` (alphabet average and invalid DC); Win16 menu and list consumers |
| `menu_metrics` | `menu/menu_metrics.c` (native font margin and GDI font restoration); Win16 owner-draw menu consumer |
| `bitmap` | `graphics/bitmap.c`; `tests/lib-win32-style/application/adapters/surface.c`; Win2000 caption button and UWP surface consumers |
| `bitmap_font` | `graphics/bitmap_font.c` (ANSI FNT v2 header fields and offsets); Win16 font realization tests |
| `global_window` | `lifecycle/global_window.c` (style lifecycle and descendant visitation); `tests/ntw16s-dll/global_style_bridge.c` (two-style window and child transitions) |
| `menu_input` | `menu/menu_input.c` |
| `control_vcl_identity` | `control/control_vcl_identity.c` (including MDI and CoolBar parent, owner and attachment checks); Win16 VCL grid, group, toolbar, MDI, CoolBar and bit-button product fixtures |
| `control_native_witness` | `control/control_native_witness.c` (ANSI and Unicode procedure and module identity); Win16 attachment native forwarding |
| `control_list` | `control/control_list.c` (native top, selection, caret and change detection); `tests/ntw16s-dll/list_view_runtime.c` (Win16 list message consumer) |
| `control_native_combo` | `control/control_native_combo.c` (native combo/date identity and invalidation) |
| `control_status` | `control/control_status.c` (native v5/v6 identity and invalidation) |
| `menu_snapshot` | `menu/menu_snapshot.c`; Win16 and Win2000 system-menu consumers |
| `menu_owner` | `menu/menu_owner.c`; `tests/ntw16s-dll/vcl_popup_scope.c` (product integration) |
| `messages` | `window/messages.c` (native order/WVR, error preservation, removal in callback, print and font protocols); ordinary Win3.x/Win2000 consumers |
| `install` | `lifecycle/install.c` (publication, cancellation, foreign subclass, setup failure rollback, destruction/replacement identity and lifetime); actual two-style window and control fixtures; explicit failed setup retaining ownership |
| `font_pair` | `graphics/font_pair.c` (real GDI ownership, partial failure retains old pair, stock lease and disposal); ordinary and modern frame consumers |
| `native_frame` | `window/native_frame.c` (observed/default DWM, child exclusion, pending and keep-outer restoration, coalesced deferred refresh); actual Win3.x/Win2000 frame consumers |
| `caption_button` | `window/caption_button.c` (capture ownership, release/cancel synchronous reentry, destroyed target); Win3.x `control_link.c` and Win2000 `instance_lifetime.c` (three actual caption commands, busy removal and cancel) |
| `frame` resize restoration | `window/frame_restore.c`; Win16 and Win2000 frame lifecycle tests |
| `scroll_state` | `tests/ntw16s-dll/win16_style_scroll_geometry.c` (null and disabled control) |
| `print_ranges` | `dialog/print_ranges.c` |
| `classic_file_contract`, `classic_shell_item` | `dialog/classic_file.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c` |
| `theme_state` | `lifecycle/theme_state.c`; `lifecycle/theme_exception.c` (native unwind and reuse, static, shared DLL, comctl v6); `lifecycle/theme_shared.c` (shared DLL and Win16 integration); `tests/ntw16s-dll/theme_stage.c` |
| `theme_track` | `lifecycle/theme_track.c` (handle aliases, concurrent open/close, capacity and cancellation) |
| `file_events` | `dialog/file_events.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c` |
| `modern_file_dialog` | `dialog/modern_file_dialog.c`; `dialog/modern_file_depth.c` (thread-local nesting and failure cleanup); `tests/lib-win32-style/application/dialog/classic_dialog_link.c` (argument validation) |
| `classic_shell_folder` | `tests/lib-win32-style/application/dialog/classic_dialog_link.c` (product linkage; interactive browser not covered) |
| `dialog_primary`, `dialog_v2`, `dialog_customize`, `dialog_ole_window` | `dialog/dialog_adapter.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c`; `tests/lib-win32-style/application/dialog/classic_dialog_chrome.c` |
| `observation` | `dialog/observation.c` (real current-thread hooks, tracked/untracked nesting, inactive barrier, prepare/hook failure, native unwind and post-end silence) |
| `dialog_combo` | `control/dialog_combo.c` (owned native combo/list/edit, pending identity, nested install, unsupported/main failure, advisory child failures and destruction) |
| `classic_file_controller` | `dialog/classic_file_controller.c` (same production A/W facades, real owned thread hooks, native result/LastError, custom/multi passthrough, nested state, scope/hook failure, actual native unwind and observer restoration) |
| `classic_com_controller` | `dialog/classic_com_controller.c` (actual native COM objects through captured production facades: three activation entries, identity/refs, scopes, inactive/experimental options and registration failure); product COM identity consumers |
| `classic_com_show`, `classic_browse`, `classic_file_live` | `dialog/classic_bridge.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c`; `tests/lib-win32-style/application/dialog/classic_dialog_chrome.c` |
| `classic_file_helpers`, `classic_hidden_files` | `dialog/classic_helpers.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c` |
| `classic_dialog_controls` | `tests/lib-win32-style/application/dialog/classic_dialog_chrome.c` |
| `classic_file_hook`, `classic_com_hook` | `tests/lib-win32-style/application/dialog/classic_dialog_chrome.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c` |
| `classic_dialog_proxy`, `classic_dialog_proxy_dispatch` | `dialog/classic_dialog_proxy.c`; `tests/lib-win32-style/application/dialog/classic_dialog_chrome.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c`; `dialog/dialog_adapter.c` |
| `classic_dialog_events`, `classic_dialog_state` | `tests/lib-win32-style/application/dialog/classic_dialog_link.c` |
| `message_observer` | `dialog/message_observer.c` (thread hook install and removal) |
| `dialog_hook` | `dialog/dialog_hook.c` (return, error, rollback and native unwind); `tests/integration/automatic_target.c` and `tests/ntw2ks-dll/frame_target.c` (Win16/Win2000 actual dialog APIs) |
| `control_protocol` | `control/control_protocol.c`; `tests/ntw16s-dll/control_protocol_owner.c` (direct worker); control integration probes |
| `control_protocol` runtime class | `control/control_runtime.c` (class, owner module, case and missing module) |
| `control_scope` | `control/control_scope.c`; `tests/lib-win32-style/appearance/win3x/control/control_scope.c` |
| `frame` XAML child detection | `window/frame_xaml.c`; Win16 frame integration |
| `control_redraw` | `control/control_redraw.c`; Win16 list/combo/edit integration |
| `print_resources` | `dialog/print_resources.c`; Win16 PrintDlgEx integration |
| `print_extended` | `dialog/print_extended.c`; `tests/ntw16s-dll/print_dialog_target.c` (product integration) |
| `classic_print_contract` | `dialog/classic_print_contract.c`; `tests/lib-win32-style/application/dialog/classic_dialog_link.c` |

| `menu/popup` | `menu/popup.c` standalone and actual shared runtime: nested/non-top scopes, foreign thread rejection, owned native popup, accept/paint callback teardown and deferred disposal; profile shadow/PRINT through existing menu integration tests |

| `menu/owner` | `menu/owner.c`: real subclass/native forwarding with adapted/native item policies, failed inner entry, depth restore, application mnemonic priority and stop during initialization/entry; ordinary frame, modern menu actions and Win2000 real popup integration |

| `menu/tracking`, `tracking_input` | `menu/tracking.c` static/shared: PM_REMOVE, one nested dispatch, mixed hook sources, non-top end, foreign thread rejection, callback end and actual owned native popup; two-profile native menu regression |
| `menu/bar_state` | `menu/owner.c` actual EDIT focus return/non-stealing, stopped selection and step semantics; native Win2000 F10/key/hover/nested and Win3x cancellation matrices |

| `menu/bar_snapshot` | `menu/bar_snapshot.c`: custom/native geometry, copied text, original position/data, owner measurement/drawing, DC isolation, mutation failure cleanup; two appearance product menu suites |
| `menu/bar_controller` | `menu/bar_controller.c`: both navigation policies, command/mnemonic, focus/cancel and reentrant initialization stop; Win3x cancel/NEXTMENU and Win2000 native input suites |

| `control/control_state` | `control/control_state.c`: font lease/source/native forwarding, failure fallback, query teardown, retired setter successor, tab commit/failure/reentrant cleanup; Win3x control-link/list-paint and product font/control suites |

| `control/combo` | `control/combo.c`: actual native EDIT binding/source/paint-font messages, 32/33 lease boundary, duplicate bind, child identity replacement and setter removal/successor; existing VCL/VB combo product fixtures |

| `control/basic` | `control/basic.c`: native bitmap/borrowed DC isolation and ownership, native-before-overlay return/ordering, detach during native callback; real button/static/radio production present suite |

| `control/retire` | `control/retire.c`: recursive removal, destruction during input completion, failed detach retaining ownership, terminal child failure and disposal-before-refresh |
| `control/scroll_resources` | `control/scroll_resources.c`: real caret resize/hide/foreign ownership, timer release and borrowed capture restore |
| `control/control_state` tabstops | `control/tabstops.c`: proportional-font set-time conversion, native forwarding, invalid input preserving state and cleanup; no appearance linkage |

| `control/list` | `control/list.c`: real LISTBOX native state/forwarding, tabstop ownership, client PRINT coordinates/DC restore and reentrant retirement; production tabs/lifetime/control-present |

| `control/edit` | `control/edit.c`: real EDIT text/selection/undo, native redraw ownership, print return/DC isolation, nonclient and callback retirement; actual edit clipboard/font/control-present suites |
| `control/combo` messages | `control/combo.c`: native COMBOBOX item/selection and redraw state, borrowed PRINTCLIENT renderer; binding tests remain independent |

| `control/scroll_track` | `control/scroll_track.c`: independent layouts and two hit/repeat policies, drag/cancel/native notice contract; Win3x geometry fidelity remains product integration |
| `control/scroll_controller`, `scroll_geometry` | `control/scroll_controller.c`: real SCROLLBAR notices, owned timer/capture, epoch cancellation and callback destruction; product geometry/VCL/native input suites |

| `control/scroll_managed` | `control/scroll_managed.c`: native result/error/redraw forwarding, role qualification, report/track position, setter replacement and render-time retirement lifetime; product native/VCL scroll suites |

| `control/scroll_message` | `control/scroll_message.c`: native return and SBM rewrite, full track position, disabled native input, frame PRINT DC/retirement and sizebox; actual VCL scroll/control-present |
| `control/list_snapshot` | `control/list_snapshot.c`: native single/multicolumn rows/selection/text, failed read, DC isolation and read-time removal; actual list rendering suites |
| `control/scroll_surface` | `control/scroll_surface.c`: native client/embedded bounds, window DC origins, hidden/invalid rejection; actual scroll raster/present suites |

| `control/button_snapshot` | `control/button_snapshot.c`: native BUTTON/STATIC role/state/check/text, borrowed font/icon, query-time retirement and text cleanup; actual basic controls rendering |
| `control/combo_snapshot` | `control/combo_snapshot.c`: native dropdown/dropdownlist/simple info/text/font/children, drop state, native info failure and font-query removal cleanup; actual VCL combo and present suites |
| `control/combo` setup | `control/combo.c`: parent/child publication, query/install/edit failure, install/edit retirement, failed rollback retaining identity and successful cleanup; actual combo installation |

S10 `adapters/win32/checklist.c` → `tests/lib-win32-style/application/adapters/win32/checklist.c` 与无风格 `checklist_fixture.c`：真实native LISTBOX、runtime模块/DC身份、可配置区域上限、借用位图、两种启用/选中偏移、嵌套排除及查询中disable/style/reparent。产品组合仍由现有VCL/VB checklist夹具验证原资源绘制及回退，不把公共回调测试称像素验收。

S10 `control/control_native_combo.c` → 同名测试：公共native witness对象与instance、首次paint观察/提交、失败后不重复接入、查询中detached、entry LastError保留，以及原有color/date/edit/dialog身份关系。实际按钮区域及应用原生内容仍由产品原生color/date夹具复核。

S10 `control/native.c`、`header_snapshot.c`、`progress_snapshot.c` → 对应同名纯C测试：实际COMCTL消息返回/支持查询时机、header命中/pressed、epoch、progress借用print DC与绘中退场；原生Unicode长文本/格式mask/foreign pointer/失效清理及progress值/色彩/失效。原`header_paint_owner.c`同时编译实际公共header_snapshot和实际profile painter，保留16项分配失败/重入/销毁/DC断言，不能因迁移丢失OOM注入。

- `control/tab_snapshot.c/h` → `control/tab_snapshot.c`：无年代依赖的tab原生读/双遍顺序、长Unicode、page/child裁剪、借用图片、epoch/退场/查询失败及DC；实际Win3x消费由产品tab v5/v6夹具覆盖。

- `control/notify.c/h` → `control/notify.c`：原生通知返回/私有绘制阶段、父子绑定所有权、失效引用退场、LastError、原生派发作用域与异常展开；实际runtime-track/toolbar由对应产品夹具覆盖。

- `control/native_overlay.c/h`及`control_native_combo`按钮快照 → `control/native_overlay.c`：颜色/日期/pressed原生采集，print借用DC/原生裁剪保存恢复、paint DC、查询退场、精确region释放和LastError；实际模块异常/重入由color/date产品夹具覆盖。

- `control/runtime.c/h` → `control/runtime.c`：真实原生protocol安装且无年代样式、一次尝试/重入、apply失败remove/release、query/apply失效所有权、parent、LastError和幂等dispose；五种角色的实际Win3x安装由对应模块回归覆盖。

- `control/runtime`的edit/link/combo → `control/runtime.c`及现有edit/combo/link产品模块：原生edit协议、前后角色、一次尝试/重入、失败/失效所有权与fresh-link-witness。
- `control/report.c/h` → `control/report.c`：无风格native list/tree通知绑定、owner子类、runtime提交/失败/失效所有权、fresh-witness与LastError；实际list/tree由产品模块覆盖。

- `control/edge.c/h` → `control/edge.c`：延迟parent绑定/先发布edge、data绘后重查与user原顺序、apply失败/失效、重入、grid原生scroll和查询退场、LastError；真实VB/grid由产品夹具覆盖。

S10 runtime rich/frame: `adapters/win32/runtime_identity.{c,h}` → `adapters/win32/runtime_identity.c`; independent native identity/state/settle/retire without a renderer. Product rich/box/controlbar/group/panel models retain the actual rendering, native forwarding and reparent regressions.

S10 native group/panel paint association: `adapters/win32/frame_paint.{c,h}` → `adapters/win32/frame_paint.c`; public native pairing, caller/DC identity and complete-state commit use a no-style observer. Actual group/radio/panel/default/aware rendering remains covered by existing product models; private vcl_frame_policy.c/h removed.

S10 native forwarded-paint witnesses: `adapters/win32/runtime_observer.{c,h}` → `adapters/win32/runtime_observer.c`; complete public runtime/observation,13 native roles, ANSI DB variants, rich/box/static/cool, ownership/detach/attempt/scope rejection without rendering. Product native models retain actual protocol, drawing, reentry and unwind checks.

S10 runtime roles/related instance leases: `adapters/win32/runtime_roles.{c,h}` → `adapters/win32/runtime_roles.c`; real native instance attach/retire, transferred final-release ownership and popup→MDI release order, identity replacement, inplace/nav and terminal scroll guards without renderer. Product MDI/lookup/bit/static/navigation models verify actual composition.

S10 spin buffered native protocol: `control/spin.{c,h}` → `control/spin.c`; native clone cap/borrow, print DC source, half invalidation, invalid rect, selected/deleted bitmap and nested DC cleanup without style. Product modern/legacy/spinedit models retain actual two-half composition, application glyph/pixel preservation, failure/theme/clip and unwind assertions.

S10 toolbar text/native items: `control/toolbar_text.{c,h}` → `control/toolbar_text.c`; real native metadata/text/rect/find, policy rejection before text query,512 text bound and1024/1025 native item boundary without style. Existing VCL toolbar actual model covers64/65,duplicate IDs,images,callbacks,app skip,clip and native retention.

`control/toolbar_text_bounds.c` retains the original actual v5/v6 production-reader700-character canary and510/511 boundary cases; it moved with the reader from tests/ntw16s-dll, removes private source includes/fake font/protocol stubs, and both CTest/package entries explicitly bypass private desktop creation.

S10 month native scope: `control/month.{c,h}` → `control/month.c`; real v6 COMCTL superclass queries verify geometric success,current/detach rejection,partial header commit,nested suppression and native unwind/later query. Selection pairing/caller/native theme and shared title-fit use no style renderer. Existing month_model retains all actual Win3x font/pixel/fallback checks.

S10 status_paint.c/h：control/control_status.c真实v5/v6消息/DC、final pane text/icon、epoch/detached、原生调用者、主题配对/嵌套barrier/复制文本/512界限；双宽产品status_module/simple/pane/font原绘制与字体借用异常测试对应旧绘图入口。公共scope持完整快照，Win3x status_font仅提供已有限定文字资格/实际字体与token。

S10 application/lifecycle/dispatch.c/h：lifecycle/dispatch.c实际窗口与subclass，默认DefSubclassProc/可选invoke、嵌套顺序、entry/return LastError、一次finish、原生异常及后续调用。生产attachment的static/cool/box/lookup/spin/color/status/bit八个路径共用completed/native-unwind事务；各资源end保持原顺序，实际模型同时覆盖正常与异常绘制。

S10 control/guard.c/h：control/guard.c实际BUTTON/subclass转发先行，角色失效/owner-draw/不支持choice三种退役、不同失败回退、原始字体提交、转发内detach/destroy及terminal/release。lifecycle/instance.c新增owned lease真实同HWND替换/错误subclass data/租约跨destroy后最后归还；产品updown安装直接用公共instance租约，原updown_owner自建owner回调仍保留。

同组control_state.c的message_finish对应原control_lifetime操作：公开state、真实替换subclass后继字体及无后继回退、native-paint深度/字体lease/最后instance释放；Win3x scrollbars_paint仅renderer palette组合并入scroll_native，无第二份实现。

S10 adapters/win32/runtime_native.c/h：对应runtime_native.c真实STATIC superclass+CallWindowProc调用、原生见证/有效嵌套/不匹配barrier、完整instance退役/current window、原生异常后manager恢复与后续调用；runtime_observer.c保留13-role见证顺序。产品CallWindowProcW采用同一公共begin/end，private static scope/suspend/end已删，Win3x仅保留实际文字资格/font provider。

同组lifecycle/install的setup_finish消费既有保活引用、成功/失败移除/已退役/失败移除保留/target销毁及旧LastError副作用，实际frame-install原生矩阵验证；profile track/toolbar/grid直接接入，旧finish_install删除，首次publication仍用既有install事务，不复制安装器。

S10 notify.c/h的track_message与track_calls：control/track_message.c真实v5/v6 TRACKBAR、原生子类先转发/缓存刷新、SetParent后owned unbind不重绑、独立notify owner、native内detach/destroy与最后引用；既有notify.c保留私有通知阶段、结果/LastError与native-unwind作用域恢复，只替换为完整manager。产品私有track分派已删除，实际VCL track/native track/toolbar验证原绘制与重入退役。

S10 rebar native probe: `control/rebar.{c,h}` -> `control/rebar_probe.c` (v5/v6): native REBAR procedure, copied values, ordered queries, invalidation, DC qualification, caller style mask and post-policy geometry.

S10 owner-draw overlay: `control/basic.{c,h}` -> `control/owner_message.c`: actual subclass/native-first, input, owned paint DC, borrowed print DC, detach/destroy and renderer reentry. Product CoolBar/VB file/combo models retain policy and pixel checks.

S10 native updown registry: `control/updown.{c,h}` -> `control/updown.c`: independent Win32 windows/DCs,32 slots/overflow/deletion, native module/thread identity, false-with-window scope barrier, own lease/ref/retire and frame/theme decoding; no Win3x dependency. Existing product `tests/ntw16s-dll/updown_owner.c` retains actual original glyph renderer, deleted-DC fallback and165 handle cleanup; actual spin/track/toolbar models cover hook consumers.

S10 native/report text scopes: `adapters/win32/runtime_text.{c,h}` -> `adapters/win32/runtime_text.c`: native/window/DC and text/report nested managers, opaque borrowed context, callback ordering, cancellation, module/thread identity, actual native and nested exception cleanup via dispatch; no Win3x font dependency. `lifecycle/theme_track.{c,h}` -> existing `lifecycle/theme_track.c` now owns original6 bounded native-class assertions; kinds do not choose styles. Profile font/glyph acquisition keeps actual VB font/consumer tests.

S10 aggregate binding ownership: `control/bindings.{c,h}`, `control_state:font_read`, `lifecycle/instance:preflight` -> `control/bindings.c`: actual Windows subclass/font query cancellation and reentrant zero-data installation, foreign thread rejection, native setup/removal, owned parent notifications/updown, ordered ordinary/terminal child/theme/font/redraw release and timer cancellation; no Win3x. Existing combo/setup/retire/instance and product lifetime/presentation/track/spin/combo fixtures retain actual rendering/ownership coverage.

S10 paint manager: `adapters/win32/paint_dispatch.{c,h}` -> `adapters/win32/paint_dispatch.c`: eight real native subclass paths, ordinary/nested and native exception paths with exactly-one borrowed call release, static barrier, owner-draw payload, actual region/heap disposal, error and head restoration. Actual product CoolBar/box/panel/status/month/spin/color/bit/lookup models cover renderer-qualified observations. Native/default remains the original unprotected passthrough, not a new recovery claim.

`adapters/win32/message.c/.h` uses the same native `adapters/win32/paint_dispatch.c` test: full message retain/guard/prepare, status/spin prior heads, ordinary and nested native unwind/last reference. Actual VB/VCL role models cover ordered post-native settlement; these test files consume production message implementation, not a second coordinator. `runtime_text.c` covers report prepare native DC/file-owner/cancel and existing report/direct module callers.

`adapters/win32/message.c` parent notification entry is also covered by `adapters/win32/report_notify.c`: native result/error, three custom-draw stages/skip, file/button scopes, wrong payload/reparent, retired instance and parent subclass cleanup. Product VB/VCL probes retain their actual font/drawing assertions.

`adapters/win32/attachment.c/.h` is covered by `adapters/win32/runtime_attachment.c`: native published public root,16 role states/policy, foreign thread, failed detach ownership, reentrant ordinary removal, terminal first-error/unowned node and last release. Existing actual VB/VCL/lifetime/present probes consume the same product adapter and public root.

`control/message.c/.h` is covered by `control/message.c`: real native subclass forwarding, guard/prepare short circuits, owned temporary font leases, nested messages, rewritten native arguments and detach/last release during dispatch. Profile `control_link.c` retains original font-policy assertions through the full public message object; actual VB/VCL/controls/present models cover native family observation and font/layout behavior.

`adapters/win32/attachment` native seed path is covered by existing `runtime_attachment.c`: caller-selected family metadata, owner policy exact phase, MDI/popup held references, unpublished failure cleanup and target last release. Product classification still supplies actual supported families; tests do not register new runtime classes or enlarge target scope.

`adapters/win32/attachment` install settlement uses the same native test: 24 LISTBOX/EDIT/COMBOBOX/STATIC combinations cover theme target, file-owner exemption, list/edit scrollbar setup, original error, setup failure, cancellation and failed removal ownership. Product render selection remains in appearance; shared root owns the transaction.

`adapters/win32/attachment.c` / `style_runtime_attachment_current` is exercised in the existing `runtime_attachment.c` test: six witnessed-role rejection paths, unwitnessed candidates, native tab class/parent/RTL, exact data/user policy phases and short circuit, root replacement during policy, retirement and no-root target. Product controls call the same complete operation; no era painter or process hook is linked by the independent test.

`adapters/win32/attachment.c` late control install/remove callbacks are covered by `runtime_attachment.c`: 12 native EDIT/other-role cases for renderer/theme/scrollbar failure, cancellation before and during theme setup, link/combo/spinedit/single-line exclusions, renderer removal failure and exact theme-target clearing. Original outer runtime settlement still owns protocol/rollback/entry LastError.

`adapters/win32/attachment` publication uses the existing `runtime_attachment.c` native seed test: successful publication retains the initial call; duplicate native subclass rejection releases both related borrows before raw owner discard, preserves ERROR_ALREADY_EXISTS despite discard changing LastError, and does not invoke ordinary dispose. Product initialization calls this same transaction with free.

`adapters/win32/runtime_roles` guard additionally validates caller-selected owner-draw parent relationships in the existing native `runtime_roles.c`: unchanged/changed parent, non-owner, no recorded parent, destroy/terminal bypass, already-retired and failed removal continuing to later runtime-role checks. Win3x keeps supported drawing policy and calls this existing common guard; no parallel guard manager is introduced.

`application/control/bindings` setup owns the existing optional list-theme capture/enable/cancel transaction as well as release. Existing `control/bindings.c` native12-case table verifies exact caller phase, preexisting lease, capture/enable failure, retirement inside either callback, final cancellation precedence, non-list exclusion, unchanged held call, owned-flag publication before enable and release only by the captured owner. Actual grid/directory fixtures consume the same setup; default ordinary options remain unchanged.

## Modern host ownership (S12)

| Production source | Corresponding tests and real consumer |
| --- | --- |
| `adapters/frame.c/h` | `adapters/frame.c`: independent C provider without Win3x rendering, 256 copied actions, wrong thread/invalid/duplicate input, original error, reentrant remove/title destruction, activation/hide/repeat/final disposal; Win3x modern frame and product hook/attachment use this same root |
| `adapters/host.c/h` | `adapters/host.c`, `adapters/host_start.c`: subclass ownership/failed removal, deferred disposal, eight start outcomes and native synchronous retirement; common frame reuses this installer |
| `adapters/input.c/h` | `adapters/input.c`: complete common frame pressed/hot/capture/drag/cancel and detachment during refresh; Win3x frame fixture exercises the actual installed surface |
| `adapters/surface.c/h` | `adapters/surface.c`, `adapters/surface_present.c`: bitmap ownership/reuse, hidden/maximized/native bounds, alpha/region transfer/failure and refresh reentry; common frame owns bitmap/updating/menu anchor |
| `adapters/repaint.c/h` | `adapters/repaint.c`: coalesced timer, deadline and cancellation during callbacks; ordinary Win3x frame retains its complete scheduler |
| `adapters/uwp/native.c/h`, `target.c/h` | `adapters/uwp/native.c`, `target.c`: title/auxiliary identity, invalid UIA/thread, target/frame/core association and destruction; actual cross-process OS application coverage is recorded separately |

The existing Win3x `modern_frame.c`, `_hidden.c` and `_layout.c` fixtures cover profile paint/layout, actual menu restrictions/collisions/partial insertion rollback, nested-title destruction and peer isolation. Call `modern_frame` with first argument `--private-worker` to bypass its legacy private-desktop wrapper on the approved current desktop; no private desktop is created/switched in that mode. These tests do not establish coverage of every UWP/WinUI application or physical input.

Actual product composition: `tests/ntw16s-dll/window_thread.c --composed` selects a self-created composed-class/NOREDIRECTION window, applies through the real product thread hook and verifies the shared surface disappears on product removal. It is not an actual Chrome process or a claim about every composed application. Ordinary terminal tests remain separate from modern owner destruction.
