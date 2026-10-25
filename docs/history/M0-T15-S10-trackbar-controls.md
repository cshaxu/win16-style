# M0 T15 S10: native trackbar presentation

S10 continues the owner's automatic sequential-S authorization under M0/T15.
Baseline: c9853fb598617016ba18c17fb641c9498ad3a691. Implementation, clean dual-width build, actual83 matrix, dual publication/eight
hashes, installed28+2 and actual MD5/DK checks pass. P1 push/review is pending.
CURRENT owns the live status. Queue/proposal edits from other work are preserved.

## Diagnosis and minimum implementation

Initialized canonical msctls_trackbar32 and exact main-EXE TTrackBar must pass native
class-procedure module/extra-byte and public range/position/thumb/channel protocols.
No prefix match, private VCL storage, global settings/hook, existing-process injection,
new component/product, public export or API detour is introduced.

A trackbar caches part drawing. Native v5 and v6 emit PRE only on cached paints;
requesting ITEM alone is insufficient to regenerate the buffer. Native experiments
prove WM_THEMECHANGED invalidates v6 but not v5, while the HWND-local standard
WM_SYSCOLORCHANGE notification regenerates the parts in both. No system value is set.
Public vertical TBM_GETCHANNELRECT is unrotated; actual native custom-draw part
rectangles are correctly rotated. Painting uses those exact rectangles and native
CDIS_SELECTED, not a capture-derived guess. Input, ticks, focus, geometry, data, font,
line/page values and ordinary parent notifications remain native.

Only an application PRE result exactly CDRF_DODEFAULT permits the private observer
to request extra item/post stages. These private extra stages are consumed internally;
all stages requested by the application itself retain the original route and return.
Whole-control and item custom-draw pixels have separate positive controls. The shared
palette and border primitives present channel/thumb; Win3.1 itself had no such public
control, so this is not a historical pixel-identity claim. Original five raster headers
and provenance remain byte-exact. The passive-font owned leaf and active-call lifetime
keep font attributes untouched and remove only the correct window/parent subclasses.

Finite support is v5/v6 h/v, ordinary tick placements/no ticks/fixed thumb length,
canonical and verified alias. Selection-range, reversed/RTL, tooltips and unknown
styles remain native; reparenting unbinds the observer and invalidates the old buffer.

## Verification and failed experiments

Entry: core-trackbar-v5.exe or core-trackbar-v6.exe <absolute-run16s.exe>, each x86/x64.
Native/styled children run in owned Jobs/private desktops with finite waits, at96 and
actual PMv2 144 DPI. Cases cover20 class/orientation/style variants, normal visible
PAINT, PRINT/PRINTCLIENT parity, normal/pressed border pixels, native capture/release,
keyboard/line/page/position, app notifications/whole/part custom drawing, unchanged
font/data/geometry, notification-time removal, application-query remove/destroy,
special/fake rejection, and64 create/paint/destroy resource cycles per process.
Messages do not claim physical keyboard/mouse, IME or accessibility acceptance.

The final fixture rejects all four old S9 width/version combinations at the first
missing trackbar attachment, after their native controls pass and actual DLL paths
are recorded. An early x86 run against an incomplete x64 build directory failed
installation error2; subsequent actual four-file candidate runs pass. This is a test
setup error, not a product failure. The first real DK scan used three equal files,
producing three rows where the retained fixture correctly requires a.bin/b.bin two;
corrected owned32-byte a/b plus distinct c produces2 and normal exit, without changing
any assertion or scanning user directories.

The first full matrix stops at tree-v6-64 child wait258/10seconds, after passing input
assertions. Instrumented old/new four-concurrent controls all reproduce the deadline:
about5seconds thread CPU stretches toward10seconds wall time, including a sample
that reports all variants and freed resources before process-exit deadline. This is
not declared a product deadlock or a fixed rendering regression. Tree cases receive
the scheduler's existing two-slot heavy weight; Jobs4 admits at most two trees, while
other lightweight cases still run concurrently. The 10second/function assertions
remain unchanged. Final clean package validates this measured scheduling correction;
the original failed matrix and old/new controls remain in the structured evidence.

Actual MD5summer and owned-copy DoubleKiller checks pass on the pre-scheduler
candidate. The MD5 caption292x24 black mask equals S9 (zero differences). They also pass against the final published candidate, with the caption mask
again unchanged. HD Tune administrator UI defaults
to SKIP by the owner's instruction; no UAC probe is issued or counted passed.

## Remaining receiver and delivery

UpDown has no custom-draw; tested public MSAA supplies two buttons but no pressed
state even during native mouse/keyboard pressed painting. S10 does not guess private
state or replace its drawing. Its finite investigation/implementation is received by
the next sequential S, followed by the separate frozen-universe T15 coverage audit.
M0/T15 remain open. Full package, publication, remote/archive review and cleanup
facts will be recorded here and in [evidence](../etc/evidence/trackbar-controls.json)
after they actually complete.


## Actual delivery verification

Final source193 SHA256 is1c62dd1e6ceab74a4e82012f1e015af8cc8b20279f82c583e4611aaad4c4ddcb.
C17/GCC16.1/Ninja/ccache, clean x86/x64 build and31 named exports plus ordinal1 pass;
no ABI/detour/resource expansion. Source124/294/172,23+9 provenance negatives,
6 resource negatives and13 governance negatives pass. The final83 actual-package
cases all pass in427.1922904s atJobs4. The first failed full matrix and unchanged
10second assertion are explicitly retained, not relabelled passed.

Publication writes only the four tested names to assets root and O:/winnt/system32,
with eight matching hashes and recoverable old-file backups. Both installed locations
pass14 focused cases each (ABI/tree/progress/trackbar), plus one automatic injection
matrix each; real MD5/DK final package checks exit0 and owned Jobs reach active0.
MD5 title292x24 black glyph/border mask equals S9 with0 differences. Actual native
PNG captures remain outside the repository for owner acceptance. The structured
record carries exact hashes, commands, case outcomes and finite exclusions.

P1 commit/push and actual remote/canonical recovery/safe scratch cleanup remain
pending; S10 and T15 are not closed on these local verification results alone.
