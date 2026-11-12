# M0 T26 S1: Win16 control source audit index

This index freezes the current Win16 control C source set, not a claim that
all functions have been audited or migrated. Group decisions below are review
priorities: rendering and Win3.1 behavior remain style policy; mixed modules
must be split by actual contracts rather than moved wholesale. Standard Win32
control IDs or common Windows APIs alone do not establish reuse.

Evidence: `scroll_layout.c` explicitly implements the fixed-thumb USER 3.1
branch, while `scroll_track.c` cites USER 3.1 timing and cancellation behavior.
Win2000 proportional scrollbar behavior cannot be inferred from those functions.
`control_protocol.c` contains reusable role discovery but also a Win16 private
window property and thread-local binding state: define a shared ownership and
namespace contract before extraction. `control_install.c`, `control_remove.c`
and `control_lifetime.c` combine generic subclass lifecycle with Win16 palette,
font, scrollbar and control-specific state, and cannot be copied wholesale.
The shared `common/present` already has Win16 consumers; Win2000 use is not
yet established. `win2k-window/frame.c` and Win16 menu tracking both call
`common/menu_input.h`, but retain different layout, selection and popup policy.

Direct references below mean literal `#include` of the same basename; they are
not a call graph and do not prove runtime coverage. A dash means no such direct
include was found, not that the source has no callers. Test-family coverage is
documented by `tests/lib-style/win16-control/TEST_MAP.md`; `control_link.c`
links the component and is not by itself proof of drawing or user interaction.
A paired header is listed only when present.

## win3.1 interaction semantics

| Source | Header | Direct production includes | Direct test includes |
| --- | --- | --- | --- |
| `scroll_layout.c` | `scroll_layout.h` | — | `tests/ntw16s-dll/win16_style_scroll_geometry.c` |
| `scroll_track.c` | `scroll_track.h` | `src/ntw16s-dll/controls.c` | `tests/ntw16s-dll/win16_style_scroll_geometry.c` |

## win16 rendering or glyphs

| Source | Header | Direct production includes | Direct test includes |
| --- | --- | --- | --- |
| `basic_paint.c` | `basic_paint.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/combo_paint.c`, `src/lib-style/win16-control/list_paint.c`, `src/lib-style/win16-control/other_paint.c`, `src/lib-style/win16-control/scroll_paint.c`, `src/lib-style/win16-control/special_paint.c`, `src/lib-style/win16-control/vcl_native_paint.c` | `tests/lib-style/win16-control/control_link.c` |
| `choice_paint.c` | `choice_paint.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/controls.c`, `src/ntw16s-dll/injection/window_titles.c`, `src/lib-style/win16-control/attachment_guard.c`, `src/lib-style/win16-control/choice_view.c`, `src/lib-style/win16-control/special_paint.c` | — |
| `combo_paint.c` | `combo_paint.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/native_combo_identity.c` | — |
| `header_paint.c` | `header_paint.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/control_guard.c`, `src/lib-style/win16-control/control_state.c`, `src/lib-style/win16-control/native_presentation.c` | — |
| `list_paint.c` | `list_paint.h` | `src/ntw16s-dll/controls.c` | `tests/lib-style/win16-control/control_link.c` |
| `month_paint.c` | `month_paint.h` | `src/ntw16s-dll/attachment.c` | — |
| `other_paint.c` | `other_paint.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/choice_view.c` | — |
| `progress_paint.c` | `progress_paint.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/control_guard.c`, `src/lib-style/win16-control/control_state.c`, `src/lib-style/win16-control/native_presentation.c`, `src/lib-style/win16-control/special_install.c` | — |
| `scroll_assets.c` | `scroll_assets.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/scroll_paint.c`, `src/lib-style/win16-control/special_install.c`, `src/lib-style/win16-control/special_paint.c` | `tests/lib-style/win16-control/control_link.c` |
| `scroll_paint.c` | `scroll_paint.h` | `src/ntw16s-dll/controls.c` | — |
| `special_paint.c` | `special_paint.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/controls.c`, `src/ntw16s-dll/injection/window_titles.c`, `src/lib-style/win16-control/cool_grip_identity.c`, `src/lib-style/win16-control/month_paint.c`, `src/lib-style/win16-control/scroll_message.c`, `src/lib-style/win16-control/spin_scope.c`, `src/lib-style/win16-control/status_frame.c` (+3 more) | — |
| `tab_paint.c` | `tab_paint.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/control_guard.c`, `src/lib-style/win16-control/control_state.c`, `src/lib-style/win16-control/native_presentation.c` | — |
| `toolbar_paint.c` | `toolbar_paint.h` | `src/lib-style/win16-control/notify_message.c` | — |
| `trackbar_paint.c` | `trackbar_paint.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/notify_message.c`, `src/lib-style/win16-control/special_install.c` | — |
| `updown_paint.c` | `updown_paint.h` | — | — |
| `vcl_extra_frame.c` | `vcl_extra_frame.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/injection/window_titles.c` | — |

