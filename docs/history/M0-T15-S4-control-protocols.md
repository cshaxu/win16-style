# M0 T15 S4: initialized framework roles and native image controls

This S continues the owner's automatic sequential-S authorization under M0/T15.
The reference is c5e110bbf987309699634184d5c8012f169fed0a, with S3 published bytes.
Implementation, clean build, actual-package tests and double publication pass locally.
P1 5e8f880b9f893441cf61bf345218371b2c45b313 is pushed and remote187-file bytes match.
P2 502ec8389d20c89f80e1ee4b11a9b54ea67a39d2 fixes the strict provenance issue and passes
actual archive/remote184-file review. Safe cleanup completes; P3 closes S4 and admits S5.
T15/M0 remain open.
CURRENT alone owns admission/status. Other working-tree queue/proposal edits are preserved.

## Actual diagnosis and bounded implementation

Owned native MD5summer1.2.0.11 x86 observations find registered AND real TButton/TComboBox,
not a USER native superclass. All three buttons answer DLGC_BUTTON, valid BCM_GETIDEALSIZE,
BM_GETCHECK/STATE; the outer combo supplies a valid GetComboBoxInfo composite and two items.
Names or dialog flags alone are insufficient. The new private adapter verifies these public
protocols only after WM_CREATE, on the owned GUI thread, for exact application-registered
classes. A per-thread owned node plus exact HWND property token records the confirmed role.
Passive classification sends no messages. Removal/terminal paths release the node after the
existing painter/theme leaves. Invalid candidates, foreign properties, OOM and destruction
during the query have separate native production-module ownership tests.

The confirmed combo retires only its already-owned internal Edit attachment before taking
composite ownership. Verified framework buttons/combos use the existing private font lease
when the application retains its own WM_GETFONT value; standard font generation is unchanged.
No VCL private object layout, procedure decoding, prefix alias, global hook or process scan.

HD Tune4.60 x86 uses standard BUTTONs excluded by the earlier BS_ICON/BS_PUSHLIKE policy,
and six native client-edge EDITs. Buttons now retain native input/notifications while the
existing painter draws borrowed icon/bitmap images or checked PushLike faces. EDIT retains
its original client allocation, text, selection, undo and font strategy; only NC edges change.

