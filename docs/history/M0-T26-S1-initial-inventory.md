# M0 T26 S1: frozen initial file inventory

Baseline: T25 S8 P13, source snapshot 27fc26ca. This inventory freezes file paths, not a completed dependency or reusability audit.
Each entry still requires classification and mapping to production callers, tests and a migration decision.

Scope: style-common and all win16/win2k libraries, plus ntw16s global/injection/uwp. Product root files, launcher, CMake and tests are additional audit surfaces.

## src/lib-win32/style-common

Files: 5

- `src/lib-win32/style-common/dwm.h`
- `src/lib-win32/style-common/frame.h`
- `src/lib-win32/style-common/global_window.c`
- `src/lib-win32/style-common/global_window.h`
- `src/lib-win32/style-common/menu_hit.h`

## src/lib-win32/win16-common

Files: 11

- `src/lib-win32/win16-common/control_api.h`
- `src/lib-win32/win16-common/font.c`
- `src/lib-win32/win16-common/font.h`
- `src/lib-win32/win16-common/layout.c`
- `src/lib-win32/win16-common/layout.h`
- `src/lib-win32/win16-common/menu_layout.c`
- `src/lib-win32/win16-common/menu_layout.h`
- `src/lib-win32/win16-common/palette.c`
- `src/lib-win32/win16-common/palette.h`
- `src/lib-win32/win16-common/present.c`
- `src/lib-win32/win16-common/present.h`

## src/lib-win32/win16-window

Files: 33

- `src/lib-win32/win16-window/frame_buttons.h`
- `src/lib-win32/win16-window/frame_dispatch.c`
- `src/lib-win32/win16-window/frame_dispatch.h`
- `src/lib-win32/win16-window/frame_font.c`
- `src/lib-win32/win16-window/frame_input.c`
- `src/lib-win32/win16-window/frame_input.h`
- `src/lib-win32/win16-window/frame_layout.c`
- `src/lib-win32/win16-window/frame_layout.h`
- `src/lib-win32/win16-window/frame_lifecycle.c`
- `src/lib-win32/win16-window/frame_lifecycle.h`
- `src/lib-win32/win16-window/frame_message.c`
- `src/lib-win32/win16-window/frame_message.h`
- `src/lib-win32/win16-window/frame_paint.c`
- `src/lib-win32/win16-window/frame_paint.h`
- `src/lib-win32/win16-window/frame_state.c`
- `src/lib-win32/win16-window/frame_state.h`
- `src/lib-win32/win16-window/frame_view.c`
- `src/lib-win32/win16-window/frame_view.h`
- `src/lib-win32/win16-window/menu_bar.c`
- `src/lib-win32/win16-window/menu_bar.h`
- `src/lib-win32/win16-window/menu_bar_paint.c`
- `src/lib-win32/win16-window/menu_glyphs.h`
- `src/lib-win32/win16-window/menu_items.c`
- `src/lib-win32/win16-window/menu_items.h`
- `src/lib-win32/win16-window/menu_iteration.c`
- `src/lib-win32/win16-window/menu_iteration.h`
- `src/lib-win32/win16-window/menu_message.c`
- `src/lib-win32/win16-window/menu_message.h`
- `src/lib-win32/win16-window/menu_paint.c`
- `src/lib-win32/win16-window/menu_paint.h`
- `src/lib-win32/win16-window/menu_scope.c`
- `src/lib-win32/win16-window/menu_scope.h`
- `src/lib-win32/win16-window/win16_style.h`

## src/lib-win32/win16-control

Files: 115