## identity/protocol, needs policy separation

| Source | Header | Direct production includes | Direct test includes |
| --- | --- | --- | --- |
| `control_protocol.c` | `control_protocol.h` | `src/lib-style/win16-control/choice_view.c`, `src/lib-style/win16-control/combo_paint.c`, `src/lib-style/win16-control/control_dispatch.c`, `src/lib-style/win16-control/control_guard.c`, `src/lib-style/win16-control/notify_message.c`, `src/lib-style/win16-control/progress_paint.c`, `src/lib-style/win16-control/scroll_message.c`, `src/lib-style/win16-control/special_install.c` (+5 more) | `tests/ntw16s-dll/ocx_tree_host.c` |
| `cool_grip_identity.c` | `cool_grip_identity.h` | `src/ntw16s-dll/attachment.c` | — |
| `native_combo_identity.c` | `native_combo_identity.h` | `src/ntw16s-dll/attachment.c` | — |
| `spin_identity.c` | `spin_identity.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/attachment_guard.c`, `src/lib-style/win16-control/control_lifetime.c`, `src/lib-style/win16-control/spin_scope.c` | — |
| `status_identity.c` | `status_identity.h` | `src/ntw16s-dll/attachment.c` | — |
| `vcl_identity.c` | `vcl_identity.h` | `src/ntw16s-dll/attachment.c`, `src/lib-style/win16-control/attachment_guard.c`, `src/lib-style/win16-control/vcl_extra_frame.c` | — |

## lifecycle or message dispatch mixed with Win16 state

| Source | Header | Direct production includes | Direct test includes |
| --- | --- | --- | --- |
| `attachment_guard.c` | — | — | — |
| `basic_message.c` | `basic_message.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/control_dispatch.c` | — |
| `combo_text.c` | `combo_text.h` | `src/ntw16s-dll/controls.c` | — |
| `control_dispatch.c` | `control_dispatch.h` | `src/ntw16s-dll/controls.c` | — |
| `control_guard.c` | `control_guard.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/control_install.c` | — |
| `control_install.c` | `control_install.h` | `src/ntw16s-dll/controls.c` | — |
| `control_lifetime.c` | `control_lifetime.h` | `src/ntw16s-dll/controls.c` | — |
| `control_remove.c` | `control_remove.h` | `src/ntw16s-dll/controls.c` | — |
| `control_scope.c` | `control_scope.h` | `src/ntw16s-dll/control_scope.c` | `tests/lib-style/win16-control/control_scope.c` |
| `control_state.c` | `control_state.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/special_paint.c` | `tests/lib-style/win16-control/control_link.c` |
| `custom_draw.c` | `custom_draw.h` | `src/lib-style/win16-control/notify_message.c` | `tests/lib-style/win16-control/control_link.c` |
| `edit_message.c` | `edit_message.h` | `src/ntw16s-dll/controls.c` | — |
| `fileowner_message.c` | `fileowner_message.h` | `src/ntw16s-dll/controls.c`, `src/lib-style/win16-control/control_guard.c`, `src/lib-style/win16-control/control_install.c` | `tests/lib-style/win16-control/control_link.c` |
| `list_message.c` | `list_message.h` | `src/ntw16s-dll/controls.c` | — |
| `native_presentation.c` | `native_presentation.h` | `src/ntw16s-dll/controls.c` | — |
| `notify_message.c` | `notify_message.h` | `src/ntw16s-dll/controls.c` | — |
| `scroll_message.c` | `scroll_message.h` | `src/ntw16s-dll/controls.c` | — |
| `scroll_native.c` | `scroll_native.h` | `src/ntw16s-dll/controls.c` | `tests/ntw16s-dll/win16_style_scroll_geometry.c` |
| `special_install.c` | `special_install.h` | `src/ntw16s-dll/controls.c` | — |
| `spin_scope.c` | `spin_scope.h` | `src/ntw16s-dll/attachment.c` | — |
| `updown_adapter.c` | `updown_adapter.h` | `src/ntw16s-dll/controls.c`, `src/ntw16s-dll/injection/window_titles.c`, `src/lib-style/win16-control/control_remove.c` | `tests/ntw16s-dll/updown_owner.c` |
| `vcl_native_paint.c` | `vcl_native_paint.h` | `src/ntw16s-dll/attachment.c` | — |

