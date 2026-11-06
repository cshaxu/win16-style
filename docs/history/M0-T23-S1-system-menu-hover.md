# M0 T23 S1 P6: Notepad system-menu hover

Owner acceptance of P4/P5 reported flicker specifically inside the top-left
window popup, not the caption or client area. S1 remains active; S2 is not admitted.

## Diagnosis

The ordinary native test window converts system-menu rows to owner draw (type0x100).
The real installed Notepad keeps native strings (type0). Its popup therefore uses
menu_scope.c's fallback painter after native tracking. That path painted every row
straight to the screen on every WM_MOUSEMOVE/WM_NCMOUSEMOVE, even when selection
was unchanged. A marker in an unrelated row was erased in the real old Notepad
popup (white instead of0xdb11f3). The ordinary owner-draw fixture did not reproduce
that mechanism and was not treated as a substitute for Notepad.

The pure C thread-scoped observation hook operated only on newly launched Notepad
instances, opened/ended their system menu and inspected native item types/pixels.
No document text was changed. Existing user windows were not touched.

## Minimal change

- Read native highlight selection immediately before/after mouse dispatch. An
  unchanged selection needs no extra overlay painting; retain no parallel menu state.
- Compose necessary fallback row painting in a temporary compatible bitmap and
  copy it once to the popup. WM_PRINT keeps its existing caller DC/coordinates.
- Keep HMENU tracking, command IDs, owner data, cancellation, fonts, title renderer,
  resources and lifetime contracts. Other owner-draw/custom menu paths are unchanged.
- Temporary DC/bitmap ownership is confined to the paint call; selection reads and
  allocation failures retain the existing fallback behavior.

## Regression evidence

The extended system-menu-visual executable has a hover mode modeling a host that
retains native strings. It primes selection, writes a marker to an unrelated row,
moves within the same selected item three times, and checks that row was not
repainted. The same executable fails against the old deployed DLL on both widths
(exit1, unchanged_row=0), and passes with the candidate on both widths (exit0,
unchanged_row=1). Native item data is restored and selected color remains correct.
A first x86 candidate invocation used a directory without ntw16s32.dll and failed
with error2 before the test; the complete four-file incremental bundle passed.

Real candidate Notepad also reports type0 and retains0xdb11f3 after the same
hover sequence. This confirms the production fallback path, not merely the model.
The test proves redundant painting is removed; it does not claim to measure every
possible sub-frame compositor transition or replace the user's visual acceptance.

## Delivery

Final same-snapshot build, package regression, deployment and remote hashes are
recorded below after completion. Scratch: build/t23-package/s1-menu-hover1,
s1-clean4, s1-package4, s1-publish2. Earlier failed probe builds are diagnostic-only;
no product release was made from them. M0/T23/S1 remain open.

## P6 delivery results

Clean package s1-clean4 passed x86/x64 GCC/Ninja/ccache builds, original resource,
ABI and public dependency checks. Ten focused core tests passed (five per width).
Actual same-package s1-package4 passed29/29 with zero skips: previous25 cases plus
system-menu-hover and menu-cancel on both widths. This is focused coverage, not
all historical tests. Production delta37 added/4 removed lines; test delta24/4,
package registration1/0. No new ABI or product files.

Final real Notepad7124 loaded the exact clean4 DLL; a popup was found and
PrintWindow returned success, but the subsequent observation did not obtain
properties (exit6). The second attempt did not observe a popup. These are NOT
passes. After restoring the earlier probe DPI context and moving capture after
observation, another attempt stopped before launch because an existing Notepad
was present; it was not operated on or closed. Thus final-package physical hover
and a validated open-menu screenshot remain owner acceptance items. Earlier
incremental real Notepad and both-width old/new regression evidence remain valid;
no uncertain screenshot is presented as proof of flicker elimination.

The same tested four files were published to assets root and O:/winnt using
s1-publish2. Eight hashes were verified. Mapped old DLLs were retained in
s1-menu-hover1/mapped-backup without killing their occupants. New launches load
new files; already running processes may retain the previous mappings. S1 stays
open for owner acceptance, T23 remains open, S2 not admitted.

| Product | SHA-256 |
| --- | --- |
| demo.exe | `784344d22c65e30eb1c43fb628251d8165711890c1bd4d38cd91f80cb7aa3c26` |
| ntw16s32.dll | `e59094f5e71d84f4761f2ba2d7c36cb2741152941876a6558f3996364e1958f9` |
| ntw16s64.dll | `8018f23efd11e33cf6bdee85307e467f94663934fa2f22f08c0f98c0ef100a1a` |
| run16s.exe | `611029193467742be6dac20f092d14420035b5b7f43bf6cc46fd305330486c33` |
