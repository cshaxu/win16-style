# M0 T15 S8 native tree controls and pending frame ownership

Status: implementation and focused verification complete; same-four clean package
75/75 full regression passed (399.681s), dual-published eight hashes. Installed16 focused+2 automatic matrices and frozen six old-package negatives
passed. Actual remote/archive/safe cleanup remain; not yet closed. M0/T15 remain open.
HD Tune elevated real-app checks default to owner-requested skip, never pass.
Unrelated QUEUE and two proposal changes are preserved and excluded from delivery.

## Evidence and finite change

Actual MD5summer uses main-EXE TShellTreeView, native extra4, client-edge0x200,
MS Sans Serif -11,56 nodes, indent19 and item height16. The old frame route rejects
client-edge children and no tree renderer was installed. Canonical SysTreeView32
and main-EXE TTreeView/TShellTreeView now require initialized public protocol,
native registered procedure module/extra bytes, legal count/root/indent/height,
external hit test and same-thread parent before binding a private role. Opaque
HTREEITEM and application lParam are never interpreted as private structures.

Tree text reuses S7's native synchronous custom-draw DC observer and the existing
stroke-preserving System lease. It changes neither notification results nor
application fonts, colors, node data, layout or images. Client-edge and embedded
scrollbars reuse existing painters/input. Native images, expanders and lines
remain native. No shared class or system theme/setting is modified; unknown and
RTL controls retain native behavior. Complex/non-CP1252 glyphs stay native.

A separate early-visible frame reproduction proved cancellation left the frame
behind: both S7 widths returned CANCELLED1223, subsequent install failed183,
and an independent reentrant public owner could not install. The coordinator had
not yet assigned its frame flag. Private exact-owner tags and an install lifetime
reference now span publication/activation. Removal retires only that transaction's
frame and its own bar tracker. Independent public owners survive. Normal title
painting/font/layout routines are unchanged. The private frame header is correctly
reclassified adapt with its original source commit/blob/hash retained; no resource
or public ABI disposition changed.

## Tests and limitations

`core-tree-v5.exe <absolute-run16s.exe>` and v6, built x86/x64, exercise native and
injected children at96/actual144 DPI. Each child covers18 class/font/custom-draw
variants, independent host-System E8M0M0E8 masks, native node/image/font properties,
font updates, geometry, selection/first-expansion cancellation, keyboard and
scrolling, synchronous style removal, deferred destruction and RTL/fake rejection.
`core-pending-frame-owner.exe <absolute-dll>` checks cancelled, independent and
normal frame ownership. Test-Package adds six mandatory cases, total75.
Six old-S7 negative executions returned1 at tree attachment or stale frame183.

The first test incorrectly expected repeated TVM_EXPAND to notify after
TVIS_EXPANDEDONCE. [Microsoft documents suppression](https://learn.microsoft.com/en-us/windows/win32/controls/tvm-expand); the corrected test strictly
checks first-expansion cancellation and state. No assertion was weakened to pass.
Native no-DLL v6 destruction inside CDDS_PREPAINT also exits0xc0000005, matching
the injected case. This unsupported native operation is recorded, while ordinary
callback-posted destruction and synchronous style removal are checked.

Native no-DLL ASCII fixtures also exposed an unrelated private-desktop exit wait:
gdb showed UninitLocalMsCtfMonitor/TF_CanUninitialize/CtfImmCoUninitialize. Only the
fixture's own GUI thread calls [ImmDisableIME](https://learn.microsoft.com/en-us/windows/desktop/api/imm/nf-imm-immdisableime)(0) before windows. No product/global
change; IME acceptance is not claimed. Pending-frame test drops from31s to0.63s,
and report-v5/v6 focused tests complete in2.96/3.94s. Tree child budgets remain10s.

Real MD5summer normal-display proof finds68 changed glyph RGB pixels and4411
border pixels with count, client geometry, font property, selection and image
handles/count unchanged. Move/minimize/restore preserve the root attachment.
One prototype process-exit timeout remains recorded; bare and styled repeats
exit0 with empty owned Jobs. Final-package observation has a first5s exit wait
then exits0 within the subsequent bounded wait. DoubleKiller final-package scan
uses only its byte-identical task copy and three32-byte owned files; its own scan
produces the two duplicate rows. Four page reports have failures0. A diagnostic
64-bit driver loaded a32-bit probe and returned193; that was corrected by matching
the driver bitness, not a product change. An initial32-bit close failed with stale
error5; observed repeat records posted1/wait0/exit0/window0 and Job active0.

Not covered: physical keyboard/mouse, IME, accessibility, label editing, state
image lists/checkboxes, unusual tree styles, arbitrary private custom painting,
reparenting/multiple UI threads. HD Tune elevated actual UI is skipped by request.
Progress/trackbar/updown are transferred to the next sequential S, not claimed here.
Existing title/menu/font/bar effects and original graphics remain protected by
full retained package regression and byte/table gates. No fifth artifact, new
export, detour, external code or sibling dependency is introduced.

Machine-readable delivery evidence: [tree-controls.json](../etc/evidence/tree-controls.json).
Actual native PNGs are outside Git in the registered visualization directory.
Package/source/installation/remote/archive results and cleanup are recorded there
before this S is closed; no generated logs, probe binaries or diagnostic images
are committed. Stable Ninja and ccache remain; scratch follows exact-root guards.

## P1 implementation review

Implementation/test/build scope is14 files including three authored tests/header:
337 additions/61 deletions/net276 (tracked numstat plus201 new lines), excluding
metadata/docs/binaries and unrelated work. All187 recorded source inputs match the
clean tested snapshot. Source gate:124 decisions/294 includes/166 imports;23 source
and9 third-party negatives; original6 resource-substitution negatives; governance13.
Four artifacts are one clean GCC16.1.0/Ninja/ccache4.13.6 batch. Actual75-case suite
passed with4 slots;16 installed checks and two automatic startup matrices passed.
Published hashes, toolchains, source snapshot and raw command/case durations are
summarized in evidence/release metadata. Title/font/menu/scrollbar retained matrix
passes; original five resource headers/provenance and18 compiled tables remain exact.
No source or binary was swapped after the75-case run. P1 awaits push/actual remote
and strict archive review; closure uses a separate P after safety/recovery checks.