- `src/lib-win32/win16-control/attachment_guard.c`
- `src/lib-win32/win16-control/attachment_state.h`
- `src/lib-win32/win16-control/basic_message.c`
- `src/lib-win32/win16-control/basic_message.h`
- `src/lib-win32/win16-control/basic_paint.c`
- `src/lib-win32/win16-control/basic_paint.h`
- `src/lib-win32/win16-control/choice_paint.c`
- `src/lib-win32/win16-control/choice_paint.h`
- `src/lib-win32/win16-control/choice_view.c`
- `src/lib-win32/win16-control/choice_view.h`
- `src/lib-win32/win16-control/combo_glyph.h`
- `src/lib-win32/win16-control/combo_paint.c`
- `src/lib-win32/win16-control/combo_paint.h`
- `src/lib-win32/win16-control/combo_text.c`
- `src/lib-win32/win16-control/combo_text.h`
- `src/lib-win32/win16-control/control_class.h`
- `src/lib-win32/win16-control/control_dispatch.c`
- `src/lib-win32/win16-control/control_dispatch.h`
- `src/lib-win32/win16-control/control_glyphs.h`
- `src/lib-win32/win16-control/control_guard.c`
- `src/lib-win32/win16-control/control_guard.h`
- `src/lib-win32/win16-control/control_install.c`
- `src/lib-win32/win16-control/control_install.h`
- `src/lib-win32/win16-control/control_lifetime.c`
- `src/lib-win32/win16-control/control_lifetime.h`
- `src/lib-win32/win16-control/control_protocol.c`
- `src/lib-win32/win16-control/control_protocol.h`
- `src/lib-win32/win16-control/control_remove.c`
- `src/lib-win32/win16-control/control_remove.h`
- `src/lib-win32/win16-control/control_scope.c`
- `src/lib-win32/win16-control/control_scope.h`
- `src/lib-win32/win16-control/control_state.c`
- `src/lib-win32/win16-control/control_state.h`
- `src/lib-win32/win16-control/cool_grip_identity.c`
- `src/lib-win32/win16-control/cool_grip_identity.h`
- `src/lib-win32/win16-control/custom_draw.c`
- `src/lib-win32/win16-control/custom_draw.h`
- `src/lib-win32/win16-control/dc_geometry.h`
- `src/lib-win32/win16-control/edit_message.c`
- `src/lib-win32/win16-control/edit_message.h`
- `src/lib-win32/win16-control/fileowner_message.c`
- `src/lib-win32/win16-control/fileowner_message.h`
- `src/lib-win32/win16-control/header_paint.c`
- `src/lib-win32/win16-control/header_paint.h`
- `src/lib-win32/win16-control/list_message.c`
- `src/lib-win32/win16-control/list_message.h`
- `src/lib-win32/win16-control/list_paint.c`
- `src/lib-win32/win16-control/list_paint.h`
- `src/lib-win32/win16-control/month_paint.c`
- `src/lib-win32/win16-control/month_paint.h`
- `src/lib-win32/win16-control/month_text.c`
- `src/lib-win32/win16-control/month_text.h`
- `src/lib-win32/win16-control/native_combo_identity.c`
- `src/lib-win32/win16-control/native_combo_identity.h`
- `src/lib-win32/win16-control/native_presentation.c`
- `src/lib-win32/win16-control/native_presentation.h`
- `src/lib-win32/win16-control/notify_message.c`
- `src/lib-win32/win16-control/notify_message.h`
- `src/lib-win32/win16-control/other_paint.c`
- `src/lib-win32/win16-control/other_paint.h`
- `src/lib-win32/win16-control/progress_paint.c`
- `src/lib-win32/win16-control/progress_paint.h`
- `src/lib-win32/win16-control/scroll_assets.c`
- `src/lib-win32/win16-control/scroll_assets.h`
- `src/lib-win32/win16-control/scroll_glyphs.h`
- `src/lib-win32/win16-control/scroll_layout.c`
- `src/lib-win32/win16-control/scroll_layout.h`
- `src/lib-win32/win16-control/scroll_message.c`
- `src/lib-win32/win16-control/scroll_message.h`
- `src/lib-win32/win16-control/scroll_native.c`
- `src/lib-win32/win16-control/scroll_native.h`
- `src/lib-win32/win16-control/scroll_paint.c`
- `src/lib-win32/win16-control/scroll_paint.h`
- `src/lib-win32/win16-control/scroll_track.c`
- `src/lib-win32/win16-control/scroll_track.h`
- `src/lib-win32/win16-control/special_install.c`
- `src/lib-win32/win16-control/special_install.h`
- `src/lib-win32/win16-control/special_paint.c`
- `src/lib-win32/win16-control/special_paint.h`
- `src/lib-win32/win16-control/spin_identity.c`
- `src/lib-win32/win16-control/spin_identity.h`
- `src/lib-win32/win16-control/spin_scope.c`
- `src/lib-win32/win16-control/spin_scope.h`
- `src/lib-win32/win16-control/status_font.c`
- `src/lib-win32/win16-control/status_font.h`
- `src/lib-win32/win16-control/status_frame.c`
- `src/lib-win32/win16-control/status_frame.h`
- `src/lib-win32/win16-control/status_identity.c`
- `src/lib-win32/win16-control/status_identity.h`
- `src/lib-win32/win16-control/status_text.c`
- `src/lib-win32/win16-control/status_text.h`
- `src/lib-win32/win16-control/tab_paint.c`
- `src/lib-win32/win16-control/tab_paint.h`
- `src/lib-win32/win16-control/text_policy.c`
- `src/lib-win32/win16-control/text_policy.h`
- `src/lib-win32/win16-control/theme_state.c`
- `src/lib-win32/win16-control/theme_state.h`
- `src/lib-win32/win16-control/toolbar_paint.c`
- `src/lib-win32/win16-control/toolbar_paint.h`
- `src/lib-win32/win16-control/trackbar_paint.c`
- `src/lib-win32/win16-control/trackbar_paint.h`
- `src/lib-win32/win16-control/updown_adapter.c`
- `src/lib-win32/win16-control/updown_adapter.h`
- `src/lib-win32/win16-control/updown_paint.c`
- `src/lib-win32/win16-control/updown_paint.h`
- `src/lib-win32/win16-control/vb_text.c`
- `src/lib-win32/win16-control/vb_text.h`
- `src/lib-win32/win16-control/vcl_extra_frame.c`
- `src/lib-win32/win16-control/vcl_extra_frame.h`
- `src/lib-win32/win16-control/vcl_frame_policy.c`
- `src/lib-win32/win16-control/vcl_frame_policy.h`
- `src/lib-win32/win16-control/vcl_identity.c`
- `src/lib-win32/win16-control/vcl_identity.h`
- `src/lib-win32/win16-control/vcl_native_paint.c`
- `src/lib-win32/win16-control/vcl_native_paint.h`

