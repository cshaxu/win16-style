# M0 T26 S1: shared classic file-dialog COM adapters

The existing classic-dialog adapters for `IFileOpenDialog`, `IFileSaveDialog`,
`IFileDialog2`, `IFileDialogCustomize` and `IOleWindow`, including their shared
method definitions, are now in `lib-style/common/dialog_*`. Their COM identity,
operation dispatch, active window, native objects and fallback flag are supplied
and owned by the caller. The fallback flag retains the existing classic-dialog
compatibility policy: operations the bridge cannot mirror require the native
dialog. These adapters do not include Win16 rendering or product headers.

The Win16 proxy and dispatch in `win16-dialog` remain responsible for classic
window lifecycle, concrete UI policy and native fallback; `ntw16s-dll` supplies
product-specific callbacks. Existing COM integration tests continue to exercise
those consumers, and a standalone common-library test verifies dispatch and COM
reference forwarding without a Win16 dependency. This is not a general-purpose
replacement for arbitrary `IFileDialog` implementations; the adapter's explicit
compatibility and owner lifetime contracts must be observed.

Interactive COM dialogs and alternate-style product consumers are not proven
by the standalone test. Remaining mixed proxy, dialog UI, print, control and
window code still needs the broader T26 audit and migration.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from snapshot
`74d2eb1c27dd29b28fb3ea753c8bea8c59d90e6341b374c255a41f5edd4fe1d2`.
All eight files were published to assets and `O:\winnt` with matching hashes
before tests. Both widths passed nine common/integration component executables
(including the new adapter and Win16 chrome/link tests), plus the published
launcher menu keyboard and Win2000 scroll geometry probes: 22 focused checks.
Public process-library binary dependency checks passed both widths. Actual
interactive COM dialogs, real UWP visual behavior and other style consumers
remain unverified.
