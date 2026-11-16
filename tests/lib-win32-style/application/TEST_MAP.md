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
