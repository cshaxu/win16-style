# M0 T26 S1: shared menu input

Win16 and Win2000 separately implemented menu-label mnemonic parsing, keyboard
action classification and mouse tracking transitions. These decisions now
reside in `src/lib-style/common/menu_input.h` and are called from both styles.
The shared interface accepts each caller's no-hit, cancel and armed state;
Win16's system-menu item and Win2000's initial-button-release behavior remain
style-owned. The mnemonic parser retains the existing style-specific treatment
of text after a tab. Menu geometry, painting, popup tracking, enabled-item
selection and focus ownership remain in the corresponding style components.

Dual-width incremental builds passed. The independent common input test covers
mnemonics, escaped ampersands, tab boundaries, keys, menu hover, cancellation,
the Win16 system-menu sentinel and the Win2000 armed rule. The Win16 standalone
control test exercises the actual Win16 menu dispatch. Using a temporary mixed
preflight stage, both architectures passed Win16 mnemonic dispatch through
`run16s`, and Win2000 mnemonic, F10, selection, hover, arrow and nested-menu
message tests. The preflight stage was removed. These are message-driven tests,
not physical input or user visual acceptance.

S1 remains open: this does not claim the two complete popup trackers or all
window, control, dialog and UWP/WinUI mechanisms are unified. Existing DWM
restoration and screen-pixel issues remain as recorded in earlier evidence.

The clean x86/x64 MinGW GCC 16.1.0, Ninja and ccache 4.13.6 build passed;
snapshot SHA-256 is `bdef345996adf2f2e793dc564cedc6dd7cc13170ba8921c9c7a17c095e34a14a`.
The eight-file package was published to `assets` and `O:\winnt` before testing.
All 18 post-publication direct tests passed: common menu input and Win16 control
link, published launcher with Win16 mnemonic, and published launcher with six
Win2000 menu scenarios, each on both architectures. All eight artifact hashes
matched the candidate and both destinations. No private desktop or computer-use
was used; physical input, visual acceptance and the full matrix remain open.

| Published file | SHA-256 |
| --- | --- |
| demo.exe | `8321ca1334df529da1dcffc3d45a5c7e17980c0b4f20359722788b8862b24114` |
| ntw16s32.dll | `8c025bbbfa81168b52212f263f3ffb80f914caa58b12293a6a9e461a31f64cb2` |
| ntw16s64.dll | `6dada00b5d85a491f795d13bc4d8b86bf806d0e1cc9085cf285e4dc2537bd65c` |
| ntw2ks32.dll | `36623f1810a090775cd09fa6ab195a27f26d739d02daf03b69c718cf44831e10` |
| ntw2ks64.dll | `05f00aebd82cc1b0bac1daf5c3ba0b04190156cfa5a7fc3d522b6cff1207dc14` |
| ntwshk32.dll | `b81a96beba53b3c289b14ea90b49ceb293be6cfcdcfe1831a4bdd51888351fef` |
| ntwshk64.dll | `ef5c0fe11ec917d896da0d7722e8cfe34a04f0b2852dceb9869e083cb7c8c085` |
| run16s.exe | `3d4a2a472b04a2adb655ad512e9cf3d7aafde7f861522b56821e63b7bcb5022b` |
