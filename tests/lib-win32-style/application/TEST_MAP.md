# Shared style test map

| Source family | Native test adapter |
| --- | --- |
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