## text or view policy, review reuse contract

| Source | Header | Direct production includes | Direct test includes |
| --- | --- | --- | --- |
| `choice_view.c` | `choice_view.h` | `src/ntw16s-dll/controls.c` | — |
| `month_text.c` | `month_text.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/injection/window_titles.c`, `src/lib-style/win16-control/month_paint.c` | `tests/ntw16s-dll/month_model.c` |
| `status_font.c` | `status_font.h` | `src/ntw16s-dll/attachment.c` | — |
| `status_frame.c` | `status_frame.h` | `src/ntw16s-dll/attachment.c` | — |
| `status_text.c` | `status_text.h` | `src/ntw16s-dll/attachment.c` | — |
| `text_policy.c` | `text_policy.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/injection/window_titles.c` | `tests/lib-style/win16-control/control_link.c` |
| `theme_kind.c` | `theme_kind.h` | — | `tests/lib-style/win16-control/control_link.c` |
| `vb_text.c` | `vb_text.h` | `src/ntw16s-dll/attachment.c`, `src/ntw16s-dll/controls.c`, `src/ntw16s-dll/vb_text.c`, `src/ntw16s-dll/global/hook.c`, `src/ntw16s-dll/global/vb_text.c`, `src/ntw16s-dll/injection/window_titles.c`, `src/lib-style/win16-control/special_paint.c`, `src/lib-style/win16-control/status_frame.c` (+1 more) | `tests/ntw16s-dll/global_vb_text.c`, `tests/ntw16s-dll/vb_font.c` |
| `vcl_frame_policy.c` | `vcl_frame_policy.h` | `src/ntw16s-dll/attachment.c` | — |

## Header-only components

- `attachment_state.h`: Win16 attachment and lifetime state, mixed policy.
- `control_class.h`: VB/VCL and native-control recognition coupled to the
  current protocol registry; review a generic admission interface.
- `dc_geometry.h`: pixel viewport predicate; candidate for sharing after
  finding consumers and confirming each requires identical coordinates.
- `combo_glyph.h`, `control_glyphs.h`, `scroll_glyphs.h`: Win16 glyph data;
  keep style-owned and preserve provenance.

## Protocol fixture on the current desktop

The existing `core-control-protocol-owner.exe --private-worker` path invokes
its worker directly; it does not execute the fixture's private-desktop setup.
Both existing x86 and x64 incremental binaries returned zero and reported
`PROTOCOL_OWNER_OK`: zero live allocations/bindings after cleanup, five invalid
cases, destroy during query, allocation failure, foreign property conflict and
combo/edit/list impostors. The test creates and destroys its own hidden windows.
This validates the existing Win16 protocol only, not a new shared implementation,
style-switching coexistence or actual Win2000 controls. The fixture's default
entry creates a private desktop and was not used.

## Remaining decisions

1. Specify a style-neutral control attachment/retirement contract with
   reentrancy, per-thread ownership and restoration tests before extraction.
2. Separate protocol discovery and namespace/ownership from style policy;
   retain exact VB/VCL admission rules until both consumers are tested.
3. Compare Win2000 scrollbar geometry and message behavior against its source;
   extract only genuinely shared algorithms, keeping separate policies.
4. Audit the remaining Win16 window, dialog and UWP/WinUI components and all
   indirect callers; construct a full S sequence before closing S1.
