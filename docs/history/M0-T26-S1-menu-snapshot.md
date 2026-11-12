# M0 T26 S1: shared system-menu rows

The Win16 and Win2000 system-popup renderers now call the same
`lib-style/common/menu_snapshot.h` row reader. It reads USER's text, state,
geometry, bitmap and submenu fields and preserves both renderers' existing
rejection of owner-draw, oversized text and unknown bitmap types. Win16 passes
its live detach flag so synchronous USER calls can still cancel painting.
Each style retains its own layout, glyphs, colors, painting and popup lifetime.

The native test creates its own popup on the ordinary desktop, observes an
actual menu item with a bounded timer, and checks text, geometry, owner-draw
rejection and cancellation. It does not use physical input, a private desktop
or computer-use. Full visual acceptance, nested menu interaction, and the
existing Win2000 DWM restoration issue remain open.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from source
snapshot `e1be6978839a4d0783cce38571e47583c1f977f592f6d0fb2e219e28430921b6`.
All eight files were published to assets and `O:\winnt` before verification;
their hashes matched the candidate at both destinations. Thirty component
executions passed across both bitnesses. Twelve additional published-launcher
executions passed: normal, application owner-draw and synthetic hover messages
for both Win16 and Win2000 system menus on both bitnesses. These probes do not
establish physical input or all menu lifetime and reentrancy combinations.

The original production row readers were removed, while each renderer remains
the owner of its own popup and style-specific state. No new renderer or
alternate graphics were introduced.
