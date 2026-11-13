# Shared style test map

| Source family | Native test adapter |
| --- | --- |
| `present` | `present.c`; `present_fault.c`; `present_print.c`; `tests/ntw16s-dll/win16_style_control_present.c` |
| `clip` | `clip.c` (full and nonrectangular clipping, DC mapping restoration); Win16 spin paint consumer |
| `dc_geometry` | `dc_geometry.c` (viewport, mapping and RTL); Win16 control painting consumers |
| `text_metrics` | `text_metrics.c` (alphabet average and invalid DC); Win16 menu and list consumers |
| `menu_metrics` | `menu_metrics.c` (native font margin and GDI font restoration); Win16 owner-draw menu consumer |
| `bitmap` | `bitmap.c`; `tests/lib-style/uwp/surface.c`; Win2000 caption button and UWP surface consumers |
| `bitmap_font` | `bitmap_font.c` (ANSI FNT v2 header fields and offsets); Win16 font realization tests |
| `global_window` | `global_window.c` (style lifecycle and descendant visitation); `tests/ntw16s-dll/global_style_bridge.c` (two-style window and child transitions) |
| `menu_input` | `menu_input.c` |
| `control_vcl_identity` | `control_vcl_identity.c`; Win16 VCL grid, group, toolbar and bit-button product fixtures |
| `control_native_witness` | `control_native_witness.c` (ANSI and Unicode procedure and module identity); Win16 attachment native forwarding |
| `control_list` | `control_list.c` (native top, selection, caret and change detection); `tests/ntw16s-dll/list_view_runtime.c` (Win16 list message consumer) |
| `control_native_combo` | `control_native_combo.c` (native combo/date identity and invalidation) |
| `control_status` | `control_status.c` (native v5/v6 identity and invalidation) |
| `menu_snapshot` | `menu_snapshot.c`; Win16 and Win2000 system-menu consumers |
| `menu_owner` | `menu_owner.c`; `tests/ntw16s-dll/vcl_popup_scope.c` (product integration) |
| `frame` resize restoration | `frame_restore.c`; Win16 and Win2000 frame lifecycle tests |
| `scroll_state` | `tests/ntw16s-dll/win16_style_scroll_geometry.c` (null and disabled control) |
| `print_ranges` | `print_ranges.c` |
| `classic_file_contract`, `classic_shell_item` | `classic_file.c`; `tests/lib-style/common/classic_dialog_link.c` |
| `theme_state` | `theme_state.c`; `theme_exception.c` (native unwind and reuse, static, shared DLL, comctl v6); `theme_shared.c` (shared DLL and Win16 integration); `tests/ntw16s-dll/theme_stage.c` |
| `theme_track` | `theme_track.c` (handle aliases, concurrent open/close, capacity and cancellation) |
| `file_events` | `file_events.c`; `tests/lib-style/common/classic_dialog_link.c` |
| `modern_file_dialog` | `modern_file_dialog.c`; `modern_file_depth.c` (thread-local nesting and failure cleanup); `tests/lib-style/common/classic_dialog_link.c` (argument validation) |
| `classic_shell_folder` | `tests/lib-style/common/classic_dialog_link.c` (product linkage; interactive browser not covered) |
| `dialog_primary`, `dialog_v2`, `dialog_customize`, `dialog_ole_window` | `dialog_adapter.c`; `tests/lib-style/common/classic_dialog_link.c`; `tests/lib-style/common/classic_dialog_chrome.c` |
| `classic_com_show`, `classic_browse`, `classic_file_live` | `classic_bridge.c`; `tests/lib-style/common/classic_dialog_link.c`; `tests/lib-style/common/classic_dialog_chrome.c` |
| `classic_file_helpers`, `classic_hidden_files` | `classic_helpers.c`; `tests/lib-style/common/classic_dialog_link.c` |
| `classic_dialog_controls` | `tests/lib-style/common/classic_dialog_chrome.c` |
| `classic_file_hook`, `classic_com_hook` | `tests/lib-style/common/classic_dialog_chrome.c`; `tests/lib-style/common/classic_dialog_link.c` |
| `classic_dialog_proxy`, `classic_dialog_proxy_dispatch` | `classic_dialog_proxy.c`; `tests/lib-style/common/classic_dialog_chrome.c`; `tests/lib-style/common/classic_dialog_link.c`; `dialog_adapter.c` |
| `classic_dialog_events`, `classic_dialog_state` | `tests/lib-style/common/classic_dialog_link.c` |
| `message_observer` | `message_observer.c` (thread hook install and removal) |
| `dialog_hook` | `dialog_hook.c` (return, error, rollback and native unwind); `tests/integration/automatic_target.c` and `tests/ntw2ks-dll/frame_target.c` (Win16/Win2000 actual dialog APIs) |
| `control_protocol` | `control_protocol.c`; `tests/ntw16s-dll/control_protocol_owner.c` (direct worker); control integration probes |
| `control_protocol` runtime class | `control_runtime.c` (class, owner module, case and missing module) |
| `control_scope` | `control_scope.c`; `tests/lib-style/win16-control/control_scope.c` |
| `frame` XAML child detection | `frame_xaml.c`; Win16 frame integration |
| `control_redraw` | `control_redraw.c`; Win16 list/combo/edit integration |
| `print_resources` | `print_resources.c`; Win16 PrintDlgEx integration |
| `print_extended` | `print_extended.c`; `tests/ntw16s-dll/print_dialog_target.c` (product integration) |
| `classic_print_contract` | `classic_print_contract.c`; `tests/lib-style/common/classic_dialog_link.c` |
