# M0 T26 S1: window and UWP source index

This indexes the current C source files in both window style libraries, the
Win16 foundation and UWP libraries. It is a dependency/test navigation aid,
not a completed function-by-function migration audit or visual verification.
Direct includes match the header basename, so names such as `frame.h` can
produce ambiguous matches. A dash indicates no literal direct include found;
indirect calls and CMake linkage are not represented.

Win16 frame lifecycle combines common subclass/restore operations with Win16
palette, typography, geometry and attachment state. The current Win2000 frame
contains its own layout, DWM, menus and window lifecycle; shared `common/dwm`,
`common/frame` and `common/menu_input` are used but do not prove full dedup.
`frame_dispatch` includes XAML repaint scheduling bound to Win16 frame state;
the shared UWP `native`, `target` and `surface` libraries cover separate
mechanisms and do not imply Win2000 UWP support. Prior frame-UWP lifecycle
probe returned access denied (5); real UWP and WinUI visual tests remain open.

| Component/source | Paired header | Literal production includes | Literal test includes |
| --- | --- | --- | --- |
| `win16-window/frame_dispatch.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window/frame_lifecycle.c` | — |
| `win16-window/frame_font.c` | — | — | — |
| `win16-window/frame_input.c` | yes | `src/ntw16s-dll/win16_style.c` | — |
| `win16-window/frame_layout.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window/frame_dispatch.c`, `src/lib-style/win16-window/frame_lifecycle.c`, `src/lib-style/win16-window/frame_message.c`, `src/lib-style/win16-window-uwp/frame.c` (+1 more) | `tests/run16s-exe/uwp.c`, `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-window-uwp/frame.c`, `tests/lib-style/win16-window-uwp/frame_layout.c` |
| `win16-window/frame_lifecycle.c` | yes | `src/ntw16s-dll/win16_style.c` | — |
| `win16-window/frame_message.c` | yes | `src/ntw16s-dll/win16_style.c` | — |
| `win16-window/frame_paint.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window/frame_view.c`, `src/lib-style/win16-window/menu_scope.c` | — |
| `win16-window/frame_state.c` | yes | `src/ntw16s-dll/win16_style.c` | `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-window/window_link.c` |
| `win16-window/frame_view.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window/frame_message.c`, `src/lib-style/win16-window-uwp/frame.c` | `tests/lib-style/win16-control/control_link.c` |
| `win16-window/menu_bar_paint.c` | — | — | — |
| `win16-window/menu_bar.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window/frame_input.c`, `src/lib-style/win16-window/frame_layout.c`, `src/lib-style/win16-window/frame_view.c`, `src/lib-style/win16-window/menu_bar_paint.c` (+1 more) | `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-window/window_link.c` |
| `win16-window/menu_items.c` | yes | `src/ntw16s-dll/win16_style.c` | — |
| `win16-window/menu_iteration.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/ntw16s-dll/injection/window_titles.c` | — |
| `win16-window/menu_message.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window-uwp/frame.c` | — |
| `win16-window/menu_paint.c` | yes | `src/lib-style/win16-window/menu_scope.c` | `tests/ntw16s-dll/win16_style_dc_failure.c` |
| `win16-window/menu_scope.c` | yes | `src/ntw16s-dll/win16_style.c` | — |
| `win16-window-uwp/frame.c` | yes | `src/ntw16s-dll/attachment.c`, `src/ntw2ks-dll/controls.c`, `src/ntw2ks-dll/window.c`, `src/ntw16s-dll/uwp/hook.c`, `src/lib-style/win16-window/frame_layout.c` (+2 more) | `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-window-uwp/frame.c` |
| `win16-window-uwp/frame_layout.c` | yes | `src/ntw16s-dll/win16_style.c`, `src/lib-style/win16-window/frame_dispatch.c`, `src/lib-style/win16-window/frame_layout.c`, `src/lib-style/win16-window/frame_lifecycle.c`, `src/lib-style/win16-window/frame_message.c` (+1 more) | `tests/run16s-exe/uwp.c`, `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-window-uwp/frame.c`, `tests/lib-style/win16-window-uwp/frame_layout.c` |
| `win16-common/font.c` | yes | `src/ntw16s-dll/controls.c`, `src/ntw16s-dll/win16_style.c`, `src/ntw16s-dll/injection/window_titles.c`, `src/lib-style/win16-control/combo_text.c`, `src/lib-style/win16-control/control_guard.c` (+13 more) | `tests/ntw16s-dll/status_font_isolation.c`, `tests/ntw16s-dll/vb_font.c`, `tests/ntw16s-dll/win16_style_font.c`, `tests/ntw16s-dll/win16_style_list_tabs.c`, `tests/lib-style/win16-common/common_link.c` (+1 more) |
| `win16-common/layout.c` | yes | `src/ntw16s-dll/win16_style.c` | `tests/lib-style/win16-common/common_link.c` |
| `win16-common/menu_layout.c` | yes | — | `tests/ntw16s-dll/win16_style_dc_failure.c`, `tests/lib-style/win16-common/common_link.c` |
| `win16-common/palette.c` | yes | `src/lib-style/win16-control/vcl_native_paint.c` | `tests/lib-style/win16-common/common_link.c`, `tests/lib-style/win16-control/control_link.c` |
| `win2k-window/caption.c` | yes | `src/lib-style/win2k-window/frame.c` | `tests/lib-style/win2k-window/caption.c` |
| `win2k-window/frame.c` | yes | `src/ntw16s-dll/attachment.c`, `src/ntw2ks-dll/controls.c`, `src/ntw2ks-dll/window.c`, `src/ntw16s-dll/uwp/hook.c`, `src/lib-style/win16-window/frame_layout.c` (+2 more) | `tests/lib-style/win16-control/control_link.c`, `tests/lib-style/win16-window-uwp/frame.c` |
| `win2k-window/system_menu.c` | yes | `src/lib-style/win2k-window/frame.c` | `tests/ntw16s-dll/system_menu_visual.c` |
| `uwp/native.c` | yes | `src/ntw16s-dll/uwp/hook.c`, `src/ntw16s-dll/uwp/target.c` | `tests/lib-style/uwp/native.c` |
| `uwp/surface.c` | yes | `src/lib-style/win16-window-uwp/frame.c` | `tests/lib-style/uwp/surface.c` |
| `uwp/target.c` | yes | `src/run16s-exe/uwp.c`, `src/ntw16s-dll/global/manager.c`, `src/ntw16s-dll/uwp/global.c`, `src/ntw16s-dll/uwp/target.c` | `tests/ntw16s-dll/uwp/global.c`, `tests/lib-style/uwp/target.c` |

## Required follow-up

- Trace production exports and message paths across launcher, DLL, global
  mode, Win32, UWP host and WinUI; map tests to actual behavior, not linkage.
- Separate shared attach/detach, DWM restoration, XAML invalidation and popup
  lifecycle from style-specific metrics, rendering, owner policy and protocol.
- Preserve active/inactive, maximize/restore, menus, DPI, removal and reentrancy
  behavior; compare Win16 and Win2000 consumers before claiming deduplication.
- Resolve or explicitly retain the known UWP lifecycle access-denied gap; no
  ordinary desktop or synthetic paint probe substitutes for real-app coverage.
