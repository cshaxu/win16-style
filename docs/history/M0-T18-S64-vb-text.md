# M0 T18 S64 P1 — VB observer cross-process control text

The VB intrinsic observer used GetWindowTextW to log control text and find an
About button in another process. A real Actor edit showed stale/empty cached
text, causing a separate diagnostic to misreport a successful change. The
observer now sends WM_GETTEXT with SendMessageTimeoutW and a bounded 1000 ms
wait; failures are explicit. The two top-level window-title reads remain as
before. The separate vb6-probe runs inside the target process and was not
changed. This changes test instrumentation only, not the product renderer.

The self-owned dual-width native test launches a separate EDIT owner on a
private desktop, sets and reads Unicode text, checks invalid-window failure,
and closes the process/job. The old GetWindowTextW substitute failed its
positive text assertion on both bit widths, exit 1; the new helper passed and
reported legacy_equal=0. Real x86 Actor under the installed package exposed
13 windows and correctly reported O:\winnt, 0, and 1 in its textboxes, then
closed its owned job. Previous real bare/styled Actor and MadShow checks used
WM_GETTEXT for mutation, selection and restoration; see
[VB evidence](../etc/evidence/vb-current-s64.json). Those checks do not prove
physical input, IME, every VB version or every control state.

A clean GCC 16.1.0, Ninja and ccache 4.13.6 x86/x64 build used snapshot
manifest SHA-256 `321f530c3f9cf68a84aefae28a819c67ebbf2bb46e8f9c495cf9d24c5bd9363e`.
Source, resources, governance and PE/ABI gates passed. Fourteen actual-package
focused cases passed in 12.66 seconds at four jobs with no skips, including
both VB controls/fonts and the new observer test. Both installed launchers
passed the test with both bit widths. Other regression cases were not run.
The rebuild changed binary hashes, so all four tested files were published to
assets root and O:/winnt/system32 with matching hashes:

| File | SHA-256 |
| --- | --- |
| demo.exe | `783266e149e1c489f1554ab041246ea23600ec29515c588d618d7df871febcf7` |
| ntw16s32.dll | `e651e215977012fa4800ccbff75deae38f6091bf4554d529fb2f71cd623d7800` |
| ntw16s64.dll | `d84bd56f8194c5ca53362666250d19a64c2d20b37f661324fb5a613f2a277d1e` |
| run16s.exe | `27557905bf2dd62d8ef543d118b78a1fb6b2d7b5696a53197b9d2c9ffb119dea` |

Search `rg -n 'GetWindowText[AW]\\(' tests/integration` found the two fixed
cross-process VB control reads, retained top-level title reads, the in-target
VB and Delphi observers, and unrelated top-level/test-local paths. The original
style renderer and graphics remain byte-for-byte untouched at source level;
this is a P1 within S64, whose separate VCL investigation remains open.

Code/build wiring scope: 78 added lines and 2 removed lines (net +76),
including the two new C test files; production source delta is zero. The
observer owns the timeout and read buffer; the target continues to own its
control state and strings. The former control-read path was replaced only at
the two cross-process sites.
