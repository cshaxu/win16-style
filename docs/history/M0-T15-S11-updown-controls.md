# M0 T15 S11: native UpDown rendering and exact DC ownership

This continuation stays in M0/T15 and preserves the owner's unrelated queue/proposals.
The owner authorized sequential S implementation, publication and commit/push. Elevated
HD Tune UI remains default skip; no UAC prompt or existing-process injection is used.
Baseline is S10 P2 2608b6150a4bdad28ae18ce9fb9898f6f3dfad13.

Native prototypes disprove input/capture inference: WM_CANCELMODE can release capture
while the native pressed visual persists, and buddy keyboard processing does not enter
the UpDown HWND's input subclass. UDN cancellation or app-modified delta also cannot
identify the physical painted direction. MSAA exposes two buttons but no useful pressed
state; no NM_CUSTOMDRAW arrives. These are failed candidates, not proof of impossibility.

Actual API interception in owned C probes establishes v5 DrawFrameControl direction and
DFCS_PUSHED/INACTIVE, and v6 DrawThemeBackground SPNP1..4/state1..4. V6 uses two cached
compatible DCs per control; a bounded per-thread table attributes48/48 nonprint calls,
with8 observed creates/deletes. Begin/EndBufferedPaint and direct DTBEx calls are zero.
Two UpDowns and OpenThemeData(NULL,SPIN) share HTHEME0x20001: theme handles cannot identify
windows. The reviewed packet admits four additional process-local APIs and35 detours /
40 private slots; public ABI3/31 named exports+ordinal1 and three components/four products
remain. Original vendor notices and five graphics/provenance are unchanged.

The private painter is embedded in the existing control owner. Only initialized native
role9 or verified exact main-EXE TUpDown participates. Drawing requires exact owned HWND,
synchronous supplied print DC or a compatible DC created by that control's original
COMCTL module. Nested foreign-window paint is a barrier. A32-entry weak table owns no
native DC, clears on successful DeleteDC or control retirement, and falls back natively
on overflow. Acquired owner references cover every paint and release at the last active
call; removal/terminal invalidates attribution before deferred free. No theme identity,
input-state machine, private object decoding, global hook, shared-class or system change.

The original scroll arrow interior scales equally on both axes within actual native
hit rectangles; existing palette/frame primitives supply the bevel. Native UDN/buddy,
position/range/base, geometry/font, mouse/keyboard/cancel and native drawing parameters
remain. Unknown styles, RTL, extra edges or unknown draw flags/states remain native.
Passive ordinary DLL loading without startup API initialization does not falsely attach
UpDown; the test proves native pixels unchanged.

Verification includes x86/x64 v5/v6, native/styled96/actual144 DPI, eight canonical/alias,
horizontal/vertical and small/large variants. It checks visible paint/PRINT parity,
normal/pressed/disabled pixels, buddy keys, capture/cancel, reverse range, app cancellation
and delta override, fake alias rejection, notification-time removal and query destruction.
A shared-theme unowned control stays byte-identical while another owned control exists.
All four old S10 product cases fail at missing attachment after native baseline passes.
The new40-slot transaction test rejects old32-slot source at its33rd attach on both
architectures; current40 slots/41st rejection/abort restoration pass.

Global GDI/USER counters fluctuate in native no-DLL runs even after GdiFlush. They are
recorded, not asserted as leak proof or made to pass with warmup, retries or wider limits.
The production-module owner fixture instead checks32-slot overflow/recovery,130 cycles,
retirement clearing, wrong caller/thread and nested barriers, exact references zero and
165 owned DC DeleteDC/invalid-SaveDC results. GetObjectType can still report type3 after
DeleteDC; that failed predicate was replaced with the actual failed SaveDC contract.

The structured [evidence](../etc/evidence/updown-controls.json) owns final package,
publication, installed checks, remote/archive/recovery and cleanup results. Initial14
focused tests pass. Clean GCC/Ninja/ccache x86/x64 builds pass in34.52/38.51 seconds;
source124/294/176,23+9 provenance negatives,6 graphic negatives and13 governance negatives
pass. Final actual-package91 cases pass in406.0985072 seconds atJobs4, with no failed case or HD Tune UAC run. Double publication and installed verification pass; actual remote/archive/cleanup review follows the P1 push.

Real final-candidate MD5summer and owned DoubleKiller pass normal display, moves/restores,
expected app-produced two duplicate rows, close exit0 and Job active0. Original user data
is untouched. Same-probe S10/S11 MD5 caption292x24 black mask has0 differences. Historical
S10 PNG differs at only two outer corners (1,1 and290,1), with text/buttons identical;
the contemporaneous raw baseline resolves this capture difference, no renderer change.

Finite coverage is not arbitrary framework support. Custom VCL TSpinButton, windowless
artwork, arbitrary languages/OCX/IME/physical input and HD Tune current elevated UI remain
outside this S. T15 still requires a separately frozen coverage-universe audit.


P1 delivery source identity:197 inputs, SHA256
1f196f850d88d876b3e18e14616c95220c5f8bed07ffd82b9afbe0b1c776ba73.
Seventeen implementation/test/build files add429/delete12/net417 lines; docs,
receipts, binaries and unrelated work excluded. Both installed locations pass18
focused ABI/tree/progress/trackbar/UpDown checks plus one automatic injection matrix,
38 checks in total. Actual installed DLL paths match each location.
