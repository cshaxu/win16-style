# M0 T26 S1: shared classic file helpers

The classic file dialog's displayed-folder path reconstruction, Shell item
selection, typed and drive targets, overwrite target and shortcut resolution
now reside in `lib-style/common/classic_file_helpers`. They operate on standard
classic dialog control IDs and preserve the existing input validation and Shell
ownership behavior. The experimental hidden-file list enumeration is in
`common/classic_hidden_files` with a caller-supplied folder lookup. Its Win16
private registered message and scheduling remain in the product adapter.

Win16 hooks, classic event handling, folder logic and injection callers now
use the shared functions directly; the product-private duplicate declaration
was removed. The standalone common test covers folder path reconstruction and
buffer rejection with an owned hidden native window. Existing integration tests
cover more typed, drive, overwrite, selection and hidden-file paths. These are
classic-dialog mechanisms, not a promise of arbitrary modern control support.
Interactive file selection and alternate-style product use remain unverified;
T26's wider window, control and dialog audit remains open.

The x86/x64 clean MinGW GCC, Ninja and ccache build passed from snapshot
`59004871cb67c283fa6c139f32e459e587c5e32562383f1904034822c791a87f`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. Both widths passed eleven component executables (including the
new folder helper and existing classic/COM dialog tests) and two published
launcher probes, for 26 focused checks. Public process-library binary boundary
checks passed both widths. No interactive dialog or alternate-style product
consumer was tested.
