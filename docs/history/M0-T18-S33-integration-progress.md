# M0 T18 S33 — ongoing final integration and audit

The independent `s33-clean1` four-file package passed a clean dual-width
GCC 16.1.0 / Ninja / ccache 4.13.6 build, source, original resource,
governance and ABI gates. Its snapshot.json SHA-256 was
`8445ade70b92485a03a604dcc10542c5e701eef8469b56d64a0bb4ed646e3475`.
The full actual-package integration run passed 127 cases, no skips, in
581.74 seconds (two jobs). It included both font stress cases, creation and
rollback, native and styled dialogs and messages, as well as the standard
VB/VCL and two-architecture menu matrix. No physical input is asserted.
The running tests used self-owned isolated desktops and instances. The real
HD Tune administrator probe was not run, as requested by the user.

The independently rebuilt package was also used by two actual isolated
DelphiVCL4Python 1.0.7 examples at 144 DPI. Both bitnesses loaded their exact
corresponding candidate DLL and reported the root and normal and disabled
TStaticText attached, retained the original Segoe UI/Tahoma fonts, and closed
successfully. Captures compared with the S31 old-package same-scene captures
showed zero differences in the top 26 title rows and 1107 changed pixels in
the remainder on each width. This is a bounded runtime sample, not proof for
all Delphi releases, languages or physical input. The screenshot and raw
observer output are retained in registered `build/t18-s33-final-audit`.

All eight deployment files match the tested package, and installed x86 assets
and x64 system32 entry points passed the static-text model probe:

| File | SHA-256 |
| --- | --- |
| demo.exe (x64) | `5826a9afad39191c7f9152d38f68752a348fa3a0676f0196cdc6d7edad33a5fc` |
| ntw16s32.dll (x86) | `8d89e5534f6d6996f9c1d1f5e88b869f850783a8d17e6c4310a3ee04e4a38925` |
| ntw16s64.dll (x64) | `2ac073790af4e356c9b24a58221a60bdb1b941e55ddd29fffa42f50b3ec82c3e` |
| run16s.exe (x64) | `c627667650ae72620d30101bcaaccade97646f1432c74b4f1dadc8e4b73cf4cb` |

The [working 74-row ledger](../etc/evidence/delphi-s33-working-ledger.json)
contains exactly the frozen S1 set with no duplicate or missing names. It
updates the actual TStaticText, TForm and windowless TLabel findings but the
remaining rows still require review; it is not a claim of 74 supported controls.
The user-authorized 23 deferred VB/OCX units and other content-neutral contract
limitations remain in TODO. The observed x86/x64 dialog-scope cases took about
25 seconds, dialog-show about 115 seconds, and final dialogs/messages suites
85/92 seconds; these are recorded for a subsequent T18 efficiency S rather
than silently deleting scenarios. S33 and T18 remain open pending further
row-by-row review and the user's requested efficiency work.