The first 45-case candidate passed its automatic suite but FAILED actual visual acceptance:
six HD Tune toolbar buttons were blank. The subsequent approved one-shot probe establishes
BM_GETIMAGE=NULL for both types, BCM_GETIMAGELIST=TRUE, centered32x32 lists with one or five
state images. This candidate was NOT published. The corrected painter borrows that list,
uses native button state to choose the image, keeps natural-size transparent padding, and
refreshes after BCM_SETIMAGELIST. Application list/image ownership remains unchanged.
References: [BUTTON_IMAGELIST](https://learn.microsoft.com/en-us/windows/win32/api/commctrl/ns-commctrl-button_imagelist),
[state image contract](https://learn.microsoft.com/en-us/windows/win32/controls/bcm-setimagelist).

## Verification design and limits

`core/control-extensions` dynamically loads the actual DLL and uses public creation scopes.
It checks borrowed icon/bitmap pixels, PushLike checked rendering, native click/space and
disabled-notification equivalence, edit client geometry/undo, initialized framework role
pixels and repeated remove/destroy. Explicit COMCTL6 enables real image-list semantics;
29x26 buttons with32x32 transparent state images exercise the actual HD Tune geometry.
Expected colors come from independent native ImageList drawing, including its alpha rounding.
`core/control-protocol-owner` compiles the real private module with allocation tracking and
checks zero nodes after every reject/release, destroyed-query, foreign-token and OOM path.
The actual-package suite includes both tests on both architectures.

Earlier test failures are retained honestly: an under-sized fake button registration was
rejected against its native class metadata; a v5 fixture lacked BCM_SETIMAGELIST; disabled
BM_CLICK notification differs between COMCTL versions and now compares native behavior;
hard-coded RGB missed native alpha rounding. Process-wide GDI totals warmed asynchronously
and are diagnostic only, NOT a zero-leak pass. Owned protocol allocations/images and exact
release calls are tested separately. First two MD5 probe root-selection failures lacked
sufficient diagnostic logs; a later Information dialog was actually observed and dismissed,
but does not retroactively prove those earlier failures had the same cause.

Real MD5summer confirms four roles and combo open/cancel/selection preservation, screenshots,
move/minimize/restore and normal exit0/empty Job. DoubleKiller four pages and MadShow19
controls retain their native probe regressions. HD Tune is observed only in new owned
instances with the approved bounded one-shot admin probe, after automatic regressions.
No disk operations, existing-process injection, persistent controller or computer-use.

Physical mouse/keyboard, alternate physical DPI, arbitrary VCL/DLL-registered classes,
image-plus-text buttons without image styles, application self-drawn surfaces, and remaining
tab/list/header/tree/progress/trackbar/updown types are not claimed by S4. Actual apps report window DPI96; physical monitor scaling is not independently measured;
the pre-existing font/menu matrices retain their separate context/nominal-DPI scope.
Title/menu/font/scroll source preservation and original five headers/provenance remain
required. T15 stays open for the proposal's remaining common-control work and final audit.

## Local delivery

175 frozen inputs, GCC16.1.0/Ninja/ccache, pure C17; ABI3/31 exports and three components/four
products retained. The corrected clean package passes45/45 in364.1806654 seconds, four slots;
12 additional frame/default/lifetime/terminal/presentation/scroll tests pass on both widths.
88 CTests are registered; not all88 were run. Four old-package negatives fail as expected:
the native mode first rejects its image button; the separate protocol mode rejects the first
verified role. This does not prove every later native assertion independently rejects old code.

Final approved HD Tune controller16152 exits0; bare/styled owned processes close with native
exit2 and empty Jobs. Basic counts73 buttons/6 edits/5 combos=84 attached, failures0; hidden3
stay hidden. The six toolbar interiors change from0 to188/166/119/193/290/94 non-palette pixels;
top26 caption rows are RGB-identical to the rejected candidate. Normal shown screenshot,
menu open/cancel and minimize/restore/move are captured. No disk operation is executed.

The same tested four files are backed up and published to assets root and O:/winnt/system32;
eight hashes match. Restart Manager reports eight resources/zero owners, so no existing
process is closed. Both installed locations pass x86/x64 ABI and actual automatic launch.
An auxiliary pixel summary initially failed UTF8 decoding of the native CRT multibyte log;
ASCII structural fields are now parsed from raw bytes and the comparison passes. Publication
itself independently validated actual-package hashes. No failed pixel summary is counted pass.

Exact SHA, cases, preserved source/resource hashes, selected native facts and limits are in
[the evidence index](../etc/evidence/control-protocols.json). P1/P2 actual remote/coordinator reviews and safe registered-scratch cleanup pass;
T15/M0 remain open for automatic following S.

## P2: canonical provenance correction

Actual P1 archive review found one pre-existing authored receipt from T14: legacy_rejection.c
was hashed with working-tree CRLF, while .gitattributes and its existing Git blob use LF.
The content is identical after newline normalization; there is no C token or Git source change.
The receipt now matches the already-committed LF SHA ee931c0390948de1446acb4827804041a04329625d372d5bca6b5366d8348233;
working-tree bytes are canonicalized. Strict validator logic and all retained resources stay exact.
The tested snapshot's raw CRLF and canonical Git identities remain explicit in release.json.

The exported source with this single receipt correction passes source23/third-party9/resource6
negative gates, a clean cached dual-width all-target build/ABI, and the native legacy rejection
regression against the actual published launcher. Its first probe invocation lacked the required
capture-directory environment (203); the corrected native invocation passes all4 cases.
The proof build is verification scratch and does not replace the tested/published P1 four files.
No product Git input changed; all eight deployment hashes remain P1. This is metadata correction,
not an additional styling change. Actual P2 archive gates pass;175 exported inputs equal the rebuilt proof (LICENSE uses its
explicit recorded raw export-EOL identity). Remote184 files match in9.421 seconds. Eight
published files remain unchanged. No validator or resource byte check was weakened.

## Closure and handoff

P1 remote187-file retrieval takes10.741 seconds; P2 remote184-file retrieval takes9.421 seconds.
Sequential coordinator review reads actual commits/diffs, the original request, source identities
and selected evidence. All other queue/proposal changes are excluded. Safe cleanup verifies exact
workspace roots/no reparse points, live task executables0 and110 task DLL resources/owners0.
All eight prior deployment bytes are recoverable from c5e110b assets. Obsolete S4 builds, package
copies/backups, logs, probes and captures are removed after extraction. Seven reviewed observer/
verifier sources transfer under build/t15-controls to following S; stable Ninja/ccache and
external acceptance PNGs remain. S4 closes; S5 handles TabControl/TPageControl, with other
families and final frozen-universe task audit still required before T15 can close.