## src/lib-win32/win16-dialog

Files: 53

- `src/lib-win32/win16-dialog/browse.c`
- `src/lib-win32/win16-dialog/browse.h`
- `src/lib-win32/win16-dialog/chrome.c`
- `src/lib-win32/win16-dialog/chrome.h`
- `src/lib-win32/win16-dialog/classic_events.c`
- `src/lib-win32/win16-dialog/classic_events.h`
- `src/lib-win32/win16-dialog/classic_folder.c`
- `src/lib-win32/win16-dialog/classic_folder.h`
- `src/lib-win32/win16-dialog/com_hook.c`
- `src/lib-win32/win16-dialog/com_hook.h`
- `src/lib-win32/win16-dialog/com_show.c`
- `src/lib-win32/win16-dialog/com_show.h`
- `src/lib-win32/win16-dialog/common_methods.h`
- `src/lib-win32/win16-dialog/customize.c`
- `src/lib-win32/win16-dialog/customize.h`
- `src/lib-win32/win16-dialog/events.c`
- `src/lib-win32/win16-dialog/events.h`
- `src/lib-win32/win16-dialog/file_contract.c`
- `src/lib-win32/win16-dialog/file_contract.h`
- `src/lib-win32/win16-dialog/file_helpers.c`
- `src/lib-win32/win16-dialog/file_helpers.h`
- `src/lib-win32/win16-dialog/file_hook.c`
- `src/lib-win32/win16-dialog/file_hook.h`
- `src/lib-win32/win16-dialog/file_live.c`
- `src/lib-win32/win16-dialog/file_live.h`
- `src/lib-win32/win16-dialog/folder_create.c`
- `src/lib-win32/win16-dialog/folder_create.h`
- `src/lib-win32/win16-dialog/hidden_files.c`
- `src/lib-win32/win16-dialog/hidden_files.h`
- `src/lib-win32/win16-dialog/hook_state.c`
- `src/lib-win32/win16-dialog/hook_state.h`
- `src/lib-win32/win16-dialog/modern.c`
- `src/lib-win32/win16-dialog/modern.h`
- `src/lib-win32/win16-dialog/ole_window.c`
- `src/lib-win32/win16-dialog/ole_window.h`
- `src/lib-win32/win16-dialog/primary.c`
- `src/lib-win32/win16-dialog/primary.h`
- `src/lib-win32/win16-dialog/print_classic.c`
- `src/lib-win32/win16-dialog/print_classic.h`
- `src/lib-win32/win16-dialog/print_extended.c`
- `src/lib-win32/win16-dialog/print_extended.h`
- `src/lib-win32/win16-dialog/print_ranges.c`
- `src/lib-win32/win16-dialog/print_ranges.h`
- `src/lib-win32/win16-dialog/proxy.c`
- `src/lib-win32/win16-dialog/proxy.h`
- `src/lib-win32/win16-dialog/proxy_dispatch.c`
- `src/lib-win32/win16-dialog/proxy_dispatch.h`
- `src/lib-win32/win16-dialog/shell_folder.c`
- `src/lib-win32/win16-dialog/shell_folder.h`
- `src/lib-win32/win16-dialog/shell_item.c`
- `src/lib-win32/win16-dialog/shell_item.h`
- `src/lib-win32/win16-dialog/v2.c`
- `src/lib-win32/win16-dialog/v2.h`

