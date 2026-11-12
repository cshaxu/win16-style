# M0 T26 S1: remove unused invented folder dialog

The old file-template based folder picker and its custom New Folder dialog
had no production callers. Searches for `win16_dialog_classic_folder_hook`,
`win16_dialog_classic_folder_prepare`, `win16_dialog_classic_folder_finish`,
and `win16_dialog_create_folder` found only their own definitions and tests;
they were not exported by the product. The two implementation/header pairs
and their dedicated test were removed from the library and build. The chrome
test retains its file-dialog, COM, navigation, font and color checks without
the abandoned picker setup.

The live Shell browse adapter in `common/classic_browse` and
`common/classic_shell_folder` remains. The separate classic file-dialog
folder-result adjustment in `classic_events` is also retained: it has a live
caller in `ntw16s-dll/injection/com_dialogs.c` and is not the removed UI.
This removal does not establish that every possible folder-dialog API or
fallback is covered; full interactive and cross-style dialog audits remain open.

The clean x86/x64 MinGW GCC / Ninja / ccache build passed from snapshot
`f4f5f18a4a14873a2ed46c2157c789bc3fc463c77b6d175bbfa9954def478bb1`.
The eight files were published to assets and `O:\winnt` before testing;
candidate and both destinations matched. Thirty-six component executions and
four published-launcher system-menu probes passed on both bitnesses. The
dialog-link test exercises Shell browse routing with test callbacks; it does
not open and interact with the real Shell browser. The removed symbols were
absent from both clean-built Win16 dialog archives. No private desktop or
computer-use was used.
