# Shared style test map

| Source family | Native test adapter |
| --- | --- |
| `present` | `present.c`; `present_fault.c`; `present_print.c`; `tests/ntw16s-dll/win16_style_control_present.c` |
| `global_window` | `global_window.c` |
| `menu_input` | `menu_input.c` |
| `menu_snapshot` | `menu_snapshot.c`; Win16 and Win2000 system-menu consumers |
| `print_ranges` | `print_ranges.c` |
| `classic_file_contract`, `classic_shell_item` | `classic_file.c`; `tests/lib-style/win16-dialog/dialog_link.c` |
| `theme_state` | `theme_state.c`; `tests/ntw16s-dll/theme_stage.c` |
| `theme_track` | `theme_track.c` (handle aliases, concurrent open/close, capacity and cancellation) |
| `file_events` | `file_events.c`; `tests/lib-style/win16-dialog/dialog_link.c` |
| `modern_file_dialog` | `modern_file_dialog.c`; `tests/lib-style/win16-dialog/dialog_link.c` (argument validation) |
| `classic_shell_folder` | `tests/lib-style/win16-dialog/dialog_link.c` (product linkage; interactive browser not covered) |
| `dialog_primary`, `dialog_v2`, `dialog_customize`, `dialog_ole_window` | `dialog_adapter.c`; `tests/lib-style/win16-dialog/dialog_link.c`; `tests/lib-style/win16-dialog/dialog_chrome.c` |
| `classic_com_show`, `classic_browse`, `classic_file_live` | `classic_bridge.c`; `tests/lib-style/win16-dialog/dialog_link.c`; `tests/lib-style/win16-dialog/dialog_chrome.c` |
| `classic_file_helpers`, `classic_hidden_files` | `classic_helpers.c`; `tests/lib-style/win16-dialog/dialog_link.c` |
| `control_protocol` | `control_protocol.c`; `tests/ntw16s-dll/control_protocol_owner.c` (direct worker); control integration probes |
| `control_scope` | `control_scope.c`; `tests/lib-style/win16-control/control_scope.c` |
| `frame` XAML child detection | `frame_xaml.c`; Win16 frame integration |
| `control_redraw` | `control_redraw.c`; Win16 list/combo/edit integration |
| `print_resources` | `print_resources.c`; Win16 PrintDlgEx integration |