## src/lib-win32/win16-window-uwp

Files: 4

- `src/lib-win32/win16-window-uwp/frame.c`
- `src/lib-win32/win16-window-uwp/frame.h`
- `src/lib-win32/win16-window-uwp/frame_layout.c`
- `src/lib-win32/win16-window-uwp/frame_layout.h`

## src/lib-win32/win2k-window

Files: 6

- `src/lib-win32/win2k-window/caption.c`
- `src/lib-win32/win2k-window/caption.h`
- `src/lib-win32/win2k-window/frame.c`
- `src/lib-win32/win2k-window/frame.h`
- `src/lib-win32/win2k-window/system_menu.c`
- `src/lib-win32/win2k-window/system_menu.h`

## src/ntw16s-dll/global

Files: 23

- `src/ntw16s-dll/global/attach.c`
- `src/ntw16s-dll/global/attach.h`
- `src/ntw16s-dll/global/hook.c`
- `src/ntw16s-dll/global/late.c`
- `src/ntw16s-dll/global/late.h`
- `src/ntw16s-dll/global/manager.c`
- `src/ntw16s-dll/global/manager.h`
- `src/ntw16s-dll/global/policy.c`
- `src/ntw16s-dll/global/policy.h`
- `src/ntw16s-dll/global/process.c`
- `src/ntw16s-dll/global/process.h`
- `src/ntw16s-dll/global/remote.c`
- `src/ntw16s-dll/global/remote.h`
- `src/ntw16s-dll/global/session.c`
- `src/ntw16s-dll/global/session.h`
- `src/ntw16s-dll/global/start.c`
- `src/ntw16s-dll/global/start.h`
- `src/ntw16s-dll/global/thread.c`
- `src/ntw16s-dll/global/thread.h`
- `src/ntw16s-dll/global/vb_text.c`
- `src/ntw16s-dll/global/vb_text.h`
- `src/ntw16s-dll/global/window.c`
- `src/ntw16s-dll/global/window.h`

## src/ntw16s-dll/injection

Files: 14

- `src/ntw16s-dll/injection/com_dialogs.c`
- `src/ntw16s-dll/injection/create_process.c`
- `src/ntw16s-dll/injection/entry.c`
- `src/ntw16s-dll/injection/file_dialogs.c`
- `src/ntw16s-dll/injection/folder_dialogs.c`
- `src/ntw16s-dll/injection/intercept.h`
- `src/ntw16s-dll/injection/late_process.c`
- `src/ntw16s-dll/injection/late_process.h`
- `src/ntw16s-dll/injection/modern_dialogs.c`
- `src/ntw16s-dll/injection/subclass32.def`
- `src/ntw16s-dll/injection/subclass64.def`
- `src/ntw16s-dll/injection/theme_track.c`
- `src/ntw16s-dll/injection/theme_track.h`
- `src/ntw16s-dll/injection/window_titles.c`

## src/ntw16s-dll/uwp

Files: 8

- `src/ntw16s-dll/uwp/global.c`
- `src/ntw16s-dll/uwp/global.h`
- `src/ntw16s-dll/uwp/hook.c`
- `src/ntw16s-dll/uwp/hook.h`
- `src/ntw16s-dll/uwp/native.c`
- `src/ntw16s-dll/uwp/native.h`
- `src/ntw16s-dll/uwp/target.c`
- `src/ntw16s-dll/uwp/target.h`
