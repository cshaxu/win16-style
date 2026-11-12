# M0 T26 S1: shared print resource helpers

`lib-style/common/print_resources` now owns the existing movable-global-memory
availability and cloning, print-to-file DEVNAMES clone, ANSI/Unicode DEVMODE
copy and collate normalization, and requested printer DC/IC construction.
The Win16 PrintDlgEx bridge calls these helpers directly. Its dialog layout,
controls, snapshot ownership and compatibility/fallback policy remain in the
Win16 adapter. Returned global handles and DCs remain caller-owned; low-level
helpers require valid locked printer structures.

The standalone test exercises ANSI and Unicode memory clones, lock checks,
independent ownership, file-output naming and copy/collate normalization. It
does not create an actual printer DC or interact with a printer driver. Real
PrintDlgEx UI and alternate-style consumers are not established by this test.
The larger T26 dialog, control and window migration remains open.

The clean MinGW GCC / Ninja / ccache builds passed for x86 and x64. The eight
files were published to both destinations before testing and their hashes
matched the candidate. The focused 46 executions passed across both bitnesses,
including the resource test, dialog and control regressions, product launcher
probes and the synthetic UWP manager lifecycle test. The latter required a
non-sandboxed process-access test; it did not enable global styling. The
common dialog archive had no undefined Win16, Win2000 or product symbols.
