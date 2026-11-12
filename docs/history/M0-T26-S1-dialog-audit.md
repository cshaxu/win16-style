# M0 T26 S1: dialog audit and common coordination

The file-dialog audit identifies three style-independent units: COM listener
subscription, snapshot dispatch and lifetime (`common/file_events`); modern
open/save/folder invocation with caller-supplied scope (`common/modern_file_dialog`);
and the legacy Shell filesystem folder browser (`common/classic_shell_folder`).
The existing Win16 proxy and injection callers now use these shared interfaces.
Legacy event ordering remains an explicitly named choice, not a Win16 dependency.
The Shell browser is not a Win3.1 dialog renderer.

Remaining `win16-dialog` sources, with matching headers, require the following
further work; this classification does not claim their mechanisms are all
inherently Win16-specific:

- `chrome`, `folder_create`: Win16 visual and interaction implementation.
- `classic_events`, `classic_folder`, `file_helpers`, `file_live`, `file_hook`,
  `com_hook`, `hook_state`: mixed classic-dialog policy, hook and UI state;
  separate reusable contract from Win16 choices before moving.
- `proxy`, `proxy_dispatch`, `primary`, `v2`, `customize`, `ole_window`,
  `common_methods`, `com_show`, `browse`: COM adaptation and classic routing
  intertwined with ownership and policy; audit interfaces before extracting.
- `print_classic`, `print_extended`: print contracts and concrete presentation
  are mixed; split only after tracing callers and restore behavior.
- `hidden_files`: existing opt-in behavior; review policy and callers before
  deciding its destination.

The common event test exercises listener ownership, snapshots, reentrancy and
vetoes. The modern-file native test checks invalid arguments without opening a
UI; the existing Win16 link test covers adapter linkage. Interactive selection,
actual alternate-style consumers and full dialog lifetime remain to be tested.
The larger S1 inventory, cross-style migration and UWP visual verification remain
open; this is not a claim that T26 is complete.

The x86/x64 clean MinGW GCC, Ninja and ccache build passed from snapshot
`3a425436258e118390f60789c14244f84dfd3d19c0fd3977425880a66fe9e676`.
All eight candidate files were published to the project assets root and
`O:\winnt` with matching SHA-256 values before focused tests. On both widths,
the seven static component executables passed: file events, modern-file argument
validation, classic-file contract, Win16 dialog linkage, print ranges, UWP
surface presentation and style dispatch. The published launcher also passed
the menu keyboard and Win2000 scroll geometry probes on both widths (18 checks
in total). These checks do not establish interactive dialog behavior or real
UWP rendering; the previously observed UWP lifecycle access-denied limitation
remains open. Public process-library binary dependency checks passed both widths.
